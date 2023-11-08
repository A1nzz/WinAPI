#include <Windows.h>
#include <CommCtrl.h>
#include <string>

// Идентификаторы элементов управления
#define IDC_TREEVIEW 1001
#define IDC_LISTVIEW 1002
HWND hTreeView;
HWND hListView;

HWND g_hEdit;
HWND g_hEditNewName;
HWND g_hAddButton;
HWND g_hDeleteButton;
HWND g_hRenameButton;


HWND g_hEditKeyName;
HWND g_hEditValueName;
HWND g_hEditValueData;
HWND g_hAddValueButton;
HWND g_hDeleteValueButton;
HWND g_hEditValueButton;

void TraverseRegistry(HWND hTreeView, HKEY hKey, HTREEITEM hParentItem);

void AddRegistryKey();
void DeleteRegistryKeyByName(HWND hEdit);
void RenameRegistryKey(HWND, HWND);
void AddRegistryValue(HWND, HWND, HWND);
void EditRegistryValue(HWND, HWND, HWND);
void DeleteRegistryValue(HWND, HWND);

// Обработчик сообщений главного окна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_COMMAND:
	{
		if (reinterpret_cast<HWND>(lParam) == g_hAddButton)
		{
			AddRegistryKey();
		}
		else if (reinterpret_cast<HWND>(lParam) == g_hDeleteButton)
		{
			DeleteRegistryKeyByName(g_hEdit);
		}
		else if (reinterpret_cast<HWND>(lParam) == g_hRenameButton) {
			RenameRegistryKey(g_hEdit, g_hEditNewName);
		}
		else if (reinterpret_cast<HWND>(lParam) == g_hAddValueButton) {
			AddRegistryValue(g_hEditKeyName, g_hEditValueName, g_hEditValueData);
		}
		else if (reinterpret_cast<HWND>(lParam) == g_hEditValueButton) {
			EditRegistryValue(g_hEditKeyName, g_hEditValueName, g_hEditValueData);
		}
		else if (reinterpret_cast<HWND>(lParam) == g_hDeleteValueButton) {
			DeleteRegistryValue(g_hEditKeyName, g_hEditValueName);
		}
		break;
	}
	case WM_CREATE:
	{
		// Создание элемента управления TreeView
		hTreeView = CreateWindowEx(0, WC_TREEVIEW, L"", WS_CHILD | WS_VISIBLE | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS,
			0, 0, 300, 400, hWnd, reinterpret_cast<HMENU>(IDC_TREEVIEW), nullptr, nullptr);

		// Создание элемента управления ListView
		hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT,
			320, 0, 300, 400, hWnd, reinterpret_cast<HMENU>(IDC_LISTVIEW), nullptr, nullptr);

		// Добавление колонок в ListView
		LVCOLUMN lvColumn{};
		lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
		lvColumn.pszText = const_cast<LPWSTR>(L"Name");
		lvColumn.cx = 150;
		ListView_InsertColumn(hListView, 0, &lvColumn);

		lvColumn.pszText = const_cast <LPWSTR>(L"Value");
		lvColumn.cx = 150;
		ListView_InsertColumn(hListView, 1, &lvColumn);

		g_hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 620, 10, 150, 23, hWnd, nullptr, nullptr, nullptr);
		g_hEditNewName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 620, 70, 150, 23, hWnd, nullptr, nullptr, nullptr);

		g_hEditKeyName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 620, 100, 150, 23, hWnd, nullptr, nullptr, nullptr);
		g_hEditValueName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 620, 130, 150, 23, hWnd, nullptr, nullptr, nullptr);
		g_hEditValueData = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, nullptr, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 620, 160, 150, 23, hWnd, nullptr, nullptr, nullptr);

		g_hAddButton = CreateWindowEx(0, WC_BUTTON, L"Add", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 10, 75, 23, hWnd, nullptr, nullptr, nullptr);
		g_hDeleteButton = CreateWindowEx(0, WC_BUTTON, L"Delete", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 40, 75, 23, hWnd, nullptr, nullptr, nullptr);
		g_hRenameButton = CreateWindowEx(0, WC_BUTTON, L"Rename", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 70, 75, 23, hWnd, nullptr, nullptr, nullptr);

		g_hAddValueButton = CreateWindowEx(0, WC_BUTTON, L"AddVal", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 100, 75, 23, hWnd, nullptr, nullptr, nullptr);
		g_hDeleteValueButton = CreateWindowEx(0, WC_BUTTON, L"DelVal", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 130, 75, 23, hWnd, nullptr, nullptr, nullptr);
		g_hEditValueButton = CreateWindowEx(0, WC_BUTTON, L"EditVal", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 780, 160, 75, 23, hWnd, nullptr, nullptr, nullptr);

		HKEY hRootKey;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, nullptr, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS)
		{
			TraverseRegistry(hTreeView, hRootKey, nullptr);

			RegCloseKey(hRootKey);
		}

		break;
	}
	case WM_NOTIFY:
	{
		NMHDR* pHeader = reinterpret_cast<NMHDR*>(lParam);
		if (pHeader->idFrom == IDC_TREEVIEW)
		{
			if (pHeader->code == TVN_SELCHANGED)
			{
				ListView_DeleteAllItems(hListView);

				HTREEITEM hSelectedItem = TreeView_GetSelection(hTreeView);
				if (hSelectedItem != nullptr)
				{
					TVITEM item{};
					item.hItem = hSelectedItem;
					item.mask = TVIF_PARAM;
					TreeView_GetItem(hTreeView, &item);

					item.mask = TVIF_TEXT;
					item.pszText = new wchar_t[256];
					item.cchTextMax = 256;
					TreeView_GetItem(hTreeView, &item);
					std::wstring keyName = item.pszText;

					HKEY hSelectedKey;
					if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName.c_str(), 0, KEY_READ | KEY_QUERY_VALUE, &hSelectedKey) == ERROR_SUCCESS)
					{
						DWORD dwValueCount;
						if (RegQueryInfoKey(hSelectedKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &dwValueCount, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
						{
							for (DWORD i = 0; i < dwValueCount; ++i)
							{
								wchar_t valueName[256];
								DWORD valueNameSize = sizeof(valueName) / sizeof(valueName[0]);

								DWORD valueType;
								BYTE data[1024];
								DWORD dataSize = sizeof(data);

								if (RegEnumValue(hSelectedKey, i, valueName, &valueNameSize, nullptr, &valueType, data, &dataSize) == ERROR_SUCCESS)
								{
									LVITEM lvItem{};
									lvItem.mask = LVIF_TEXT;
									lvItem.pszText = valueName;
									lvItem.iItem = i;

									std::wstring valueString;
									switch (valueType)
									{
									case REG_SZ:
									case REG_EXPAND_SZ:
										valueString = reinterpret_cast<wchar_t*>(data);
										break;
									case REG_DWORD:
										valueString = std::to_wstring(*reinterpret_cast<DWORD*>(data));
										break;
									case REG_QWORD:
										valueString = std::to_wstring(*reinterpret_cast<ULONGLONG*>(data));
										break;
									default:
										valueString = L"";
										break;
									}

									lvItem.iSubItem = 0;
									ListView_InsertItem(hListView, &lvItem);

									lvItem.mask = LVIF_TEXT;
									lvItem.pszText = const_cast<LPWSTR>(valueString.c_str());
									lvItem.iItem = i;       // Номер элемента
									lvItem.iSubItem = 1;    // Номер колонки
									ListView_SetItem(hListView, &lvItem);
									int firstItemIndex = 0; // индекс первого элемента, который нужно перерисовать
									int lastItemIndex = ListView_GetItemCount(hListView) - 1; // индекс последнего элемента в списке
									ListView_RedrawItems(hListView, firstItemIndex, lastItemIndex);
								}
							}


						}
						else {

						}

						// Закрытие выбранного ключа
						RegCloseKey(hSelectedKey);
					}

					delete[] item.pszText;
				}
			}
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Функция для заполнения TreeView данными из реестра
void TraverseRegistry(HWND hTreeView, HKEY hKey, HTREEITEM hParentItem)
{
	wchar_t subKeyName[256];
	DWORD subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);

	// Перебор подключей в текущем ключе
	for (DWORD i = 0; RegEnumKeyEx(hKey, i, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS; ++i)
	{
		HKEY hSubKey;
		if (RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
		{
			TVINSERTSTRUCT tvInsert{};
			tvInsert.hParent = hParentItem;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
			tvInsert.item.pszText = subKeyName;
			tvInsert.item.lParam = reinterpret_cast<LPARAM>(hSubKey);

			HTREEITEM hNewItem = TreeView_InsertItem(hTreeView, &tvInsert);

			TraverseRegistry(hTreeView, hSubKey, hNewItem);

			RegCloseKey(hSubKey);
		}

		subKeyNameSize = sizeof(subKeyName) / sizeof(subKeyName[0]);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"SampleWindowClass";

	WNDCLASS wc{};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"Sample Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 900, 680, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
		return 0;

	ShowWindow(hWnd, nCmdShow);

	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


void AddRegistryKey()
{
	wchar_t keyName[256];
	GetWindowText(g_hEdit, keyName, 256);
	OutputDebugString(keyName);


	HKEY hSoftwareKey;
	RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_ALL_ACCESS, &hSoftwareKey);

	HKEY hNewKey;
	LONG result = RegCreateKeyEx(hSoftwareKey, keyName, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &hNewKey, nullptr);

	RegCloseKey(hNewKey);
	RegCloseKey(hSoftwareKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ключ успешно добавлен", L"Успех", MB_OK | MB_ICONINFORMATION);
	}

	TreeView_DeleteAllItems(hTreeView);

	HKEY hRootKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, nullptr, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS)
	{
		TraverseRegistry(hTreeView, hRootKey, nullptr);

		RegCloseKey(hRootKey);
	}
}


void DeleteRegistryKeyByName(HWND hEdit)
{
	int textLength = GetWindowTextLength(hEdit);
	if (textLength == 0)
	{
		MessageBox(nullptr, L"Введите имя ключа", L"Ошибка", MB_OK);
		return;
	}

	std::wstring keyName(textLength + 1, L'\0');

	GetWindowText(hEdit, &keyName[0], textLength + 1);

	HKEY hParentKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_WRITE, &hParentKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия родительского ключа", L"Ошибка", MB_OK);
		return;
	}

	LONG result = RegDeleteKey(hParentKey, keyName.c_str());

	RegCloseKey(hParentKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ключ успешно удален", L"Успех", MB_OK | MB_ICONINFORMATION);
	}
	else if (result == ERROR_FILE_NOT_FOUND)
	{
		MessageBox(nullptr, L"Ключ с указанным именем не найден", L"Ошибка", MB_OK);
	}
	else
	{
		MessageBox(nullptr, L"Ошибка при удалении ключа", L"Ошибка", MB_OK);
	}
	TreeView_DeleteAllItems(hTreeView);

	HKEY hRootKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, nullptr, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS)
	{
		TraverseRegistry(hTreeView, hRootKey, nullptr);

		RegCloseKey(hRootKey);
	}
}

void RenameRegistryKey(HWND hEditOldName, HWND hEditNewName)
{
	int oldNameLength = GetWindowTextLength(hEditOldName);
	if (oldNameLength == 0)
	{
		MessageBox(nullptr, L"Введите старое имя ключа", L"Ошибка", MB_OK);
		return;
	}

	int newNameLength = GetWindowTextLength(hEditNewName);
	if (newNameLength == 0)
	{
		MessageBox(nullptr, L"Введите новое имя ключа", L"Ошибка", MB_OK);
		return;
	}

	std::wstring oldKeyName(oldNameLength + 1, L'\0');
	std::wstring newKeyName(newNameLength + 1, L'\0');

	GetWindowText(hEditOldName, &oldKeyName[0], oldNameLength + 1);

	GetWindowText(hEditNewName, &newKeyName[0], newNameLength + 1);

	HKEY hParentKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_WRITE, &hParentKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия родительского ключа", L"Ошибка", MB_OK);
		return;
	}

	LONG result = RegRenameKey(hParentKey, oldKeyName.c_str(), newKeyName.c_str());

	RegCloseKey(hParentKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ключ успешно переименован", L"Успех", MB_OK | MB_ICONINFORMATION);
	}
	else if (result == ERROR_FILE_NOT_FOUND)
	{
		MessageBox(nullptr, L"Ключ с указанным старым именем не найден", L"Ошибка", MB_OK);
	}
	else if (result == ERROR_ALREADY_EXISTS)
	{
		MessageBox(nullptr, L"Ключ с указанным новым именем уже существует", L"Ошибка", MB_OK);
	}
	else
	{
		MessageBox(nullptr, L"Ошибка при переименовании ключа", L"Ошибка", MB_OK);
	}
	TreeView_DeleteAllItems(hTreeView);


	HKEY hRootKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, nullptr, 0, KEY_READ, &hRootKey) == ERROR_SUCCESS)
	{
		TraverseRegistry(hTreeView, hRootKey, nullptr);

		RegCloseKey(hRootKey);
	}

}

