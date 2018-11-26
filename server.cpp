

#include "server.h"

int socket_bind_listen(int port) {
    int sockfd;
    int opt;
    struct sockaddr_in sin;

    /* Create socket */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    /* Call setsockopt for port reuse */
    opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(int));

    /* Set server addr struct */
    memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons((unsigned short)port);

    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("ERROR binding socket");
        return -1;
    }

    /* Listen on socket */
    if ((listen(sockfd, BACKLOG)) < 0) {
        perror("ERROR listening");
        return -1;
    }

    return sockfd;

}

int accept_connection(int sockfd) {
    int newfd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;

    sin_size = sizeof(their_addr);
    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (newfd == -1) {
        perror("ERROR in accepting");
        return -1;
    }

    return newfd;
}
