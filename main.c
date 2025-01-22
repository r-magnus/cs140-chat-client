// Chat Client for CS140
// @author Ryan Magnuson rmagnuson@westmont.edu

// IMPORTS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// #include <Winsock2.h> // arpa for Windows development
#include <arpa/inet.h>
#include <poll.h>

// CONSTANTS
#define ADDRESS "10.115.12.240" // "vmwardrobe.westmont.edu" IP Address
#define PORT 49153
#define BUFFER 1024 // standard buffer size, from what I can tell

// MAIN FUNCTION
int main(void)
{
    // Socket Connection
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(ADDRESS);

    // Poll Setup for 'netcat' experience
    struct pollfd pfds[2];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    // User Input
    char user_input[BUFFER];

    // Connection
    int connection_status = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (connection_status < 0) {
        perror("connect");
    } else {
        char server_recv[BUFFER];
        while (strcmp(user_input, "q") != 0) {
            poll(pfds, 2, -1); // Unassigned int

            if (pfds[0].revents & POLLIN) { // Listen and readout from server
                memset(server_recv,0,BUFFER);
                recv(sockfd, server_recv, BUFFER, 0);
                printf("%s", server_recv);
            }
            if (pfds[1].revents & POLLIN) { // Allow client to msg while listening
                memset(user_input,0,BUFFER);
                fgets(user_input, BUFFER, stdin); // essentially scanf

                send(sockfd, user_input, strlen(user_input), 0);
            }
        }
    }
    return 0;
}
