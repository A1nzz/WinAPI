#include <windows.h>
#include <vector>
#include <thread>

const int NUM_CARS = 5;
const int CAR_WIDTH = 50;
const int CAR_HEIGHT = 30;
const int TRACK_WIDTH = 800;
const int TRACK_HEIGHT = 400;

// Глобальные переменные
HWND g_hWnd;
HDC g_hDC;
std::vector<int> g_carPositions;
HANDLE g_hMutex;

// Функция выбора цвета автомобиля
COLORREF GetCarColor(int index) {
    if (index == 0) {
        return RGB(255, 0, 0);
    } else if (index == 1) {
        return RGB(0, 255, 0);
    } else if (index == 2) {
        return RGB(0, 0, 255);
    } else if (index == 3) {
        return RGB(100, 0, 100);
    } else if (index == 4) {
        return RGB(0, 110, 0);
    }
}

// Функция рисования автомобилей
void DrawCars()
{
    int gap = 0;

    WaitForSingleObject(g_hMutex, INFINITE); // Ждем доступа к общему ресурсу

    for (int i = 0; i < NUM_CARS; i++)
    {
        RECT rect;
        rect.left = g_carPositions[i];
        rect.right = rect.left + CAR_WIDTH;
        rect.top = gap; // Устанавливаем вертикальную позицию
        rect.bottom = CAR_HEIGHT + gap; // Устанавливаем вертикальную позицию

        HBRUSH hBrush = CreateSolidBrush(GetCarColor(i));
        FillRect(g_hDC, &rect, hBrush);
        DeleteObject(hBrush);
        gap += 50;
    }

    ReleaseMutex(g_hMutex); // Освобождаем доступ к общему ресурсу
}

// Функция обновления позиций автомобилей
void UpdateCarPositions(int carIndex)
{
    while (true)
    {
        WaitForSingleObject(g_hMutex, INFINITE); // Ждем доступа к общему ресурсу

        g_carPositions[carIndex] += rand() % 5 + 1; // Генерация случайного шага
        if (g_carPositions[carIndex] >= TRACK_WIDTH) // Если автомобиль достиг конца трека, вернуть его в начало
            g_carPositions[carIndex] = 0;

        ReleaseMutex(g_hMutex); // Освобождаем доступ к общему ресурсу

        // Обновить окно
        InvalidateRect(g_hWnd, NULL, TRUE);
        UpdateWindow(g_hWnd);

        Sleep(100); // Задержка между обновлениями позиций
    }
}

// Функция обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        g_hDC = BeginPaint(hWnd, &ps);

        DrawCars();

        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Функция создания окна
HWND CreateAppWindow(HINSTANCE hInstance)
{
    const wchar_t CLASS_NAME[] = L"RaceWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    return CreateWindowEx(
        0,                              // дополнительные стили окна
        CLASS_NAME,                     // имя класса окна
        L"Гонка машин",                 // заголовок окна
        WS_OVERLAPPEDWINDOW,            // стиль окна
        CW_USEDEFAULT, CW_USEDEFAULT,   // позиция окна
        TRACK_WIDTH, TRACK_HEIGHT,      // размеры окна
        NULL,                           // дескриптор родительского окна
        NULL,                           // дескриптор меню
        hInstance,                      // дескриптор экземпляра приложения
        NULL                            // указатель на данные создания окна
    );
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hMutex = CreateMutex(NULL, FALSE, NULL); // Создаем мьютекс

    g_carPositions.resize(NUM_CARS, 0); // Инициализируем позиции автомобилей

    // Создаем потоки для каждой машины
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_CARS; i++)
    {
        threads.push_back(std::thread(UpdateCarPositions, i));
    }

    // Создаем окно приложения
    g_hWnd = CreateAppWindow(hInstance);
    if (g_hWnd == NULL)
        return 0;

    // Отображаем окно
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Запускаем цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Ожидаем завершения потоков
    for (auto& thread : threads)
    {
        thread.join();
    }

    CloseHandle(g_hMutex); // Закрываем мьютекс

    return static_cast<int>(msg.wParam);
}