// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// #define PORT 8080
int main(int argc, char const* argv[])
{
    char *endptr;
    long port_number;
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };

    // accept exactly one argument which is port
    if (argv[1] == NULL) {
        perror("No Port Number");
        exit(EXIT_FAILURE);
    }

    // Convert the argument to a long integer
    port_number = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
      perror("Provided Port Number is not an integer\n");
      exit(EXIT_FAILURE);
    }
    printf("hellow world lol!");
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    // address.sin_port = htons(PORT);
    address.sin_port = htons(port_number);
  
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if ((new_socket
             = accept(server_fd, (struct sockaddr*)&address,
                      (socklen_t*)&addrlen))
            < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // receive string from client store in buffer
        valread = read(new_socket, buffer, 1024);

        // // add all digit up
        int len = strlen(buffer);
        int sum = 0;
        char* fail_compute = "Sorry, cannot compute!";
        for (int i = 0; i < len; i++) {
            int digit = (int) (buffer[i] - '0');
            if (digit > 10 || digit < 0) {
                send(new_socket, fail_compute , strlen(fail_compute), 0); 
                close(new_socket);
            }
            sum += digit;
        }
        // send the result sum to client
        char result_sum[1024];
        memset(result_sum, '\0', sizeof(result_sum));
        sprintf(result_sum, "%d", sum);
        result_sum[strlen(result_sum)] = '^';
        send(new_socket, result_sum, strlen(result_sum), 0);
        while (sum >= 10) {
            int digit_sum = 0;
            while (sum) {
                digit_sum += sum % 10;
                sum /= 10;
            }
            char new_sum[1024];
            memset(new_sum, '\0', sizeof(new_sum));
            sprintf(new_sum, "%d", digit_sum);
            new_sum[strlen(new_sum)] = '^';
            send(new_socket, new_sum, strlen(new_sum), 0);
            printf("new_sum%s", new_sum);
            sum = digit_sum;
        }
        memset(result_sum, '\0', sizeof(result_sum));
        memset(buffer, '\0', sizeof(buffer));
        close(new_socket);
    }

    // closing the listening socket
    // shutdown(server_fd, SHUT_RDWR);
    return 0;
}
