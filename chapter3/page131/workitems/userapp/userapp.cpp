// boost.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <Psapi.h>
#include <wil\resource.h>
#include <shlwapi.h>
#include <memory>
#include "..\workitem\WorkitemCommon.h"



bool EnableDebugPrivilege() {
	wil::unique_handle hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, hToken.addressof()))
		return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		return false;

	if (!::AdjustTokenPrivileges(hToken.get(), FALSE, &tp, sizeof(tp), nullptr, nullptr))
		return false;

	return ::GetLastError() == ERROR_SUCCESS;
}

int GetProcessPid(LPCWSTR ProcName) {
	int maxCount = 256;
	std::unique_ptr<DWORD[]> pids;
	int count = 0;
	DWORD target_pid = NULL;

	for (;;) {
		pids = std::make_unique<DWORD[]>(maxCount);
		DWORD actualSize;
		if (!::EnumProcesses(pids.get(), maxCount * sizeof(DWORD), &actualSize))
			break;

		count = actualSize / sizeof(DWORD);
		if (count < maxCount)
			break;

		maxCount *= 2;
	}

	for (int i = 0; i < count; i++) {
		DWORD pid = pids[i];
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (!hProcess) {
			printf("Failed to get a handle to process %d (error=%d) [can be ignored]\n", pid, ::GetLastError());
			continue;
		}
		FILETIME start = { 0 }, dummy;
		if (!::GetProcessTimes(hProcess, &start, &dummy, &dummy, &dummy)) {
			printf("Failed!!!\n");
			continue;
		}

		SYSTEMTIME st;
		::FileTimeToLocalFileTime(&start, &start);
		::FileTimeToSystemTime(&start, &st);
		WCHAR exeName[MAX_PATH];
		DWORD size = MAX_PATH;
		DWORD count =
			::QueryFullProcessImageName(hProcess, 0, exeName, &size);

		if (wcsstr(exeName, ProcName)) {
			printf("PID: %5d, Image: %ws\n", pid, exeName);
			target_pid = pid;
		}

	}

	if (!target_pid) {
		printf("Failed to get the target process PID - Quitting\n");
		exit(0);
	}
	return target_pid;
}

int GetProcessThreads(DWORD PID, int threads_list[]) {
	HANDLE thread_snap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;
	int count = 0;

	thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (thread_snap == INVALID_HANDLE_VALUE) {
		printf("Invalid Handle Value");
		return(FALSE);
	}

	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(thread_snap, &te32)) {
		printf("Thread32First Error");
		CloseHandle(thread_snap);
		return(FALSE);
	}

	do {
		if (te32.th32OwnerProcessID == PID) {
			//printf("THREAD ID: %d\n", te32.th32ThreadID);
			threads_list[count] = te32.th32ThreadID;
			count++;
		}
	} while (Thread32Next(thread_snap, &te32));

	CloseHandle(thread_snap);
	return count;
}

int wmain(int argc, const wchar_t* argv[]) {

	if (!EnableDebugPrivilege()) {
		printf("Failed to enable Debug privilege!\n");
	}


	JunkData data;



	HANDLE hDevice = CreateFile(L"\\\\.\\workitem", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Error opening device (%u)\n", GetLastError());
		return 1;
	}

	DWORD bytes;

	data.junk = 47;

	if (!DeviceIoControl(hDevice, IOCTL_WORKITEM, &data, sizeof(data), nullptr, 0, &bytes, nullptr)) {
		printf("Error  (%u)\n", GetLastError());
		return 1;
	}
	CloseHandle(hDevice);
	

}

