// monitor.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

bool IsHandleValid(const HANDLE& hHandle)
{
        return (hHandle && (hHandle != INVALID_HANDLE_VALUE));
}


bool IsHandleSigned(const HANDLE& hHandle)
{
        if (!IsHandleValid(hHandle)) {
                return true;
        }

        return (WAIT_OBJECT_0 == WaitForSingleObject(hHandle, 0));
}

int main()
{
        HANDLE process_handle = 0;
        while (1)
        {
                if (IsHandleValid(process_handle))
                {
                        if (IsHandleSigned(process_handle))
                        {
                                CloseHandle(process_handle);
                                process_handle = 0;
                                printf("===================== process crashed \n");
                        }
                        else
                        {
                                //printf("run normally");
                        }
                }
                else {
                        PROCESS_INFORMATION pi = {};
                        STARTUPINFO si = {};
                        si.cb = sizeof(STARTUPINFO);
                        si.dwFlags = STARTF_FORCEOFFFEEDBACK;
                        si.wShowWindow = SW_HIDE;

                        wchar_t cmd[] = L"demo.exe";
                        BOOL bOK = CreateProcessW(cmd, cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
                        if (bOK) {
                                CloseHandle(pi.hThread);
                                process_handle = pi.hProcess;
                                printf("run process \n");
                        }
                }

                Sleep(3000);
        }


        return 0;
}