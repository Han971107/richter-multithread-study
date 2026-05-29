#include <Windows.h>
#include <process.h>
#include <iostream>

using namespace std;

unsigned __stdcall ThreadProc(void* lpParameter)
{
    int sum = 0;
    for (int i = 1; i <= 100; ++i) {
        sum += i;
    }
    return sum;
}

int main()
{
    cout << "[Main] Creating thread..." << endl;

    HANDLE h = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, 0, 0, NULL);
    if (h == 0) {
        cout << "Failed creating thread..." << endl;
        cout << GetLastError() << endl;
        return -1;
    }

    cout << "[Main] Waiting for thread..." << endl;

    DWORD res = ::WaitForSingleObject(h, INFINITE);
    if (res == WAIT_FAILED) {
        cout << "[Main] Wait failed: " << GetLastError() << endl;
        ::CloseHandle(h);
        return -1;
    }

    DWORD exitCode = 0;
    if (!::GetExitCodeThread(h, &exitCode)) {
        cout << "[Main] GetExitCodeThread failed: " << GetLastError() << endl;
        ::CloseHandle(h);
        return -1;
    }
    cout << "[Main] result = " << exitCode << endl;

    ::CloseHandle(h);

    cout << "[Main] Thread finished. Bye!" << endl;

    return 0;
}