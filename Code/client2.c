#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_BUF 512

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in server;

    char serverIP[50];
    char roll[50], name[100];
    char sendData[MAX_BUF], response[MAX_BUF];

    printf("\n==== STUDENT ATTENDANCE CLIENT ====\n");

    printf("Enter Teacher (Server) IP: ");
    scanf("%s", serverIP);
    getchar();

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed!\n");
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket Error!\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(clientSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connection failed!\n");
        return 1;
    }

    printf("\nConnected\n");

    printf("Enter Roll Number: ");
    scanf("%s", roll);
    getchar();

    printf("Enter Student Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    sprintf(sendData, "%s,%s", roll, name); // Combine

    send(clientSocket, sendData, strlen(sendData), 0);

    int recvSize = recv(clientSocket, response, sizeof(response), 0);
    response[recvSize] = '\0';

    printf("\nServer Response: %s\n", response);

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

// gcc client2.c -o client2.exe -lws2_32
// ./client2.exe