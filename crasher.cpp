// crasher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>

#include <iostream>

// ��ĳ�����̱���
// http://zplutor.github.io/2016/09/08/how-to-make-a-specified-application-crash/

bool enableDebugPriv() {
 HANDLE hToken;
 LUID sedebugnameValue;
 TOKEN_PRIVILEGES tkp;

 if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
   return false;
 }

 if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {
   CloseHandle(hToken);
   return false;
 }

 tkp.PrivilegeCount = 1;
 tkp.Privileges[0].Luid = sedebugnameValue;
 tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

 if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
   CloseHandle(hToken);
   return false;
 }

 return true;
}

// ��ָ����ʱ���
bool CrashForNullPointer(HANDLE process_handle) {
 bool ret = true;
 HANDLE thread_handle = CreateRemoteThread(process_handle, nullptr, 0, 0, nullptr, 0, nullptr);
 if (thread_handle == nullptr) {
   ret = false;
   std::wcout << L"Create remote thread failed. Error: " << GetLastError() << '.' << std::endl;
 }

 return ret;
}

// ��ջ�������
// �Զ����̺߳���
#pragma pack(push, 1)
struct StackOverFlowThreadData {
 char buffer[256];
};
#pragma pack(pop)

DWORD WINAPI StackOverFlowThreadFunc(LPVOID lpParam) {
 // ���߳����еĴ�����Ҫ�ܹ���Ŀ�������ִ��
 //   StackOverFlowThreadData* data = static_cast<StackOverFlowThreadData*>(lpParam);

 MessageBoxA(NULL, "aaa", "aaa", NULL);

 return 0;
}

bool CrashForStackOverFlow(HANDLE process_handle) {
 bool ret = false;
 // ��Ŀ������з����̺߳���������ڴ�
 const DWORD dwThreadSize = 4096;
 SIZE_T dwWriteBytes = 0;
 LPVOID threadFunctionAddress = VirtualAllocEx(process_handle, NULL, dwThreadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

 // ���Զ����̺߳���д��Ŀ������ڴ�
 if (!WriteProcessMemory(process_handle, threadFunctionAddress, &StackOverFlowThreadFunc, dwThreadSize, &dwWriteBytes)) {
   std::wcout << L"WriteProcessMemory failed. Error: " << GetLastError() << '.' << std::endl;
   return ret;
 }

 // ���䲢д���Զ����߳�����
 //   StackOverFlowThreadData threadData = {"Hello from injected custom thread!"};
 //   LPVOID threadDataAddress = VirtualAllocEx(process_handle, NULL, sizeof(threadData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
 //   WriteProcessMemory(process_handle, threadDataAddress, &threadData, sizeof(threadData), NULL);

 // ��Ŀ������д���Զ���̲߳������Զ����̺߳���
 DWORD threadID = 0;
 HANDLE thread_handle = CreateRemoteThread(process_handle, NULL, 0, (DWORD(WINAPI *)(void *))(threadFunctionAddress),
                                           /*threadDataAddress*/ NULL, 0, &threadID);

 if (thread_handle == nullptr) {
   std::wcout << L"Create remote thread failed. Error: " << GetLastError() << '.' << std::endl;
 } else {
   ret = true;
 }

 while (true) {
   Sleep(5000);
 }

 return ret;
}

int main(int argc, char **argv) {
 //     if (argc < 2) {
 //         std::wcout << L"A PID is needed." << std::endl;
 //         return 1;
 //     }
 //
 //     int pid = atoi(argv[1]);
 //     if (pid <= 0) {
 //         std::wcout << L"Invalid PID." << std::endl;
 //         return 2;
 //     }

 int pid = 22772;

 enableDebugPriv();
 HANDLE process_handle = OpenProcess(
     /*PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ*/ PROCESS_ALL_ACCESS, FALSE,
     pid);

 if (process_handle == nullptr) {
   std::wcout << L"Open process failed. Error: " << GetLastError() << '.' << std::endl;
   return 3;
 }

 if (!CrashForStackOverFlow(process_handle)) {
   return 4;
 }

 std::wcout << L"Crashed. :)" << std::endl;

 CloseHandle(process_handle);
 return 0;
}


