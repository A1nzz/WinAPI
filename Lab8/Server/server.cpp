#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    // Создание серверного сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу и порту
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Начало прослушивания
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port 12345...\n";

    // Ожидание клиентского подключения
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected\n";

    // Обработка обмена сообщениями
    char buffer[1024];
    int bytesReceived;

    while (true) {
        // Чтение данных от клиента
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cerr << "Client disconnected\n";
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Received from client: " << buffer << std::endl;
        
        // Ввод сообщения на сервере
        std::cout << "Enter response: ";
        std::cin.getline(buffer, sizeof(buffer));

        // Отправка ответа клиенту
        send(clientSocket, buffer, bytesReceived, 0);
    }

    // Закрытие сокетов и завершение работы с Winsock
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
