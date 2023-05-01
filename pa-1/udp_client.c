// Client UDP Socket in C 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int client_sock, port;
    struct sockaddr_in serv_addr;

    if (argc != 3)
    {
         printf("Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    // create a socket
    client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0)
    {
        printf("Error: failed to create socket.\n");
        exit(1);
    }

    // set up the server address structure
    port = atoi(argv[2]);
    if (port < 0 || port > 65535)
    {
        printf("Error: invalid port number.\n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        printf("Error: invalid address.\n");
        exit(1);
    }

    char buffer[130];
    bzero(buffer,130);
    printf("Enter string: ");
    scanf("%s", buffer);
    
    // send data to the server
    if (sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error: failed to send message.\n");
        exit(1);
    }

    // receive data from the server
    bzero(buffer, 130);
    // while (1) {
    //     memset(buffer, 0, sizeof(buffer));
    //     int response = recvfrom(client_sock, buffer, sizeof(buffer), 0, NULL, NULL);
    //     if (response < 0) {
    //         printf("Error: failed to receive message.\n");
    //         break;
    //     } 
    //     printf("From Server: %s\n", buffer);
    //     if(response == 1) {
    //         close(client_sock);
    //     }
    //      printf("Length of the string: %d\n", response);
    // }
    memset(buffer, 0, sizeof(buffer));
    int response = recvfrom(client_sock, buffer, sizeof(buffer), 0, NULL, NULL);
    while (response > 0){
        printf("From server: %s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
        response = recvfrom(client_sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if(buffer[0] == 'b' && buffer[1] == 'y' && buffer[2] == 'e') {
            close(client_sock);
            break;
        }
    }
    
    return 0;
}