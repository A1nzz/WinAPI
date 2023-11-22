#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> 

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    // Создание клиентского сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    // Установка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    serverAddr.sin_port = htons(12345);

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server\n";

    // Обмен сообщениями
    char message[1024];

    while (true) {
        // Ввод сообщения
        std::cout << "Enter message: ";
        std::cin.getline(message, sizeof(message));

        // Отправка сообщения серверу
        send(clientSocket, message, strlen(message), 0);

        // Получение ответа от сервера
        int bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cerr << "Server disconnected\n";
            break;
        }

        message[bytesReceived] = '\0';
        std::cout << "Server response: " << message << std::endl;
    }

    // Закрытие сокета и завершение работы с Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
