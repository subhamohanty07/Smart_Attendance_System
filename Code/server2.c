#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_BUF 512

// Time window (24hr format)
#define START_HOUR 10
#define START_MIN 55
#define END_HOUR 11
#define END_MIN 10

// Check if current time is inside the allowed attendance window
int isAttendanceAllowed()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    int currentMin = t->tm_hour * 60 + t->tm_min;
    int startMin = START_HOUR * 60 + START_MIN;
    int endMin = END_HOUR * 60 + END_MIN;

    return (currentMin >= startMin && currentMin <= endMin);
}

// Checks if roll already exists in file
int isRollDuplicate(const char *roll)
{
    FILE *fp = fopen("attendance.txt", "r");
    if (!fp)
        return 0;

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, roll, strlen(roll)) == 0)
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main()
{
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
    int c;
    char buffer[MAX_BUF];
    char clientIP[50];

    printf("\n==== SMART ATTENDANCE SERVER STARTED ====\n");

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed!\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed!\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed!\n");
        return 1;
    }

    listen(serverSocket, 3);
    printf("Server is waiting for students...\n");

    c = sizeof(client);

    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&client, &c);

        strcpy(clientIP, inet_ntoa(client.sin_addr)); // Get student IP

        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);

        // Data format = "ROLL,NAME"
        char *roll = strtok(buffer, ",");
        char *name = strtok(NULL, ",");

        printf("\nRequest from IP: %s\n", clientIP);
        printf("Roll: %s, Name: %s\n", roll, name);

        //  Check time restriction
        if (!isAttendanceAllowed())
        {
            char msg[] = "Attendance Window Closed!";
            send(clientSocket, msg, strlen(msg), 0);
            closesocket(clientSocket);
            continue;
        }

        //  Check duplicate roll
        if (isRollDuplicate(roll))
        {
            char msg[] = "Attendance already marked for this roll!";
            send(clientSocket, msg, strlen(msg), 0);
            closesocket(clientSocket);
            continue;
        }

        //  Write attendance to file
        FILE *fp = fopen("attendance.txt", "a");
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        fprintf(fp, "%s\t%s\t\t%02d-%02d-%04d   %02d:%02d:%02d\n",
                roll, name,
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec);

        fclose(fp);

        char msg[] = "Attendance Marked Successfully!";
        send(clientSocket, msg, strlen(msg), 0);

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

// gcc server2.c -o server2.exe -lws2_32
//./server2.exe