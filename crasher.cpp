// crasher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>

// ÈÃÄ³¸ö½ø³Ì±ÀÀ£
// http://zplutor.github.io/2016/09/08/how-to-make-a-specified-application-crash/

int main(int argc, char** argv) {
    if (argc < 2) {
        std::wcout << L"A PID is needed." << std::endl;
        return 1;
    }

    int pid = atoi(argv[1]);
    if (pid <= 0) {
        std::wcout << L"Invalid PID." << std::endl;
        return 2;
    }

    HANDLE process_handle = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE,
        pid);

    if (process_handle == nullptr) {
        std::wcout << L"Open process failed. Error: " << GetLastError() << '.' << std::endl;
        return 3;
    }

    HANDLE thread_handle = CreateRemoteThread(process_handle, nullptr, 0, 0, nullptr, 0, nullptr);
    if (thread_handle == nullptr) {
        std::wcout << L"Create remote thread failed. Error: " << GetLastError() << '.' << std::endl;
        return 4;
    }

    std::wcout << L"Crashed. :)" << std::endl;
    return 0;
}