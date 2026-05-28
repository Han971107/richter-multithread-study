#include <Windows.h>
#include <iostream>

using namespace std;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int id = (int)(intptr_t)lpParameter;
    cout << "[Worker" << " " << id << "] " << "I am thread number " << id << endl;
    return 0;
}

int main()
{
    constexpr int THREAD_COUNT = 3;

    cout << "[Main] Creating " << THREAD_COUNT << " threads..." << endl;

    HANDLE handles[THREAD_COUNT] = {};
    
    int createdCount = 0;
    for (int i = 0; i < THREAD_COUNT; ++i) {

        handles[i] = ::CreateThread(NULL, 0, ThreadProc, (LPVOID)(intptr_t)(i + 1), 0, NULL);

        if (handles[i] == NULL) {
            cout << "[Main] CreateThread failed at index " << i << endl;
            break;  // 루프 탈출
        }

        createdCount++;
    }

    cout << "[Main] Waiting for thread..." << endl;

    DWORD res = ::WaitForMultipleObjects(createdCount, handles, TRUE, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;
        for (int i = 0; i < createdCount; ++i) {
            ::CloseHandle(handles[i]);
        }
        return -1;
    }

    for (int i = 0; i < createdCount; ++i) {
        ::CloseHandle(handles[i]);
    }

    cout << "[Main] Thread finished. Bye!" << endl;

    return 0;
}