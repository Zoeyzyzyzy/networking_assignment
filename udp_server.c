// Server UDP Socket in C

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int server_sock, port, num_bytes;
    struct sockaddr_in server, client;
    unsigned int client_len;
    char buffer[130];
    char bye[] = "bye";

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // create a server socket
    server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0)
    {
        printf("Error: failed to create socket.\n");
        exit(1);
    }

    // set up the server address structure
    port = atoi(argv[1]);
    if (port < 0 || port > 65535)
    {
        printf("Error: invalid port number.\n");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to the server address and port
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Error: failed to bind socket.\n");
        exit(1);
    }
    printf("Server is listening on port %d...\n", port);

    // receive data from the client
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        client_len = sizeof(client);
        if ((num_bytes = recvfrom(server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &client_len)) < 0)
        {
            printf("Error: failed to receive message.\n");
            exit(1);
        }

        // print the data received from the client
        printf("Received message from client: %s\n", buffer);

        // Check if data is all numbers
        int is_all_numbers = 1;
        for (int i = 0; i < num_bytes; i++)
        {
            if (buffer[i] < '0' || buffer[i] > '9')
            {
                is_all_numbers = 0;
                break;
            }
        }
        printf("is_all_numbers: %d\n", is_all_numbers);

        if (is_all_numbers == 0)
        {
            char response[] = "Sorry, cannot compute!";
            sendto(server_sock, response, strlen(response), 0, (struct sockaddr *)&client, sizeof(client));
            sendto(server_sock, bye, strlen(bye), 0, (struct sockaddr *)&client, sizeof(client));
        }
        else
        {
            // add all digit up
            int len = strlen(buffer);
            int sum = 0;
            for (int i = 0; i < len; i++)
            {
                int digit = (int)(buffer[i] - '0');
                sum += digit;
            }

            // send the result sum to client
            char result_sum[130];
            sprintf(result_sum, "%d", sum);
            sendto(server_sock, result_sum, strlen(result_sum), 0, (struct sockaddr *)&client, sizeof(client));
            while (sum >= 10)
            {
                int digit_sum = 0;
                while (sum)
                {
                    digit_sum += sum % 10;
                    sum /= 10;
                }
                char result_sum[130];
                sprintf(result_sum, "%d", digit_sum);
                sendto(server_sock, result_sum, strlen(result_sum), 0, (struct sockaddr *)&client, sizeof(client));
                sum = digit_sum;
            }
            sendto(server_sock, bye, strlen(bye), 0, (struct sockaddr *)&client, sizeof(client));
        }
    }
    return 0;
}