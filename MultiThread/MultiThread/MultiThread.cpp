#include <Windows.h>
#include <process.h>
#include <iostream>

using namespace std;

struct Stats {
    LONG sum;    // 합
    LONG count;  // 개수
};

Stats g_stats = { 0, 0 };
CRITICAL_SECTION g_cs;

unsigned __stdcall ThreadProc(void* lpParameter)
{
    for (int i = 0; i < 1000000; ++i) {
        int value = rand() % 100 + 1;     // 락 밖
        EnterCriticalSection(&g_cs);
        g_stats.sum += value;             // 락 안
        g_stats.count++;                  // 락 안
        LeaveCriticalSection(&g_cs);
    }

    return 0;
}

int main()
{
    srand((unsigned)time(NULL));        
    InitializeCriticalSection(&g_cs);

    constexpr int threadCount = 4;

    HANDLE handles[threadCount] = {};

    int createdThread = 0;
    for (int i = 0; i < threadCount; ++i) {

        handles[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (void*)(intptr_t)(i + 1), 0, NULL);

        if (handles[i] == 0) {
            cout << "Failed creating thread..." << endl;
            cout << GetLastError() << endl;
            return -1;
        }

        ++createdThread;
    }

    DWORD res = ::WaitForMultipleObjects(createdThread, handles, TRUE, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;

        for (int i = 0; i < createdThread; ++i) {
            ::CloseHandle(handles[i]);
        }

        return -1;
    }

    for (int i = 0; i < createdThread; ++i) {
        ::CloseHandle(handles[i]);
    }

    DeleteCriticalSection(&g_cs);

    cout << "count = " << g_stats.count << endl;
    cout << "sum = " << g_stats.sum << endl;
    cout << "avg = " << (g_stats.sum / g_stats.count) << endl;

    return 0;
}