void AddRegistryValue(HWND hEditKeyName, HWND hEditValueName, HWND hEditValueData)
{
	int keyNameLength = GetWindowTextLength(hEditKeyName);
	if (keyNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя ключа", L"Ошибка", MB_OK);
		return;
	}

	int valueNameLength = GetWindowTextLength(hEditValueName);
	if (valueNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя данных", L"Ошибка", MB_OK);
		return;
	}

	int valueDataLength = GetWindowTextLength(hEditValueData);
	if (valueDataLength == 0)
	{
		MessageBox(nullptr, L"Введите значение данных", L"Ошибка", MB_OK);
		return;
	}

	std::wstring keyName(keyNameLength + 1, L'\0');
	std::wstring valueName(valueNameLength + 1, L'\0');
	std::wstring valueData(valueDataLength + 1, L'\0');

	GetWindowText(hEditKeyName, &keyName[0], keyNameLength + 1);

	GetWindowText(hEditValueName, &valueName[0], valueNameLength + 1);

	GetWindowText(hEditValueData, &valueData[0], valueDataLength + 1);

	HKEY hParentKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_SET_VALUE, &hParentKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия родительского ключа", L"Ошибка", MB_OK);
		return;
	}

	HKEY hKey;
	if (RegOpenKeyEx(hParentKey, keyName.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия ключа", L"Ошибка", MB_OK);
		return;
	}

	LONG result = RegSetValueEx(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()), (valueDataLength + 1) * sizeof(wchar_t));

	RegCloseKey(hKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Данные успешно добавлены или обновлены", L"Успех", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(nullptr, L"Ошибка при добавлении или обновлении данных", L"Ошибка", MB_OK);
	}
}

