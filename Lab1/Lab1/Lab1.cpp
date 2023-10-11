#include <windows.h>


// Идентификаторы элементов управления
#define ID_EDIT 1001
#define ID_FILE_NEW 2001
#define ID_FILE_OPEN 2002
#define ID_FILE_SAVE 2003
#define ID_NEW_WINDOW 2004
#define ID_STYLE_BOLD 3001
#define ID_STYLE_ITALIC 3002
#define ID_STYLE_UNDERLINE 3003
#define ID_BACKGROUND_COLOR 4000
#define ID_TEXT_COLOR 4001
#define ID_FONT_FACE_TNR 5001
#define ID_FONT_FACE_CALIBRI 5002
#define ID_FONT_FACE_ARIAL 5003
#define ID_FONT_SIZE_10 6010
#define ID_FONT_SIZE_12 6012
#define ID_FONT_SIZE_14 6014
#define ID_FONT_SIZE_16 6016
#define ID_FONT_SIZE_18 6018
#define ID_FONT_SIZE_20 6020

#define ID_BIN_SAVE 7001
#define ID_BIN_UPLOAD 7002



bool g_bBold = false;
bool g_bItalic = false;
bool g_bUnderline = false;

// Глобальные переменные
HWND g_hMainWindow; // Основное окно приложения
HWND g_hEdit; // Редактор текста
int g_nDocCount = 1; // Счетчик документов
TCHAR g_szCurrentFile[MAX_PATH] = TEXT(""); // Текущий открытый файл
HBRUSH hEditBgBrush = NULL;
HBRUSH hEditTextColorBrush = NULL;
static COLORREF bgColor = RGB(255, 255, 255); // Цвет фона по умолчанию
static COLORREF textColor = RGB(0, 0, 0); // Цвет фона по умолчанию
const wchar_t* g_FontFace = L"Arial";
int g_fontSize = 16;
HANDLE g_hFileMapping;
LPVOID g_lpFileBase;
DWORD fileSize = 0;



// Прототипы функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateNewDocument();
void OpenDocument();
void SaveDocument();
void UpdateWindowTitle();
void CreateNeWindow();
void UpdateTextStyle(HWND hWnd);
void ChangeBgc(HWND hwnd);
void ChangeTextColor(HWND hwnd);
void WriteToFile();
void ReadFromFile();

// Прототип функции-перехватчика
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

// Глобальная переменная для хранения хука
HHOOK g_hHook = NULL;

