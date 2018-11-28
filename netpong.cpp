#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/time.h>
using namespace std;

#include "client.h"
#include "server.h"
#include "network_utils.h"

#define WIDTH 43
#define HEIGHT 21
#define PADLX 1
#define PADRX WIDTH - 2
#define NULL_INT 1000000

// Global variables recording the state of the game
// Position of ball
int ballX, ballY;
// Movement of ball
int dx, dy;
// Position of paddles
int padLY, padRY;
// Player scores
int scoreL, scoreR;

int new_game = 0;

bool host;

// ncurses window
WINDOW *win;

// global lock for updating and sending game state
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Draw the current game state to the screen
 * ballX: X position of the ball
 * ballY: Y position of the ball
 * padLY: Y position of the left paddle
 * padRY: Y position of the right paddle
 * scoreL: Score of the left player
 * scoreR: Score of the right player
 */
void draw(int ballX, int ballY, int padLY, int padRY, int scoreL, int scoreR) {
    // Center line
    int y;
    for(y = 1; y < HEIGHT-1; y++) {
        mvwaddch(win, y, WIDTH / 2, ACS_VLINE);
    }
    // Score
    mvwprintw(win, 1, WIDTH / 2 - 3, "%2d", scoreL);
    mvwprintw(win, 1, WIDTH / 2 + 2, "%d", scoreR);
    // Ball
    mvwaddch(win, ballY, ballX, ACS_BLOCK);
    // Left paddle
    for(y = 1; y < HEIGHT - 1; y++) {
	int ch = (y >= padLY - 2 && y <= padLY + 2)? ACS_BLOCK : ' ';
        mvwaddch(win, y, PADLX, ch);
    }
    // Right paddle
    for(y = 1; y < HEIGHT - 1; y++) {
	int ch = (y >= padRY - 2 && y <= padRY + 2)? ACS_BLOCK : ' ';
        mvwaddch(win, y, PADRX, ch);
    }
    // Print the virtual window (win) to the screen
    wrefresh(win);
    // Finally erase ball for next time (allows ball to move before next refresh)
    mvwaddch(win, ballY, ballX, ' ');
}

/* Return ball and paddles to starting positions
 * Horizontal direction of the ball is randomized
 */
void reset() {
    ballX = WIDTH / 2;
    padLY = padRY = ballY = HEIGHT / 2;
    // dx is randomly either -1 or 1
    dx = (rand() % 2) * 2 - 1;
    dy = 0;
    // Draw to reset everything visually
    draw(ballX, ballY, padLY, padRY, scoreL, scoreR);
}

/* Display a message with a 3 second countdown
 * This method blocks for the duration of the countdown
 * message: The text to display during the countdown
 */
void countdown(const char *message) {
    int h = 4;
    int w = strlen(message) + 4;
    WINDOW *popup = newwin(h, w, (LINES - h) / 2, (COLS - w) / 2);
    box(popup, 0, 0);
    mvwprintw(popup, 1, 2, message);
    int countdown;
    for(countdown = 3; countdown > 0; countdown--) {
        mvwprintw(popup, 2, w / 2, "%d", countdown);
        wrefresh(popup);
        sleep(1);
    }
    wclear(popup);
    wrefresh(popup);
    delwin(popup);
    padLY = padRY = HEIGHT / 2; // Wipe out any input that accumulated during the delay
}

/* Perform periodic game functions:
 * 1. Move the ball
 * 2. Detect collisions
 * 3. Detect scored points and react accordingly
 * 4. Draw updated game state to the screen
 */
