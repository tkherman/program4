#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <fstream>
#include <unordered_map>
#include <unistd.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int socket_bind_listen(int port);
int accept_connection(int sockfd);
