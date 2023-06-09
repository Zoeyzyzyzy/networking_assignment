#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>
#include "header.h"

#define MAX_CLIENTS 3
#define BUFFER_SIZE 1024

void error(const char *message)
{
    perror(message);
    exit(1);
}

char *read_word_list()
{
    FILE *file = fopen("hangman_words.txt", "r");
    if (file == NULL)
    {
        error("Error: Failed to open word list file");
    }

    char line[BUFFER_SIZE];
    char *words[15];

    int word_count = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character
        words[word_count] = strdup(line);
        word_count++;
    }

    fclose(file);
    return words[rand() % word_count];
}

// void handle_client(int client_socket)
// {
//     struct ServerMessageHeader server_header;
//     server_header.msgFlag = 0;
//     char *word = read_word_list();
//     server_header.wordLength = strlen(word);
//     // char guess_word[server_header.wordLength];
//     int guesses_left = 6;
//     server_header.numIncorrect = 0;
//     char incorrect_guesses[BUFFER_SIZE] = "";

//     // Initialize the guessed word with underscores
//     memset(server_header.data, '_', server_header.wordLength);
//     server_header.data[server_header.wordLength] = '\0';
//     send(client_socket, &server_header, sizeof(server_header), 0);
// }

void *client_thread(void *arg)
{
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    struct ServerMessageHeader server_message;
    struct ClientMessageHeader client_message;

    // // Generate random words
    // char words[15][BUFFER_SIZE];
    // int word_count = load_words("hangman_words.txt", words, 15);

    // // Choose a word for the game
    // char chosen_word[BUFFER_SIZE];
    // int word_length;
    // choose_word(words, word_count, chosen_word, &word_length);

    char *chosen_word = read_word_list();
    int word_length = strlen(chosen_word);
    printf("%s\n", chosen_word);

    // Prepare the game control packet
    server_message.msgFlag = 1;
    server_message.wordLength = word_length;
    server_message.numIncorrect = 0;
    memset(server_message.data, '_', BUFFER_SIZE);
    server_message.data[word_length] = '\0';

    // Send the game control packet to the client
    send(client_socket, &server_message, sizeof(server_message), 0);

    while (1)
    {
        // Receive the client's guess
        int bytes_received = recv(client_socket, &client_message, sizeof(client_message), 0);
        if (bytes_received == -1)
        {
            error("Error: Could not receive data from client");
        }
        else if (bytes_received == 0)
        {
            // Client terminated connection (Ctrl+D)
            printf("\n");
            close(client_socket);
            pthread_exit(NULL);
            break;
        }

        // Update the game state
        int remain_words = word_length;
        int flag = 0;
        printf("111 %c\n", client_message.data[0]);
        for (int i = 0; i < word_length; i++)
        {
            if (tolower(chosen_word[i]) == client_message.data[0])
            {
                server_message.data[i] = chosen_word[i];
                flag = 1;
            }
            else if (server_message.data[i] == '_')
            {
                remain_words++;
            }
        }
        // Update the number of incorrect guesses
        if (flag == 0)
        {
            server_message.numIncorrect++;
            server_message.data[word_length] = client_message.data;
        }

        // Check if the game is over
        if (remain_words == 0 || server_message.numIncorrect >= 6)
        {
            // Game over
            if (remain_words == 0)
            {
                memset(&server_message, 0, sizeof(server_message));
                strncpy(server_message.data, "You Win!", BUFFER_SIZE);
                server_message.msgFlag = strlen(server_message.data);
            }
            else
            {
                memset(&server_message, 0, sizeof(server_message));
                strncpy(server_message.data, "You Lose :(", BUFFER_SIZE);
                server_message.msgFlag = strlen(server_message.data);
            }
            send(client_socket, &server_message, sizeof(server_message), 0);
            memset(&server_message, 0, sizeof(server_message));
            strncpy(server_message.data, "Game Over!", BUFFER_SIZE);
            server_message.msgFlag = strlen(server_message.data);
            send(client_socket, &server_message, sizeof(server_message), 0);
            break;
        }
        else
        {
            // Continue the game
            server_message.msgFlag = 0;
            // strncpy(server_message.data, "Incorrect Guesses:", BUFFER_SIZE);
            send(client_socket, &server_message, sizeof(server_message), 0);
        }
    }

    // Close the client socket
    close(client_socket);
    pthread_exit(NULL);

    return NULL;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_socket, client_sockets[MAX_CLIENTS];
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        error("Error: Could not create socket");
    }

    // Set server details
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        error("Error: Could not bind the socket");
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        error("Error: Could not listen for connections");
    }

    printf("Hangman Game Server started. Waiting for connections...\n");

    int num_clients = 0;

    while (1)
    {
        // Accept a new client connection
        socklen_t client_address_size = sizeof(client_address);
        int new_client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (new_client_socket == -1)
        {
            error("Error: Could not accept connection");
        }

        printf("Client connected. Creating a new game...\n");

        // Check if maximum number of clients reached
        if (num_clients >= MAX_CLIENTS)
        {
            struct ServerMessageHeader server_message;
            server_message.msgFlag = 2; // Server overloaded message
            server_message.wordLength = 0;
            server_message.numIncorrect = 0;
            strncpy(server_message.data, "Server overloaded. Please try again later.", BUFFER_SIZE);

            send(new_client_socket, &server_message, sizeof(server_message), 0);
            close(new_client_socket);
        }
        else
        {
            // Create a new thread for the client
            pthread_t thread;
            if (pthread_create(&thread, NULL, client_thread, &new_client_socket) != 0)
            {
                error("Error: Could not create client thread");
            }

            // Detach the thread to allow it to run independently
            pthread_detach(thread);

            // Add the new client socket to the array
            client_sockets[num_clients] = new_client_socket;
            num_clients++;
        }
    }
    close(server_socket);

    return 0;
}