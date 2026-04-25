#include <windows.h>
#include <winternl.h>
#include <cstdio>
#include <cstdlib>
#include <conio.h>

// http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FTime%2FNtQueryTimerResolution.html
using NtQueryTimerResolution_t = NTSTATUS (NTAPI *) (
	OUT PULONG minRes,
	OUT PULONG maxRes,
	OUT PULONG currentRes
);

// http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FTime%2FNtSetTimerResolution.html
using NtSetTimerResolution_t = NTSTATUS (NTAPI *) (
	IN ULONG desiredRes,
	IN BOOLEAN SetRes,
	OUT PULONG currentRes
);

namespace TimerResolution {
	BOOL SetBestResolution() {
		HMODULE ntdll = GetModuleHandleA("ntdll.dll");
		if (not ntdll) {
			return FALSE;
		}

		NtQueryTimerResolution_t NtQueryTimerResolution = (NtQueryTimerResolution_t)GetProcAddress(ntdll, "NtQueryTimerResolution");
		if (not NtQueryTimerResolution) {
			return FALSE;
		}

		NtSetTimerResolution_t NtSetTimerResolution = (NtSetTimerResolution_t)GetProcAddress(ntdll, "NtSetTimerResolution");
		if (not NtSetTimerResolution) {
			return FALSE;
		}

		ULONG maxRes;
		{
			ULONG minRes, currentRes;

			NTSTATUS result = NtQueryTimerResolution(&minRes, &maxRes, &currentRes);
			if (not NT_SUCCESS(result)) {
				return FALSE;
			}
			printf("%-40s %.2lfms\n", "Current kernel clock resolution is:", (double)currentRes/10000);
		}

		{
			ULONG oldRes;

			NTSTATUS result = NtSetTimerResolution(maxRes, true, &oldRes);
			if (not NT_SUCCESS(result)) {
				return FALSE;
			}

			printf("%-40s %.2lfms\n", "Set kernel clock resolution to:", (double)maxRes/10000);
		}

		return TRUE;

	}
}

int main() {
	BOOL ok = TimerResolution::SetBestResolution();
	if (ok) {
		printf("Set new kernel clock resolution successfully, close the program to revert changes\n");
	}
	else {
		printf("Failed to set new kernel clock resolution\n");
		(void)_getch();
		return EXIT_FAILURE;
	}
	while (true) {
		Sleep(INFINITE);
	}
	return EXIT_SUCCESS;
}
