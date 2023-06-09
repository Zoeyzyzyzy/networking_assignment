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

void print_game_state(struct ServerMessageHeader *server_header)
{
    if (server_header->msgFlag == 0 || server_header->msgFlag == 1)
    {
        printf(">>>");
        for (int i = 0; i < server_header->wordLength; i++)
        {
            printf("%c", server_header->data[i]);
        }
        printf("\n");
        printf(">>>Incorrect Guesses:");
        for (size_t i = server_header->wordLength; i < server_header->wordLength + server_header->numIncorrect; i++)
        {
            printf(" %c", server_header->data[i]);
        }
        printf("\n>>>\n");
    }
    else if (server_header->msgFlag != 0 && server_header->msgFlag != 1)
    {
        printf(">>> %s\n", server_header->data);
    }
}

void receive_game_control_packet(int server_socket, struct ServerMessageHeader *server_message)
{
    ssize_t bytes_received = recv(server_socket, server_message, sizeof(*server_message), 0);
    if (bytes_received == -1)
    {
        error("Error: Failed to receive game control packet");
    }
    else if (bytes_received == 0)
    {
        printf("Server closed the connection\n");
        exit(1);
    }
}

void receive_data_packet(int server_socket, struct ServerMessageHeader *server_message)
{
    ssize_t bytes_received = 0;
    size_t total_bytes = sizeof(*server_message);

    while (bytes_received < total_bytes)
    {
        ssize_t received = recv(server_socket, server_message + bytes_received, total_bytes - bytes_received, 0);
        if (received == -1)
        {
            error("Error: Failed to receive data packet");
        }
        else if (received == 0)
        {
            printf("Server closed the connection\n");
            exit(1);
        }

        bytes_received += received;
    }
}

void send_client_message(int server_socket, struct ClientMessageHeader *client_message)
{
    ssize_t bytes_sent = 0;
    size_t total_bytes = sizeof(*client_message);

    while (bytes_sent < total_bytes)
    {
        ssize_t sent = send(server_socket, client_message + bytes_sent, total_bytes - bytes_sent, 0);
        if (sent == -1)
        {
            error("Error: Failed to send client message");
        }

        bytes_sent += sent;
    }
}

void play_game(int server_socket)
{
    struct ServerMessageHeader server_message;
    struct ClientMessageHeader client_message;
    char buffer[BUFFER_SIZE];

    receive_game_control_packet(server_socket, &server_message);

    // Ask if user is ready to start the game
    printf(">>> Ready to start game? (y/n):");
    char response;
    scanf(" %c", &response);

    if (response == 'y' || response == 'Y')
    {
        // Send an empty message to signal game start
        client_message.length = 0;
        strcpy(client_message.data, "");
        send_client_message(server_socket, &client_message);
    }
    else
    {
        // User doesn't want to start, terminate the client
        close(server_socket);
        return;
    }
    print_game_state(&server_message);
    while (1)
    {
        // Ask for user's guess
        printf(">>> Letter to guess: ");
        if (scanf("%s", buffer) == EOF)
        {
            // Handle Ctrl+D input or incorrect termination
            printf("\n>>>????");
            break;
        }

        if (strlen(buffer) > 1 || !islower(buffer[0]))
        {
            printf(">>> Error! Please guess one lowercase letter.\n");
            continue;
        }

        // Convert guess to lowercase
        char guess = tolower(buffer[0]);

        // Send the client's guess to the server
        memset(&client_message, 0, sizeof(client_message));
        client_message.length = 1;
        client_message.data[0] = guess;
        send_client_message(server_socket, &client_message);

        // Receive the updated game state from the server
        memset(&server_message, 0, sizeof(server_message));
        receive_data_packet(server_socket, &server_message);

        printf("%d", server_message.numIncorrect);

        print_game_state(&server_message);

        if (server_message.msgFlag != 0 && server_message.msgFlag != 1)
        {
            break;
        }
    }
    // Receive the updated game state from the server
    receive_data_packet(server_socket, &server_message);
    print_game_state(&server_message);
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
    // char buffer[BUFFER_SIZE];

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

    play_game(client_socket);

    // // Receive the game control packet from the server
    // struct ServerMessageHeader server_header;
    // memset(&server_header, 0, sizeof(server_header));
    // // Receive the updated game control packet from the server
    // if (recv(client_socket, &server_header, sizeof(server_header), 0) == -1)
    // {
    //     error("Error: Failed to receive game control packet");
    // }

    // // Ask if user is ready to start the game
    // printf(">>> Ready to start game? (y/n):");
    // char response;
    // scanf(" %c", &response);

    // struct ClientMessageHeader client_header;
    // if (response == 'y' || response == 'Y')
    // {
    //     // Send an empty message to signal game start
    //     client_header.length = 0;
    //     strcpy(client_header.data, "");
    //     send(client_socket, &client_header, sizeof(client_header), 0);
    // }
    // else
    // {
    //     // User doesn't want to start, terminate the client
    //     close(client_socket);
    //     return 0;
    // }

    // // printf(">>>%s\n", server_header.data);
    // // printf(">>>Incorrect Guesses:\n");
    // // printf(">>>\n");

    // while (1)
    // {
    //     print_game_state(&server_header);

    //     if (server_header.msgFlag != 0 && server_header.msgFlag != 1)
    //     {
    //         // Game over
    //         printf(">>>%s\n", server_header.data);
    //         // Check if game is over
    //         if (strstr(server_header.data, "Game Over!") != NULL)
    //         {
    //             printf("1234553\n");
    //             break;
    //         }
    //     }

    //     // Ask for user's guess
    //     printf(">>> Letter to guess: ");
    //     if (scanf("%s", buffer) == EOF)
    //     {
    //         // Handle Ctrl+D input or incorrect termination
    //         printf("\n>>>????");
    //         break;
    //     }

    //     if (strlen(buffer) > 1 || !islower(buffer[0]))
    //     {
    //         printf(">>> Error! Please guess one lowercase letter.\n");
    //         continue;
    //     }

    //     // Convert guess to lowercase
    //     char guess = tolower(buffer[0]);

    //     // Send the guess to the server
    //     memset(&client_header, 0, sizeof(client_header));
    //     client_header.length = 1;
    //     strcpy(client_header.data, &guess);
    //     send(client_socket, &client_header, sizeof(client_header), 0);

    //     // Receive and print updated game information
    //     memset(&server_header, 0, sizeof(server_header));
    //     if (recv(client_socket, &server_header, sizeof(server_header), 0) == -1)
    //     {
    //         error("Error: Failed to receive game information");
    //     }

    //     // if (server_header.msgFlag == 0)
    //     // {
    //     //     printf("%.*s\n", server_header.wordLength, server_header.data);
    //     //     printf("%d\n", server_header.wordLength);
    //     //     printf(">>>Incorrect Guesses:");
    //     //     for (size_t i = server_header.wordLength; i < server_header.wordLength + server_header.numIncorrect; i++)
    //     //     {
    //     //         printf(" %c", server_header.data[i]);
    //     //     }
    //     //     printf("\n>>>\n");
    //     // }
    //     // else
    //     // {
    //     //     printf(">>>%s\n", server_header.data);
    //     //     // Check if game is over
    //     //     if (strstr(server_header.data, "Game Over!") != NULL)
    //     //     {
    //     //         printf("1234553\n");
    //     //         break;
    //     //     }
    //     // }
    // }

    // // Print the final game state
    // print_game_state(&server_header);

    // Close the socket and terminate the client
    close(client_socket);
    return 0;
}
