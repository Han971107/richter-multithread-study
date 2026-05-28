#include <Windows.h>
#include <iostream>

using namespace std;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    cout << "Hello from thread!" << endl;
    return 0;
}

int main()
{
    cout << "[Main] Creating thread..." << endl;

    HANDLE h = ::CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
    if (h == NULL) {
        cout << "[Main] CreateThread failed: " << GetLastError() << endl;
        return 1;
    }

    cout << "[Main] Waiting for thread..." << endl;

    DWORD res = ::WaitForSingleObject(h, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;
        ::CloseHandle(h);
        return -1;
    }

    ::CloseHandle(h);

    cout << "[Main] Thread finished. Bye!" << endl;

    return 0;
}