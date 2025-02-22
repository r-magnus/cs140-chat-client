// CS140 Chat Client
// Ryan Magnuson rmagnuson@westmont.edu

// Library Setup
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
// #include <WinSock2.h> // For Windows dev
#include <poll.h>
#include <ncurses.h>
#include <signal.h>

//  Var Setup
#define ADDRESS "10.115.12.240"
#define PORT 49153
#define BUFFER 1024

int sockfd;
volatile sig_atomic_t running = 1;
WINDOW *msg_win, *input_win;

// HELPER METHODS //
void handle_signal(int sig) {
    // "Did we quit?"
    (void)sig;
    running = 0;
}

void cleanup() {
    // Close the socket if need be
    if (sockfd > 0) close(sockfd);
    endwin();
}

void init_ncurses() {
    // Initializes everything we need for ncurses... :)
    initscr();
    cbreak();
    noecho();
    raw();
    refresh();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    msg_win = newwin(rows - 2, cols, 0, 0);
    input_win = newwin(1, cols, rows - 1, 0);
    scrollok(msg_win, TRUE);
    nodelay(input_win, TRUE);
    keypad(input_win, TRUE);
}

int create_socket() {
    // Forms socket connection
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        endwin();
        exit(1);
    }
    return sock;
}

struct sockaddr_in get_server_address() {
    // Builds sockaddr_in struct
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = inet_addr(ADDRESS),
        .sin_zero = {0}
    };
    return server;
}

void connect_to_server() {
    // Uses previously defined sockaddr_in to connect to server
    struct sockaddr_in server = get_server_address();
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        endwin();
        exit(1);
    }
    wrefresh(msg_win);
}

// MAIN //
int main(void) {
    // Call all of our handy-dandy helpers.
    signal(SIGINT, handle_signal);
    atexit(cleanup);

    init_ncurses();
    wrefresh(msg_win);

    sockfd = create_socket();
    connect_to_server();

    // Setup poll
    struct pollfd pfds[1] = {{ .fd = sockfd, .events = POLLIN }};
    char user_input[BUFFER] = {0};
    int input_len = 0;
    char server_recv[BUFFER];

    while (running) {
        if (poll(pfds, 1, 100) > 0 && (pfds[0].revents & POLLIN)) {
            memset(server_recv, 0, BUFFER);
            int bytes_received = recv(sockfd, server_recv, BUFFER, 0);
            if (bytes_received > 0) {
                wprintw(msg_win, "%s", server_recv);
                wrefresh(msg_win);
            } else {
                wrefresh(msg_win);
                break;
            }
        }

        int ch = wgetch(input_win);
        if (ch == 3) {  // Ctrl+C
            running = 0;
            break;
        } else if (ch != ERR) {
            if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
                if (input_len > 0) {
                    user_input[input_len] = '\n';
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

    return 0;
}