void tock(int sockfd) {
    // Move the ball
    ballX += dx;
    ballY += dy;

    // Check for paddle collisions
    // padY is y value of closest paddle to ball
    int padY = (ballX < WIDTH / 2) ? padLY : padRY;
    // colX is x value of ball for a paddle collision
    int colX = (ballX < WIDTH / 2) ? PADLX + 1 : PADRX - 1;
    if(ballX == colX && abs(ballY - padY) <= 2) {
        if (host && ballX < WIDTH / 2) {
            // Collision detected!
            dx *= -1;
            // Determine bounce angle
            if(ballY < padY) dy = -1;
            else if(ballY > padY) dy = 1;
            else dy = 0;

            // Send game state update to client
            GameState gs;
            gs.ballX = NULL_INT;
            gs.ballY = NULL_INT;
            gs.dx = dx;
            gs.dy = dy;
            gs.padLY = NULL_INT;
            gs.padRY = NULL_INT;
            gs.scoreL = NULL_INT;
            gs.scoreR = NULL_INT;
            send_struct(sockfd, gs);
        } else if (!host && ballX >= WIDTH/2) {
            // Collision detected!
            dx *= -1;
            // Determine bounce angle
            if(ballY < padY) dy = -1;
            else if(ballY > padY) dy = 1;
            else dy = 0;

            // Send game state update to host
            GameState gs;
            gs.ballX = NULL_INT;
            gs.ballY = NULL_INT;
            gs.dx = dx;
            gs.dy = dy;
            gs.padLY = NULL_INT;
            gs.padRY = NULL_INT;
            gs.scoreL = NULL_INT;
            gs.scoreR = NULL_INT;
            send_struct(sockfd, gs);
        }
    }

    // Check for top/bottom boundary collisions
    if(ballY == 1) dy = 1;
    else if(ballY == HEIGHT - 2) dy = -1;

    // Score points
    if(ballX == 0 && host) {
        scoreR = (scoreR + 1) % 100;

        GameState gs;
        gs.ballX = NULL_INT;
        gs.ballY = NULL_INT;
        gs.dx = NULL_INT;
        gs.dy = NULL_INT;
        gs.padLY = NULL_INT;
        gs.padRY = NULL_INT;
        gs.scoreL = NULL_INT;
        gs.scoreR = scoreR;
        send_struct(sockfd, gs);

        new_game = 1;
    } else if(ballX == WIDTH - 1 && !host) {
        scoreL = (scoreL + 1) % 100;

        GameState gs;
        gs.ballX = NULL_INT;
        gs.ballY = NULL_INT;
        gs.dx = NULL_INT;
        gs.dy = NULL_INT;
        gs.padLY = NULL_INT;
        gs.padRY = NULL_INT;
        gs.scoreL = scoreL;
        gs.scoreR = NULL_INT;
        send_struct(sockfd, gs);

        new_game = 2;
    }
    
    if (new_game == 1) {
        reset();
        countdown("SCORE -->");
        new_game = 0;
    } else if (new_game == 2) {
        reset();
        countdown("<-- SCORE");
        new_game = 0;
    }
    // Finally, redraw the current state
    draw(ballX, ballY, padLY, padRY, scoreL, scoreR);
}

/* Listen to keyboard input
 * Updates global pad positions
 */
void *listenInput(void *args) {
    int sockfd = *(int*)args;
    bool update = false;
    while(1) {
        switch(getch()) {
            case KEY_UP:
                if (host)
                    padLY--;
                else
                    padRY--;
                update = true;
			    break;
            case KEY_DOWN:
                if (host)
                    padLY++;
                else
                    padRY++;
                update = true;
			    break;
            default: break;
	    }
        
        if (update) {
            GameState gs;
            if (host) {
                gs.ballX = NULL_INT;
                gs.ballY = NULL_INT;
                gs.dx = NULL_INT;
                gs.dy = NULL_INT;
                gs.padLY = padLY;
                gs.padRY = NULL_INT;
                gs.scoreL = NULL_INT;
                gs.scoreR = NULL_INT;
            } else {
                gs.ballX = NULL_INT;
                gs.ballY = NULL_INT;
                gs.dx = NULL_INT;
                gs.dy = NULL_INT;
                gs.padLY = NULL_INT;
                gs.padRY = padRY;
                gs.scoreL = NULL_INT;
                gs.scoreR = NULL_INT;
            }

            send_struct(sockfd, gs);
        }

        update = false;
    }
    return NULL;
}

