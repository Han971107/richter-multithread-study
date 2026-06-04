#include <Windows.h>
#include <process.h>
#include <map>
#include <string>
#include <iostream>
using namespace std;

SRWLOCK g_lock;
map<int, string> g_assets;   // ID → 자산 이름
LONG g_writerDone = 0;
LONG g_successCount = 0;

unsigned __stdcall WriterThreadProc(void* lpParameter)
{
    for (int i = 0; i < 100; ++i) {
        int id = rand() % 100 + 1;
        AcquireSRWLockExclusive(&g_lock);
        g_assets.insert(std::make_pair(id, std::to_string(id)));
        ReleaseSRWLockExclusive(&g_lock);
        Sleep(10);
    }
    InterlockedExchange(&g_writerDone, 1);

    return 0;
}

unsigned __stdcall ReaderThreadProc(void* lpParameter)
{
    while (g_writerDone == 0) {
        int id = rand() % 100 + 1;
        AcquireSRWLockShared(&g_lock);
        auto iter = g_assets.find(id);
        if (iter != g_assets.end()) {
            InterlockedIncrement(&g_successCount);
        }
        ReleaseSRWLockShared(&g_lock);
    }

    return 0;
}

int main()
{
    srand((unsigned)time(NULL));        
    InitializeSRWLock(&g_lock);

    // Writer
    HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, WriterThreadProc, (void*)(intptr_t)(1), 0, NULL);
    if (handle == 0) {
        cout << "Failed creating thread..." << endl;
        cout << GetLastError() << endl;
        return -1;
    }

    constexpr int readerThreadCount = 9;

    HANDLE readHandles[readerThreadCount] = {};

    int createdThread = 0;
    for (int i = 0; i < readerThreadCount; ++i) {

        readHandles[i] = (HANDLE)_beginthreadex(NULL, 0, ReaderThreadProc, (void*)(intptr_t)(i + 1), 0, NULL);

        if (readHandles[i] == 0) {
            cout << "Failed creating thread..." << endl;
            cout << GetLastError() << endl;
            return -1;
        }

        ++createdThread;
    }

    DWORD res = ::WaitForMultipleObjects(createdThread, readHandles, TRUE, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;

        for (int i = 0; i < createdThread; ++i) {
            ::CloseHandle(readHandles[i]);
        }

        return -1;
    }
    
    res = ::WaitForSingleObject(handle, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;
        ::CloseHandle(handle);
        return -1;
    }

    for (int i = 0; i < createdThread; ++i) {
        ::CloseHandle(readHandles[i]);
    }

    ::CloseHandle(handle);

    cout << "총 데이터 수: " << g_assets.size() << endl;
    cout << "성공 횟수: " << g_successCount << endl;

    return 0;
}