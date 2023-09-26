#include <windows.h>

// Идентификаторы элементов управления
#define ID_EDIT 1001
#define ID_FILE_NEW 2001
#define ID_FILE_OPEN 2002
#define ID_FILE_SAVE 2003
#define ID_NEW_WINDOW 2004

// Глобальные переменные
HWND g_hMainWindow; // Основное окно приложения
HWND g_hEdit; // Редактор текста
int g_nDocCount = 1; // Счетчик документов
TCHAR g_szCurrentFile[MAX_PATH] = TEXT(""); // Текущий открытый файл

// Прототипы функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateNewDocument();
void OpenDocument();
void SaveDocument();
void UpdateWindowTitle();
void CreateNeWindow();

// Точка входа в программу
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"TextEditorClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Создание главного окна
    g_hMainWindow = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Text Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (g_hMainWindow == NULL)
    {
        return 0;
    }

    // Создание меню
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();

    AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open");
    AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save");
    AppendMenu(hFileMenu, MF_STRING, ID_NEW_WINDOW, L"&New WIndow");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    SetMenu(g_hMainWindow, hMenu);

    ShowWindow(g_hMainWindow, nCmdShow);
    // Цикл обработки сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Создание редактора текста
        g_hEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 0, 0,
            hwnd,
            (HMENU)ID_EDIT,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );
        break;
    }
    case WM_SIZE:
    {
        // Перерасчет размеров редактора текста при изменении размеров окна
        int nWidth = LOWORD(lParam);
        int nHeight = HIWORD(lParam);

        MoveWindow(g_hEdit, 0, 0, nWidth, nHeight, TRUE);
        break;
    }
    case WM_COMMAND:
    {
        // Обработка командных сообщений
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_FILE_NEW:
            CreateNewDocument();
            break;
        case ID_FILE_OPEN:
            OpenDocument();
            break;
        case ID_FILE_SAVE:
            SaveDocument();
            break;

        case ID_NEW_WINDOW:
            CreateNeWindow();
            break;
        }
        break;
    }
    case WM_CLOSE:
    {
        
            // Если пользователь соглашается сохранить изменения, закройте текущее окно
            if (MessageBox(hwnd, L"Do you want to save the changes?", L"Save Changes", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                SaveDocument();
            }
            DestroyWindow(hwnd);
        
        return 0;
    }
    case WM_DESTROY:
    {
        // Обработка сообщения разрушения окна
        PostQuitMessage(0);
        break;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateNewDocument()
{
    g_nDocCount++;
    g_szCurrentFile[0] = TEXT('\0');
    SetWindowText(g_hMainWindow, L"Text Editor - New Document");

    SetWindowText(g_hEdit, L"");
}

// Открытие документа
void OpenDocument()
{
    OPENFILENAME ofn = {};
    TCHAR szFileName[MAX_PATH] = TEXT("");

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = g_hMainWindow;
    ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

   

    if (GetOpenFileName(&ofn))
    {
        
        
            // Открытие файла в текущем окне
            g_nDocCount++;
            lstrcpy(g_szCurrentFile, szFileName);
            SetWindowText(g_hMainWindow, g_szCurrentFile);

            HANDLE hFile = CreateFile(g_szCurrentFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                DWORD dwFileSize = GetFileSize(hFile, NULL);
                if (dwFileSize != INVALID_FILE_SIZE)
                {
                    LPSTR lpFileData = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
                    if (lpFileData != NULL)
                    {
                        DWORD dwBytesRead;
                        if (ReadFile(hFile, lpFileData, dwFileSize, &dwBytesRead, NULL))
                        {
                            lpFileData[dwBytesRead] = '\0';
                            SetWindowTextA(g_hEdit, lpFileData);
                        }
                        GlobalFree(lpFileData);
                    }
                }
                CloseHandle(hFile);
            }
        
    }
}

// Сохранение документа
void SaveDocument()
{
    if (g_szCurrentFile[0] == TEXT('\0'))
    {
        // Если текущий файл пуст, вызываем диалог сохранения файла
        OPENFILENAME ofn = {};
        TCHAR szFileName[MAX_PATH] = TEXT("");

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = g_hMainWindow;
        ofn.lpstrFilter = TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn))
        {
            lstrcpy(g_szCurrentFile, szFileName);
            SetWindowText(g_hMainWindow, g_szCurrentFile);
        }
        else
        {
            return;
        }
    }

    HANDLE hFile = CreateFile(g_szCurrentFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        int nLength = GetWindowTextLength(g_hEdit);
        LPSTR lpFileData = (LPSTR)GlobalAlloc(GPTR, nLength + 1);
        if (lpFileData != NULL)
        {
            GetWindowTextA(g_hEdit, lpFileData, nLength + 1);

            DWORD dwBytesWritten;
            WriteFile(hFile, lpFileData, nLength, &dwBytesWritten, NULL);

            GlobalFree(lpFileData);
        }
        CloseHandle(hFile);
    }
}

// Обновление заголовка окна
void UpdateWindowTitle()
{
    TCHAR szTitle[MAX_PATH];
    if (g_szCurrentFile[0] == TEXT('\0'))
    {
        wsprintf(szTitle, TEXT("Text Editor - New Document %d"), g_nDocCount);
    }
    else
    {
        wsprintf(szTitle, TEXT("Text Editor - %s"), g_szCurrentFile);
    }
    SetWindowText(g_hMainWindow, szTitle);
}

void CreateNeWindow() {
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};

    // Создание нового процесса с текущим исполняемым файлом
    if (CreateProcess(NULL, GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}