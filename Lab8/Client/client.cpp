#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> 

#pragma comment(lib, "ws2_32.lib")

int main() {
    // ������������� Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    // �������� ����������� ������
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    // ��������� ������ �������
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    serverAddr.sin_port = htons(12345);

    // ����������� � �������
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server\n";

    // ����� �����������
    char message[1024];

    while (true) {
        // ���� ���������
        std::cout << "Enter message: ";
        std::cin.getline(message, sizeof(message));

        // �������� ��������� �������
        send(clientSocket, message, strlen(message), 0);

        // ��������� ������ �� �������
        int bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cerr << "Server disconnected\n";
            break;
        }

        message[bytesReceived] = '\0';
        std::cout << "Server response: " << message << std::endl;
    }

    // �������� ������ � ���������� ������ � Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
