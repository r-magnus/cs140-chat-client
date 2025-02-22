// Chat Client with ncurses UI (Multiplexing Fixed)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ncurses.h>
#include <signal.h>

#define ADDRESS "10.115.12.240"
#define PORT 49153
#define BUFFER 1024

int sockfd;
volatile sig_atomic_t running = 1;

// Handle Ctrl+C (SIGINT)
void handle_signal(int sig) {
    (void)sig;
    running = 0;
    close(sockfd);
    endwin();
    exit(0);
}

int main(void) {
    // Register SIGINT handler
    signal(SIGINT, handle_signal);

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    raw();
    refresh();

    // Get terminal dimensions
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Create message and input windows
    WINDOW *msg_win = newwin(rows - 2, cols, 0, 0);
    WINDOW *input_win = newwin(1, cols, rows - 1, 0);
    scrollok(msg_win, TRUE);

    wprintw(msg_win, "Connecting to server...\n");
    wrefresh(msg_win);

    nodelay(input_win, TRUE);  // Make input non-blocking
    keypad(input_win, TRUE);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        endwin();
        return 1;
    }

    // Define server address
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(ADDRESS);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        endwin();
        close(sockfd);
        return 1;
    }

    wprintw(msg_win, "Connected!\n");
    wrefresh(msg_win);

    // Setup polling for server socket
    struct pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;  // Watch for server messages

    char user_input[BUFFER] = {0};
    int input_len = 0;
    char server_recv[BUFFER];

    while (running) {
        // Poll for server messages (timeout 100ms)
        int poll_count = poll(pfds, 1, 100);

        if (poll_count > 0 && (pfds[0].revents & POLLIN)) {
            memset(server_recv, 0, BUFFER);
            int bytes_received = recv(sockfd, server_recv, BUFFER, 0);

            if (bytes_received > 0) {
                wprintw(msg_win, "%s", server_recv);
                wrefresh(msg_win);
            } else {
                wprintw(msg_win, "Server disconnected.\n");
                wrefresh(msg_win);
                break;
            }
        }

        // Handle user input
        int ch = wgetch(input_win);
        if (ch != ERR) {
            if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                if (input_len > 0) {
                    user_input[input_len] = '\n';
                    user_input[input_len + 1] = '\0';
                    send(sockfd, user_input, strlen(user_input), 0);

                    memset(user_input, 0, BUFFER);
                    input_len = 0;
                    werase(input_win);
                    wrefresh(input_win);
                }
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                if (input_len > 0) {
                    input_len--;
                    user_input[input_len] = '\0';
                    werase(input_win);
                    mvwprintw(input_win, 0, 0, "%s", user_input);
                    wrefresh(input_win);
                }
            } else if (input_len < BUFFER - 2 && ch > 0 && ch < 256) {
                user_input[input_len++] = ch;
                waddch(input_win, ch);
                wrefresh(input_win);
            }
        }
    }

    // Clean up
    endwin();
    close(sockfd);
    return 0;
}