// Точка входа в программу
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
	if (g_hHook == NULL)
	{
		// Обработка ошибки
		return 1;
	}
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
	HMENU hStyleMenu = CreateMenu();
	HMENU hFontMenu = CreateMenu();
	HMENU hFontSize = CreateMenu();

	AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New");
	AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open");
	AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save");
	AppendMenu(hFileMenu, MF_STRING, ID_NEW_WINDOW, L"&New WIndow");
	AppendMenu(hFileMenu, MF_STRING, ID_BIN_SAVE, L"&Bin Save");
	AppendMenu(hFileMenu, MF_STRING, ID_BIN_UPLOAD, L"&Bin Open");

	AppendMenu(hStyleMenu, MF_STRING, ID_STYLE_BOLD, L"&Bold");
	AppendMenu(hStyleMenu, MF_STRING, ID_STYLE_ITALIC, L"&Italic");
	AppendMenu(hStyleMenu, MF_STRING, ID_STYLE_UNDERLINE, L"&Underline");

	AppendMenu(hFontMenu, MF_STRING, ID_FONT_FACE_TNR, L"&Times New Roman");
	AppendMenu(hFontMenu, MF_STRING, ID_FONT_FACE_ARIAL, L"&Arial");
	AppendMenu(hFontMenu, MF_STRING, ID_FONT_FACE_CALIBRI, L"&Calibri");

	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_10, L"&10");
	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_12, L"&12");
	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_14, L"&14");
	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_16, L"&16");
	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_18, L"&18");
	AppendMenu(hFontSize, MF_STRING, ID_FONT_SIZE_20, L"&20");


	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hStyleMenu, L"&Style");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFontMenu, L"&Font Face");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFontSize, L"&Font Size");
	AppendMenu(hMenu, MF_STRING, ID_BACKGROUND_COLOR, L"&Background color");
	AppendMenu(hMenu, MF_STRING, ID_TEXT_COLOR, L"&Text color");

	SetMenu(g_hMainWindow, hMenu);

	ShowWindow(g_hMainWindow, nCmdShow);
	// Цикл обработки сообщений
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Удаление хука
	UnhookWindowsHookEx(g_hHook);

	return 0;
}

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hDefaultFont = nullptr; // Хранит дескриптор шрифта по умолчанию
	

	switch (uMsg)
	{
	case WM_CREATE:
	{
		// Создаем шрифт с дефолтными параметрами
		LOGFONT lf;
		ZeroMemory(&lf, sizeof(LOGFONT));
		lf.lfHeight = g_fontSize;
		lf.lfWeight = FW_NORMAL;
		lstrcpy(lf.lfFaceName, g_FontFace);

		hDefaultFont = CreateFontIndirect(&lf);
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
		SendMessage(g_hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hDefaultFont), MAKELPARAM(TRUE, 0));
		// Создание файла и отображение его в память
		g_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, L"MyMappedFile");
		if (!g_hFileMapping)
		{
			MessageBox(g_hMainWindow, L"Failed to create file mapping", L"Error", MB_OK | MB_ICONERROR);
			return -1;
		}
		g_lpFileBase = MapViewOfFile(g_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!g_lpFileBase)
		{
			MessageBox(g_hMainWindow, L"Failed to map view of file", L"Error", MB_OK | MB_ICONERROR);
			CloseHandle(g_hFileMapping);
			return -1;
		}

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
		if (LOWORD(wParam) == ID_BACKGROUND_COLOR)
		{
			ChangeBgc(hwnd);
		}
		if (LOWORD(wParam) == ID_TEXT_COLOR)
		{
			ChangeTextColor(hwnd);
		}
		 
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
		case ID_STYLE_BOLD:
			g_bBold = !g_bBold;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_STYLE_ITALIC:
			g_bItalic = !g_bItalic;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_STYLE_UNDERLINE:
			g_bUnderline = !g_bUnderline;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_FACE_ARIAL:
			g_FontFace = L"Arial";
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_FACE_CALIBRI:
			g_FontFace = L"Calibri";
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_FACE_TNR:
			g_FontFace = L"Times New Roman";
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_10:
			g_fontSize = 10;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_12:
			g_fontSize = 12;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_14:
			g_fontSize = 14;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_16:
			g_fontSize = 16;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_18:
			g_fontSize = 18;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_FONT_SIZE_20:
			g_fontSize = 20;
			UpdateTextStyle(g_hEdit);
			break;
		case ID_BIN_SAVE:
			WriteToFile();
			break;

		case ID_BIN_UPLOAD:
			ReadFromFile();
			break;
		}
		break;
	}
	case WM_CTLCOLOREDIT:
	{
		HDC hdcEdit = (HDC)wParam;
		SetTextColor(hdcEdit, textColor);
		SetBkColor(hdcEdit, bgColor);
		return reinterpret_cast<LRESULT>(hEditBgBrush);	
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
		if (g_lpFileBase != NULL)
		{
			UnmapViewOfFile(g_lpFileBase);
			g_lpFileBase = NULL;
		}

		if (g_hFileMapping != NULL)
		{
			CloseHandle(g_hFileMapping);
			g_hFileMapping = NULL;
		}
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

void UpdateTextStyle(HWND hWnd)
{
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));
	lf.lfHeight = g_fontSize; // высота шрифта
	if (g_bBold){lf.lfWeight = FW_BOLD;}
	if (g_bItalic) {
		lf.lfItalic = TRUE; // курсив
	}
	if (g_bUnderline) {
		lf.lfUnderline = TRUE; // подчеркивание
	}
	lstrcpy(lf.lfFaceName, g_FontFace); // имя шрифта

	HFONT hFont = CreateFontIndirect(&lf);
	SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), MAKELPARAM(TRUE, 0));

	// Обновляем окно
	UpdateWindow(hWnd);
}

