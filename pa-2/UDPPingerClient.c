#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <float.h>

#define MSG_SIZE 5
#define PING "PING"
#define PONG "PONG"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    // get host and port
    char *hostname = argv[1];
    int port = atoi(argv[2]);

    // create socket
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0)
    {
        printf("Error: failed to create socket.\n");
        exit(1);
    }

    // Set the timeout value for the socket
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt failed");
        exit(1);
    }

    // Hold the server's address
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(hostname);
    servaddr.sin_port = htons(port);

    int recieved_n = 0;
    double rtt_min = DBL_MAX;
    double rtt_sum = 0;
    double rtt_max = 0;

    // Send 10 ping requests to the server, separated by approximately one second

    for (size_t i = 1; i <= 10; i++)
    {
        char msg[MSG_SIZE] = "1234";
        struct timeval send_time;
        gettimeofday(&send_time, NULL);

        if (sendto(client_sock, msg, strlen(msg), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("Error: failed to send message");
            exit(1);
        }

        char buffer[MSG_SIZE];
        ssize_t n = recvfrom(client_sock, buffer, MSG_SIZE, 0, NULL, NULL);
        struct timeval receive_time;
        gettimeofday(&receive_time, NULL);

        if (n < 0)
        {
            printf("Request timeout for seq#=%ld \n", i);
        }
        else
        {
            buffer[n] = '\0';
            double rtt_ms = (receive_time.tv_sec - send_time.tv_sec) * 1000.0;
            rtt_ms += (receive_time.tv_usec - send_time.tv_usec) / 1000.0;
            rtt_max = fmax(rtt_max, rtt_ms);
            rtt_min = fmin(rtt_min, rtt_ms);
            rtt_sum += rtt_ms;
            recieved_n++;
            printf("PING received from %s: seq#=%ld time=%.3f ms \n", hostname, i, rtt_ms);
        }

        sleep(1);
    }
    close(client_sock);
    double timeout_p = ((10 - recieved_n) / 10.0) * 100;
    double rtt_avg = rtt_sum / recieved_n;
    printf("--- %s ping statistics --- \n", hostname);
    printf("%d packets transmitted, %d received, %.1f%% packet loss rtt min / avg / max = %.3f %.3f %.3f ms\n", 10, recieved_n, timeout_p, rtt_min, rtt_avg, rtt_max);
}