void EditRegistryValue(HWND hEditKeyName, HWND hEditValueName, HWND hEditValueData)
{
	int keyNameLength = GetWindowTextLength(hEditKeyName);
	if (keyNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя ключа", L"Ошибка", MB_OK);
		return;
	}



	int valueNameLength = GetWindowTextLength(hEditValueName);
	if (valueNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя данных", L"Ошибка", MB_OK);
		return;
	}

	int valueDataLength = GetWindowTextLength(hEditValueData);
	if (valueDataLength == 0)
	{
		MessageBox(nullptr, L"Введите значение данных", L"Ошибка", MB_OK);
		return;
	}

	std::wstring keyName(keyNameLength + 1, L'\0');
	std::wstring valueName(valueNameLength + 1, L'\0');
	std::wstring valueData(valueDataLength + 1, L'\0');

	GetWindowText(hEditKeyName, &keyName[0], keyNameLength + 1);

	GetWindowText(hEditValueName, &valueName[0], valueNameLength + 1);

	GetWindowText(hEditValueData, &valueData[0], valueDataLength + 1);

	HKEY hParentKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_WRITE, &hParentKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия родительского ключа", L"Ошибка", MB_OK);
		return;
	}

	HKEY hKey;
	if (RegOpenKeyEx(hParentKey, keyName.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия ключа", L"Ошибка", MB_OK);
		return;
	}

	LONG result = RegSetValueEx(hKey, valueName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(valueData.c_str()), (valueDataLength + 1) * sizeof(wchar_t));

	RegCloseKey(hParentKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Данные успешно изменены", L"Успех", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(nullptr, L"Ошибка при изменении данных", L"Ошибка", MB_OK);
	}
}