void ChangeBgc(HWND hwnd) {
	
	// Открываем диалоговое окно цвета
	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	static COLORREF customColors[16] = { 0 };
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hwnd;
	cc.rgbResult = bgColor;
	cc.lpCustColors = customColors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
	{
		bgColor = cc.rgbResult;
		hEditBgBrush = CreateSolidBrush(cc.rgbResult);
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

void ChangeTextColor(HWND hwnd) {
	// Открываем диалоговое окно цвета
	CHOOSECOLOR cc;
	ZeroMemory(&cc, sizeof(CHOOSECOLOR));
	static COLORREF customColors[16] = { 0 };
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hwnd;
	cc.rgbResult = textColor;
	cc.lpCustColors = customColors;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc))
	{
		textColor = cc.rgbResult;
		hEditTextColorBrush = CreateSolidBrush(cc.rgbResult);
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

// Функция-перехватчик клавиатуры
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// Извлечение информации о событии клавиатуры
		KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;

		// Проверка типа события
		if (wParam == WM_KEYDOWN)
		{
			// Проверка нажатия комбинации клавиш Ctrl+N
			if ((pKeyboardStruct->vkCode == 'N') && (GetAsyncKeyState(VK_CONTROL) & 0x8000))
			{
				// Выполнение действия по созданию нового файла
				CreateNewDocument();
			}
			else if ((pKeyboardStruct->vkCode == 'O') && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
				OpenDocument();
			}
			else if ((pKeyboardStruct->vkCode == 'S') && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
				SaveDocument();
			}
		}
	}

	// Передача события дальше в цепочку хуков
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Функция для записи данных в файл
void WriteToFile()
{
	if (g_szCurrentFile[0] == TEXT('\0'))
	{
		// Если текущий файл пуст, вызываем диалог сохранения файла
		OPENFILENAME ofn = {};
		TCHAR szFileName[MAX_PATH] = TEXT("");

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = g_hMainWindow;
		ofn.lpstrFilter = TEXT("Text Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0");
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

// Функция для чтения данных из файла
void ReadFromFile()
{
	OPENFILENAME ofn;
	TCHAR szFileName[MAX_PATH] = TEXT("");

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hMainWindow;
	ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;


	if (GetOpenFileName(&ofn))
	{
		g_nDocCount++;
		lstrcpy(g_szCurrentFile, szFileName);
		SetWindowText(g_hMainWindow, g_szCurrentFile);
		HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD fileSize = GetFileSize(hFile, NULL);

			// Обновление размера представления файла в памяти
			UnmapViewOfFile(g_lpFileBase);
			CloseHandle(g_hFileMapping);

			g_hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, fileSize, L"SharedFileMapping");

			if (g_hFileMapping)
			{	
				LPSTR pFileDataA = (LPSTR)MapViewOfFile(g_hFileMapping, FILE_MAP_READ, 0, 0, fileSize);
				//g_lpFileBase = (LPSTR)MapViewOfFile(g_hFileMapping, FILE_MAP_READ, 0, 0, fileSize);
				if (g_lpFileBase)
				{


					int wideLength = MultiByteToWideChar(CP_UTF8, 0, pFileDataA, -1, NULL, 0);
					//UnmapViewOfFile(g_lpFileBase);
					g_lpFileBase = new WCHAR[wideLength];
					MultiByteToWideChar(CP_UTF8, 0, pFileDataA, -1, (LPWSTR)g_lpFileBase, wideLength);
					WCHAR* pszText = (WCHAR*)g_lpFileBase;
					SetWindowText(g_hEdit, pszText);

					// Перерисовка окна
					InvalidateRect(g_hMainWindow, NULL, TRUE);
				}
				else
				{
					MessageBox(g_hMainWindow, L"Failed to map view of file", L"Error", MB_OK | MB_ICONERROR);
					UnmapViewOfFile(g_lpFileBase);

					CloseHandle(g_hFileMapping);
				}
			}
			else
			{
				MessageBox(g_hMainWindow, L"Failed to create file mapping", L"Error", MB_OK | MB_ICONERROR);
				UnmapViewOfFile(g_lpFileBase);
				CloseHandle(g_hFileMapping);
			}

			CloseHandle(hFile);
		}
		else
		{
			MessageBox(g_hMainWindow, L"Failed to open file", L"Error", MB_OK | MB_ICONERROR);
		}
	}
	
}
