
#include <windows.h>
#include <tlhelp32.h>


// Идентификаторы элементов управления
#define BTN_REFRESH 101
#define BTN_SUSPEND 102
#define BTN_RESUME 103
#define BTN_TERMINATE 104
#define LST_PROCESSES 105

// Прототипы функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void RefreshProcessList(HWND hwnd);
void SuspendProcess(HWND hwnd);
void ResumeProcess(HWND hwnd);
void TerminateProcess(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"ProcessManagerClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(
        0,                      // дополнительные стили окна
        CLASS_NAME,             // имя класса окна
        L"Process Manager",     // заголовок окна
        WS_OVERLAPPEDWINDOW,    // стиль окна
        CW_USEDEFAULT,          // позиция X окна
        CW_USEDEFAULT,          // позиция Y окна
        500,                    // ширина окна
        400,                    // высота окна
        NULL,                   // родительское окно
        NULL,                   // дескриптор меню
        hInstance,              // дескриптор приложения
        NULL                    // указатель на данные создания окна
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Создание элементов управления
    CreateWindow(
        L"BUTTON",
        L"Refresh",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 100, 30,
        hwnd,
        reinterpret_cast<HMENU>(BTN_REFRESH),
        hInstance,
        NULL
    );

    CreateWindow(
        L"BUTTON",
        L"Suspend",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        120, 10, 100, 30,
        hwnd,
        reinterpret_cast<HMENU>(BTN_SUSPEND),
        hInstance,
        NULL
    );

    CreateWindow(
        L"BUTTON",
        L"Resume",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        230, 10, 100, 30,
        hwnd,
        reinterpret_cast<HMENU>(BTN_RESUME),
        hInstance,
        NULL
    );

    CreateWindow(
        L"BUTTON",
        L"Terminate",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        340, 10, 100, 30,
        hwnd,
        reinterpret_cast<HMENU>(BTN_TERMINATE),
        hInstance,
        NULL
    );

    HWND lstProcesses = CreateWindow(
        L"LISTBOX",
        NULL,
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_VSCROLL,
        10, 50, 460, 290,
        hwnd,
        reinterpret_cast<HMENU>(LST_PROCESSES),
        hInstance,
        NULL
    );

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Основной цикл сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case BTN_REFRESH:
                RefreshProcessList(hwnd);
                break;
            case BTN_SUSPEND:
                SuspendProcess(hwnd);
                break;
            case BTN_RESUME:
                ResumeProcess(hwnd);
                break;
            case BTN_TERMINATE:
                TerminateProcess(hwnd);
                break;
            }
        }
        break;
    }
    case WM_MOUSEWHEEL:
    {
        HWND lstProcesses = GetDlgItem(hwnd, LST_PROCESSES);
        SendMessage(lstProcesses, uMsg, wParam, lParam);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void RefreshProcessList(HWND hwnd)
{
    HWND lstProcesses = GetDlgItem(hwnd, LST_PROCESSES);
    SendMessage(lstProcesses, LB_RESETCONTENT, 0, 0);

    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        SendMessage(lstProcesses, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pe32.szExeFile));
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void SuspendProcess(HWND hwnd)
{
    HWND lstProcesses = GetDlgItem(hwnd, LST_PROCESSES);
    int selectedIndex = SendMessage(lstProcesses, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR)
    {
        return;
    }

    wchar_t processName[MAX_PATH];
    SendMessage(lstProcesses, LB_GETTEXT, selectedIndex, reinterpret_cast<LPARAM>(processName));

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (wcscmp(pe32.szExeFile, processName) == 0)
            {
                HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                THREADENTRY32 te32;
                te32.dwSize = sizeof(THREADENTRY32);

                if (Thread32First(hThreadSnapshot, &te32))
                {
                    do
                    {
                        if (te32.th32OwnerProcessID == pe32.th32ProcessID)
                        {
                            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                            if (hThread != NULL)
                            {
                                SuspendThread(hThread);
                                CloseHandle(hThread);
                            }
                        }
                    } while (Thread32Next(hThreadSnapshot, &te32));
                }

                CloseHandle(hThreadSnapshot);
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}
void ResumeProcess(HWND hwnd)
{
    HWND lstProcesses = GetDlgItem(hwnd, LST_PROCESSES);
    int selectedIndex = SendMessage(lstProcesses, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR)
    {
        return;
    }

    wchar_t processName[MAX_PATH];
    SendMessage(lstProcesses, LB_GETTEXT, selectedIndex, reinterpret_cast<LPARAM>(processName));

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (wcscmp(pe32.szExeFile, processName) == 0)
            {
                HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                THREADENTRY32 te32;
                te32.dwSize = sizeof(THREADENTRY32);

                if (Thread32First(hThreadSnapshot, &te32))
                {
                    do
                    {
                        if (te32.th32OwnerProcessID == pe32.th32ProcessID)
                        {
                            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                            if (hThread != NULL)
                            {
                                ResumeThread(hThread);
                                CloseHandle(hThread);
                            }
                        }
                    } while (Thread32Next(hThreadSnapshot, &te32));
                }

                CloseHandle(hThreadSnapshot);
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}

void TerminateProcess(HWND hwnd)
{
    HWND lstProcesses = GetDlgItem(hwnd, LST_PROCESSES);
    int selectedIndex = SendMessage(lstProcesses, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR)
    {
        return;
    }

    wchar_t processName[MAX_PATH];
    SendMessage(lstProcesses, LB_GETTEXT, selectedIndex, reinterpret_cast<LPARAM>(processName));

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (wcscmp(pe32.szExeFile, processName) == 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess != NULL)
                {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}