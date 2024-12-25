#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_ROOM 4
#define MAX_PLAYER 4
#define BUFFER_SIZE 1024

int availableRoom = 0;

typedef struct {
    int roomID;
    int numPlayer;
} Room;

Room rooms[MAX_ROOM];

void displayRoom(int clientSock) {
    char buffer[BUFFER_SIZE];
    memset(buffer,0,sizeof(buffer));
    snprintf(buffer,sizeof(buffer), "Available rooms:\n");

    availableRoom = 0;
    for (int i=0;i<MAX_ROOM;i++) {
        if (rooms[i].numPlayer > 0) {
            availableRoom = 1;
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "Room %d: %d players\n", rooms[i].roomID, rooms[i].numPlayer);
        }
    }

    if (availableRoom == 0) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "No room existing\n");
    }
    
    send(clientSock, buffer, strlen(buffer), 0);
}


void handleClient(int clientSock, struct sockaddr_in clientAddr) {
    char buffer[BUFFER_SIZE];
    int option, roomID;

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);

    send(clientSock, "Choose an option:\n1. Create room\n2. Join room\n", 47,0);
    recv(clientSock, buffer, sizeof(buffer),0);
    sscanf(buffer, "%d", &option);

    if (option == 1) {
        for (int i=0;i<MAX_ROOM;i++) {
            if (rooms[i].numPlayer == 0) {
                rooms[i].roomID = i + 1;
                rooms[i].numPlayer = 1;
                snprintf(buffer, sizeof(buffer), "Room %d created successfully.\n", rooms[i].roomID);
                send(clientSock, buffer, strlen(buffer), 0);

                printf("Client %s: %d created room %d\n", clientIP, clientPort, rooms[i].roomID);
                break;
            }
        }
    }
    else if (option == 2) {
        displayRoom(clientSock);

        if (availableRoom == 0) {
            close(clientSock);
        }
        else {
            send(clientSock, "Enter room ID to join: ", 23, 0);
            recv(clientSock, buffer, sizeof(buffer), 0);
            sscanf(buffer, "%d", &roomID);

            if (roomID <= 0 || roomID > MAX_ROOM) {
                send(clientSock, "Invalid room ID.\n", 17, 0);
            }
            else if (rooms[roomID-1].numPlayer > MAX_PLAYER) {
                send(clientSock, "Room is full.\n", 14, 0);
            }
            else {
                rooms[roomID-1].numPlayer += 1;
                snprintf(buffer, sizeof(buffer), "Join room %d successfully.\n", roomID);
                send(clientSock, buffer, strlen(buffer), 0);

                printf("Client %s: %d joined room %d", clientIP, clientPort, roomID);
            }
        }
    }

    close(clientSock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Have not provided enough parameter for program %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created\n");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bild failed");
        close(serverSock);
        exit(EXIT_FAILURE);
    }
    printf("Bind successfully\n");

    if (listen(serverSock,5) < 0) {
        perror("Listen failed");
        close(serverSock);
        exit(EXIT_FAILURE);
    }
    printf("Listen successfully\n");

    printf("Server listening on port %s\n", argv[1]);

    while (1) {
        clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSock < 0) {
            perror("Accept failed\n");
            continue;
        }

        printf("Client connected: %s - %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        handleClient(clientSock, clientAddr);
    }

    close(serverSock);
    return 0;
}
