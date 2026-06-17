#include <Windows.h>
#include <process.h>
#include <map>
#include <string>
#include <iostream>
#include <queue>
using namespace std;

CRITICAL_SECTION g_cs;
CONDITION_VARIABLE g_cv;
int g_doneCount = 0;

unsigned __stdcall WorkerThreadProc(void* lpParameter)
{
    Sleep(rand() % 5 + 1);
    EnterCriticalSection(&g_cs);
    g_doneCount++;
    WakeAllConditionVariable(&g_cv);
    LeaveCriticalSection(&g_cs);

    return 0;
}

int main()
{
    srand((unsigned)time(NULL));
    InitializeCriticalSection(&g_cs);
    InitializeConditionVariable(&g_cv);

    constexpr int readerThreadCount = 3;

    HANDLE readHandles[readerThreadCount] = {};

    int createdThread = 0;
    for (int i = 0; i < readerThreadCount; ++i) {

        readHandles[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, (void*)(intptr_t)(i + 1), 0, NULL);

        if (readHandles[i] == 0) {
            cout << "Failed creating thread..." << endl;
            cout << GetLastError() << endl;
            return -1;
        }

        ++createdThread;
    }

    EnterCriticalSection(&g_cs);
    while (g_doneCount < 3) {
        SleepConditionVariableCS(&g_cv, &g_cs, INFINITE);
    }
    LeaveCriticalSection(&g_cs);
    cout << "모두 완료" << endl;

    DWORD res = ::WaitForMultipleObjects(createdThread, readHandles, TRUE, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;

        for (int i = 0; i < createdThread; ++i) {
            ::CloseHandle(readHandles[i]);
        }

        return -1;
    }

    for (int i = 0; i < createdThread; ++i) {
        ::CloseHandle(readHandles[i]);
    }

    return 0;
}