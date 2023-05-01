// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
// #define PORT 8080
  
int main(int argc, char const* argv[])
{
    char *endptr;
    int status, client_fd;
    long port_number;
    struct sockaddr_in serv_addr;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  

    // accept exactly one argument which is port
    if (argv[1] == NULL) {
        perror("No Port Number");
        return 0;
    }

    // Convert the argument to a long integer
    port_number = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
      perror("Provided Port Number is not an integer\n");
      return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
  
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    // "127.0.0.1"
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char name[1024];
    // accept input as a string of digit
    printf("Enter string: ");
    scanf("%s", name);
    // printf("input string is: %s", name);


    // send to the server
    send(client_fd, name, strlen(name), 0);
   

    while (1) {
        char buffer[1024] = { 0 };
        if(read(client_fd, buffer, 1024) < 0){
            printf("Error while receiving server's msg\n");
            return -1;
        }
        // if (atoi(buffer) < 10) {
        //     buffer[strlen(buffer) - 1] = '\0';
        // }
        char * token = strtok(buffer, "^");
        // loop through the string to extract all other tokens
        while( token != NULL ) {
            printf( "From server: %s\n", token ); //printing each token
            token = strtok(NULL, "^");
        }

        if (atoi(buffer) < 10) {break;}
    }

    close(client_fd);

    // closing the connected socket
    return 0;
}
