#include "chatclient.h"

int socket_connect(char *host, int port) {
    int s;
    struct hostent *hp;
    struct sockaddr_in sin;
    hp = gethostbyname(host);
    if (!hp)
    {
        fprintf(stderr, "unknown host: %s\n", host);
        return -1;
    }

    // Build address data structure
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(port);

    // active open
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error\n");
        return -1;
    }

    printf("Connecting to %s\n", host);
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("connection error\n");
        close(s);
        return -1;
    }
    printf("Connection established\n");

    return s;
}