void DeleteRegistryValue(HWND hEditKeyName, HWND hEditValueName)
{
	int keyNameLength = GetWindowTextLength(hEditKeyName);
	if (keyNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя ключа", L"Ошибка", MB_OK);
		return;
	}

	int valueNameLength = GetWindowTextLength(hEditValueName);
	if (valueNameLength == 0)
	{
		MessageBox(nullptr, L"Введите имя значения", L"Ошибка", MB_OK);
		return;
	}

	std::wstring keyName(keyNameLength + 1, L'\0');
	std::wstring valueName(valueNameLength + 1, L'\0');

	GetWindowText(hEditKeyName, &keyName[0], keyNameLength + 1);

	GetWindowText(hEditValueName, &valueName[0], valueNameLength + 1);

	HKEY hParentKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software", 0, KEY_WRITE, &hParentKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия родительского ключа", L"Ошибка", MB_OK);
		return;
	}
	HKEY hKey;
	if (RegOpenKeyEx(hParentKey, keyName.c_str(), 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Ошибка открытия ключа", L"Ошибка", MB_OK);
		return;
	}

	LONG result = RegDeleteValue(hKey, valueName.c_str());

	RegCloseKey(hKey);

	if (result == ERROR_SUCCESS)
	{
		MessageBox(nullptr, L"Значение успешно удалено", L"Успех", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(nullptr, L"Ошибка при удалении значения", L"Ошибка", MB_OK);
	}
}