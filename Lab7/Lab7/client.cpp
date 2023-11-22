#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // Äëÿ InetPton

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;

    if (InetPton(AF_INET, TEXT("127.0.0.1"), &(serverAddr.sin_addr)) != 1) {
        std::cerr << "Invalid address/Address not supported\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server\n";

    char message[1024];

    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(message, sizeof(message));

        send(clientSocket, message, strlen(message), 0);

        int bytesReceived = recv(clientSocket, message, sizeof(message), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Recv failed with error\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        message[bytesReceived] = '\0';

        std::cout << "Client2 response: " << message << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