/* Listen for game state updates from other user
 * Update global positions accordingly
*/
void *recvUpdates(void *args) {
    int sockfd = *(int*)args;
    while (1) {
        GameState gs;
        recv_struct(sockfd, gs);
        if (host && gs.padRY != NULL_INT) {
            padRY = gs.padRY;
        } else if (!host && gs.padLY != NULL_INT) {
            padLY = gs.padLY;
        }

        if (gs.dx != NULL_INT)
            dx = gs.dx;

        if (gs.dy != NULL_INT)
            dy = gs.dy;

        if (gs.scoreL != NULL_INT) {
            scoreL = gs.scoreL;
            new_game = 2;
        }

        if (gs.scoreR != NULL_INT) {
            scoreR = gs.scoreR;
            new_game = 1;
        }
        
    }
}

void initNcurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    refresh();
    win = newwin(HEIGHT, WIDTH, (LINES - HEIGHT) / 2, (COLS - WIDTH) / 2);
    box(win, 0, 0);
    mvwaddch(win, 0, WIDTH / 2, ACS_TTEE);
    mvwaddch(win, HEIGHT-1, WIDTH / 2, ACS_BTEE);
}

int main(int argc, char *argv[]) {
    // Process args
    // refresh is clock rate in microseconds
    // This corresponds to the movement speed of the ball
    int refresh;
    int port;
    string machine;
    string difficulty;
    if (argc < 3) {
        printf("Usage: ./netpong --host port difficulty\n");
        printf("or\n");
        printf("Usage: ./netpong machine port\n");
        exit(0);
    }

    if (strcmp(argv[1], "--host") == 0) {
        host = true;
        port = stoi(string(argv[2]));
        difficulty = string(argv[3]);
        if(strcmp(difficulty.c_str(), "easy") == 0) refresh = 80000;
        else if(strcmp(difficulty.c_str(), "medium") == 0) refresh = 40000;
        else if(strcmp(difficulty.c_str(), "hard") == 0) refresh = 20000;
        else {
            printf("ERROR: Difficulty should be one of easy, medium, hard.\n");
            exit(1);
        }
    } else {
        machine = string(argv[1]);
        port = stoi(string(argv[2]));
        host = false;
    }

    int serv_sockfd;
    int sockfd;
    if (host) {
        if ((serv_sockfd = socket_bind_listen(port)) < 0) {
            printf("ERROR: socket bind listen failed\n");
            exit(1);
        }
        if ((sockfd = accept_connection(serv_sockfd)) < 0) {
            printf("ERROR: failed to accept connection\n");
            exit(1);
        }
    } else {
        if ((sockfd = socket_connect((char*)machine.c_str(), port)) < 0) {
            printf("ERROR: failed to connect to host\n");
            exit(1);
        }
    }
    
    // If host, send refresh rate to client
    if (host) {
        if (send_string(sockfd, to_string(refresh)) < 0) {
            printf("ERROR: can't send refresh rate to client\n");
            exit(1);
        }
    // If client, then recv refresh rate from host
    } else {
        string r;
        if (recv_string(sockfd, r) < 0) {
            printf("ERROR: can't get refresh rate from host\n");
            exit(1);
        }
        refresh = stoi(r);
    }

    // Set up ncurses environment
    initNcurses();

    // Set starting game state and display a countdown
    reset();
    countdown("Starting Game");

    // Listen to keyboard input in a background thread
    pthread_t pth;
    pthread_create(&pth, NULL, listenInput, (void*)&sockfd);

    // Listen for game state updates on a background thread
    pthread_t thread;
    pthread_create(&thread, NULL, recvUpdates, (void*)&sockfd);

    // Main game loop executes tock() method every REFRESH microseconds
    struct timeval tv;
    while(1) {
        gettimeofday(&tv,NULL);
        unsigned long before = 1000000 * tv.tv_sec + tv.tv_usec;
        tock(sockfd); // Update game state
        gettimeofday(&tv,NULL);
        unsigned long after = 1000000 * tv.tv_sec + tv.tv_usec;
        unsigned long toSleep = refresh - (after - before);
        // toSleep can sometimes be > refresh, e.g. countdown() is called during tock()
        // In that case it's MUCH bigger because of overflow!
        if(toSleep > refresh) toSleep = refresh;
        usleep(toSleep); // Sleep exactly as much as is necessary
    }

    // Clean up
    pthread_join(pth, NULL);
    endwin();
    return 0;
}
