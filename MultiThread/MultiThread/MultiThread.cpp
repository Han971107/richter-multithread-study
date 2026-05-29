#include <Windows.h>
#include <process.h>
#include <iostream>

using namespace std;

long g_counter = 0;

LONG MyAtomicAdd(LONG volatile* p, LONG value)
{
    LONG oldVal, newVal;
    do {
        oldVal = *p;
        newVal = oldVal + value;
    } while (InterlockedCompareExchange(p, newVal, oldVal) != oldVal);

    return oldVal;
}

unsigned __stdcall ThreadProc(void* lpParameter)
{
    //cout << "I am Thread " << (int)(intptr_t)(lpParameter) << endl;
    for (int i = 0; i < 1000000; ++i) {
        MyAtomicAdd(&g_counter, 1);
    }

    return 0;
}

int main()
{
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

    cout << g_counter << endl;

    return 0;
}