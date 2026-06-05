#include <Windows.h>
#include <process.h>
#include <map>
#include <string>
#include <iostream>
#include <queue>
using namespace std;

CRITICAL_SECTION g_cs;
CONDITION_VARIABLE g_cv;
queue<int> g_jobs;
bool g_shutdown = false;
LONG g_processedCount = 0;

unsigned __stdcall WorkerThreadProc(void* lpParameter)
{
    while (true) {
        EnterCriticalSection(&g_cs);
        while (g_jobs.empty() && !g_shutdown) {
            SleepConditionVariableCS(&g_cv, &g_cs, INFINITE);
        }
        if (g_jobs.empty()) {
            LeaveCriticalSection(&g_cs);
            break;
        }
        g_jobs.pop();
        LeaveCriticalSection(&g_cs);
        InterlockedIncrement(&g_processedCount);
    }

    return 0;
}

int main()
{
    srand((unsigned)time(NULL));
    InitializeCriticalSection(&g_cs);
    InitializeConditionVariable(&g_cv);

    constexpr int readerThreadCount = 4;

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

    for (int i = 0; i < 1000; ++i) {
        EnterCriticalSection(&g_cs);
        g_jobs.push(i + 1);
        LeaveCriticalSection(&g_cs);
        WakeConditionVariable(&g_cv);
    }

    g_shutdown = true;
    WakeAllConditionVariable(&g_cv);

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

    cout << g_processedCount << endl;

    return 0;
}