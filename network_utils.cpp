// network_utils.cpp

#include "network_utils.h"
using namespace std;
// send struct data to other user
int send_struct(int sockfd, GameState gs) {
    string message;

    message = string(gs.ballX) + "," + string(gs.ballY) + "," + string(gs.dx) + "," + string(gs.dy) + "," + string(gs.padLY) + "," + string(gs.padRY) + "," + string(gs.scoreL) \
    + "," + string(gs.scoreR);

    if (send_string(sockfd, message) < 0) {
        fprintf(stderr, "Failed to send struct");
        return 0;
    }

}

int recv_struct(int sockfd, GameState &gs) {
    string msg;
    vector<string> info_list;

    if (recv_string(sockfd, msg) < 0) {
        fprintf(stderr, "Error receiving struct from peer");
        return 0;
    }

    stringstream ss(msg);
    string state;

    while(getline(ss, state, ',')) {
        info_list.push_back(state);
    }

    pthread_mutex_lock(&mutex);
    update_gamestate(gs, info_list);
    pthread_mutex_unlock(&mutex);

}

void update_gamestate(GameState &gs, vector<string> list) {
    try {
        gs.ballX = stoi(list[0]);
        gs.ballY = stoi(list[1]);
        gs.dx = stoi(list[2]);
        gs.dy = stoi(list[3]);
        gs.padLY = stoi(list[4]);
        gs.padRY = stoi(list[5]);
        gs.scoreL = stoi(list[6]);
        gs.scoreR = stoi(list[7]);
    }  catch  (const exception &e) {
        cout << "A standard exception was caught, with message " << e.what() << "\n";
    }
}

int send_string(int sockfd, std::string msg) {
    int ret;
    size_t len;

    // send length of msg
    len = msg.length();
    if ((ret = send(sockfd, &len, sizeof(size_t), 0)) < 0) {
        perror("ERROR sending string length");
        return ret;
    }

    // send msg
    if ((ret = send(sockfd, msg.c_str(), len, 0)) < 0) {
        perror ("ERROR sending string message");
        return ret;
    }

    return ret;
}

int recv_string(int sockfd, std::string &msg) {
    int ret;
    size_t len;
    size_t received = 0;
    char buffer[BUFSIZ];

    // recv length of msg
    memset(buffer, 0, BUFSIZ);
    if ((ret = recv(sockfd, buffer, sizeof(size_t), 0)) < 0) {
        perror("ERROR recving string length");
        return ret;
    }
    memcpy(&len, buffer, sizeof(size_t));

    while (received < len) {
        memset(buffer, 0, BUFSIZ);
        if ((ret = recv(sockfd, buffer, len, 0)) < 0) {
            perror("ERROR receiving string");
            return ret;
        }
        received += ret;
        msg.append(std::string(buffer));
    }

    return ret;
}
