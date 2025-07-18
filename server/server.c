#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define XOR_KEY 0x5A
#define LOG_FILE "chat_messages.txt"

typedef struct {
    int fd;
    char username[32];
    int active;
} client_t;

client_t clients[MAX_CLIENTS];
int listen_fd;

// count the total clients connected
int count_active_clients() {
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            count++;
        }
    }
    return count;
}

// add message with time manche haru ko
void log_message(const char *username, const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; 
    
    fprintf(log_file, "[%s] %s: %s\n", time_str, username, message);
    fclose(log_file);
}

void xor_cipher(char *data, size_t len) {
    for (size_t i = 0; i < len; i++) data[i] ^= XOR_KEY;
}

void broadcast(const char *msg, size_t len, int exclude_fd) {
    char buffer[BUFFER_SIZE];
    memcpy(buffer, msg, len);
    xor_cipher(buffer, len);
    
    //sender bahek client count garcha
    int active_recipients = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].fd != exclude_fd) {
            active_recipients++;
        }
    }
    
    // Only broadcast if there are other clients to receive the message
    if (active_recipients > 0) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active && clients[i].fd != exclude_fd) {
                send(clients[i].fd, buffer, len, 0);
            }
        }
    }
}

int add_client(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].fd = fd;
            clients[i].username[0] = '\0';
            clients[i].active = 1;
            return i;
        }
    }
    return -1;
}

void remove_client(int idx) {
    close(clients[idx].fd);
    clients[idx].active = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    if (listen(listen_fd, 10) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", port);

    struct pollfd pfds[MAX_CLIENTS + 1];
    pfds[0].fd = listen_fd;
    pfds[0].events = POLLIN;
    for (int i = 1; i <= MAX_CLIENTS; i++) pfds[i].fd = -1;
    memset(clients, 0, sizeof(clients));

    while (1) {
        poll(pfds, MAX_CLIENTS + 1, -1);
        // Accept new client
        if (pfds[0].revents & POLLIN) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd < 0) continue;
            int idx = add_client(client_fd);
            if (idx < 0) {
                close(client_fd);
                continue;
            }
            pfds[idx+1].fd = client_fd;
            pfds[idx+1].events = POLLIN;
            printf("Client connected: fd=%d (idx=%d)\n", client_fd, idx);
            printf("Total clients connected: %d\n", count_active_clients());
        }
        // Handle messages
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            int fd = pfds[i].fd;
            if (fd < 0 || !(pfds[i].revents & POLLIN)) continue;
            char buf[BUFFER_SIZE];
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            int cid = i-1;
            if (n <= 0) {
                // client leaving
                if (clients[cid].active && clients[cid].username[0]) {
                    // Only broadcast leave message if there are other clients
                    int other_clients = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].active && clients[j].fd != fd) {
                            other_clients++;
                        }
                    }
                    
                    if (other_clients > 0) {
                        char leave_msg[BUFFER_SIZE];
                        int len = snprintf(leave_msg, sizeof(leave_msg), "%s has left", clients[cid].username);
                        broadcast(leave_msg, len, fd);
                    }
                    printf("Client disconnected: %s (idx=%d)\n", clients[cid].username, cid);
                }
                remove_client(cid);
                pfds[i].fd = -1;
                printf("Total clients connected: %d\n", count_active_clients());
            } else {
                xor_cipher(buf, n);
                if (clients[cid].username[0] == '\0') {
                    // first message is join announcement
                    buf[n] = '\0';
                    char *p = strstr(buf, " has joined");
                    if (p) *p = '\0';
                    strncpy(clients[cid].username, buf, sizeof(clients[cid].username)-1);
                    
                    // Only broadcast join message if there are other clients
                    int other_clients = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].active && clients[j].fd != fd) {
                            other_clients++;
                        }
                    }
                    
                    if (other_clients > 0) {
                        char join_msg[BUFFER_SIZE];
                        int len = snprintf(join_msg, sizeof(join_msg), "%s has joined", clients[cid].username);
                        broadcast(join_msg, len, fd);
                    }
                    printf("Client joined: %s (idx=%d)\n", clients[cid].username, cid);
                    printf("Total clients connected: %d\n", count_active_clients());
                } else {
                    // Check if there are other clients to broadcast to
                    int other_clients = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].active && clients[j].fd != fd) {
                            other_clients++;
                        }
                    }
                    
                    if (other_clients > 0) {
                        broadcast(buf, n, fd);
                        //log extract garne
                        buf[n] = '\0';
                        char *colon = strchr(buf, ':');
                        if (colon) {
                            *colon = '\0';
                            char *username = buf;
                            char *message = colon + 2; // Skip ": "
                            log_message(username, message);
                        }
                    } else {
                        // Send a notification back to the client that they're alone
                        char alone_msg[] = "You are the only one online. Your message was not delivered.";
                        char response[BUFFER_SIZE];
                        memcpy(response, alone_msg, strlen(alone_msg));
                        xor_cipher(response, strlen(alone_msg));
                        send(fd, response, strlen(alone_msg), 0);
                        
                        // Still log the message even if not delivered
                        buf[n] = '\0';
                        char *colon = strchr(buf, ':');
                        if (colon) {
                            *colon = '\0';
                            char *username = buf;
                            char *message = colon + 2; // Skip ": "
                            log_message(username, message);
                        }
                    }
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}
