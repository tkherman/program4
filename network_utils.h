// network_utils.h

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
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

#define NULL_INT 1000000

using namespace std;

// Keep track of global game state
struct GameState {
    int ballX, ballY;
    int dx, dy;
    int padLY, padRY;
    int scoreL, scoreR;
};

int send_struct(int sockfd, GameState gs);
int recv_struct(int sockfd, GameState &gs);
void update_gamestate(GameState &gs, vector<string> list);
int send_string(int sockfd, std::string msg);
int recv_string(int sockfd, std::string &msg);
