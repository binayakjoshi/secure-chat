#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define XOR_KEY 0x5A

int sockfd;
char username[32];

void xor_cipher(char *data, size_t len) {
    for (size_t i = 0; i < len; i++) data[i] ^= XOR_KEY;
}

void *recv_thread(void *arg) {
    char buf[BUFFER_SIZE+1];
    while (1) {
        ssize_t n = recv(sockfd, buf, BUFFER_SIZE, 0);
        if (n <= 0) break;
        xor_cipher(buf, n);
        buf[n] = '\0';
        printf("\n%s\n", buf);
        printf("Enter message: ");
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in srv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        perror("connect"); exit(EXIT_FAILURE);
    }

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    // send join notification
    char join_msg[BUFFER_SIZE];
    int jlen = snprintf(join_msg, sizeof(join_msg), "%s has joined", username);
    xor_cipher(join_msg, jlen);
    send(sockfd, join_msg, jlen, 0);

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, NULL);

    char msg[BUFFER_SIZE];
    while (1) {
        printf("Enter message: ");
        if (!fgets(msg, sizeof(msg), stdin)) break;
        size_t mlen = strcspn(msg, "\n"); msg[mlen] = '\0';
        if (mlen == 0) continue;
        char out[BUFFER_SIZE];
        int len = snprintf(out, sizeof(out), "%s: %s", username, msg);
        xor_cipher(out, len);
        send(sockfd, out, len, 0);
    }

    // send leave notification
    char leave_msg[BUFFER_SIZE];
    int llen = snprintf(leave_msg, sizeof(leave_msg), "%s has left", username);
    xor_cipher(leave_msg, llen);
    send(sockfd, leave_msg, llen, 0);

    close(sockfd);
    return 0;
}
