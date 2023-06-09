#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "header.h"

#define BUFFER_SIZE 64

void error(const char *message)
{
    perror(message);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        error("Error: Could not create socket");
    }

    // Set server details
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &(server_address.sin_addr)) <= 0)
    {
        error("Error: Invalid server IP address");
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        error("Error: Could not connect to server");
    }

    // Ask if user is ready to start the game
    printf(">>> Ready to start game? (y/n):");
    char response;
    scanf(" %c", &response);

    struct ClientMessageHeader client_header;
    if (response == 'n' || response == 'N')
    {
        // User doesn't want to start, terminate the client
        close(client_socket);
        return 0;
    }
    else if(response == 'y')
    {
        // Send an empty message to signal game start
        client_header.length = 0;
        strcpy(client_header.data, "");
        send(client_socket, &client_header, sizeof(client_header), 0);
    }

    // Receive and print the first game control packet
    struct ServerMessageHeader server_header;
    memset(&server_header, 0, sizeof(server_header));
    if (recv(client_socket, &server_header, sizeof(server_header), 0) == -1)
    {
        error("Error: Failed to receive game control packet");
    }
    printf(">>>%s\n", server_header.data);
    printf(">>>Incorrect Guesses:\n");
    printf(">>>\n");

    while (1)
    {
        // Ask for user's guess
        printf(">>> Letter to guess: ");
        if (scanf("%s", buffer) == EOF)
        {
            // Handle Ctrl+D input or incorrect termination
            printf(">>>\n????");
            break;
        }

        if (strlen(buffer) > 1 || !islower(buffer[0]))
        {
            printf(">>> Error! Please guess one lowercase letter.\n");
            continue;
        }

        // Convert guess to lowercase
        char guess = tolower(buffer[0]);

        // Send the guess to the server
        memset(&client_header, 0, sizeof(client_header));
        client_header.length = 1;
        strcpy(client_header.data, &guess);
        send(client_socket, &client_header, sizeof(client_header), 0);

        // Receive and print updated game information
        memset(&server_header, 0, sizeof(server_header));
        if (recv(client_socket, &server_header, sizeof(server_header), 0) == -1)
        {
            error("Error: Failed to receive game information");
        }

        if (server_header.msgFlag == 0)
        {
            printf("%.*s\n", server_header.wordLength, server_header.data);
            printf("%d\n",server_header.wordLength);
            printf(">>>Incorrect Guesses:");
            for (size_t i = server_header.wordLength; i < server_header.wordLength + server_header.numIncorrect; i++)
            {
                printf(" %c", server_header.data[i]);
            }
            printf("\n>>>\n");
        }
        else
        {
            printf(">>>%s\n", server_header.data);
            // Check if game is over
            if (strstr(server_header.data, "Game Over!") != NULL)
            {
                printf("1234553\n");
                break;
            }
        }
    }

    // Close the socket and terminate the client
    close(client_socket);
    return 0;
}
