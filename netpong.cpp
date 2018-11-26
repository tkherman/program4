


int main(int argc, char *argv[]) {
    // Process args
    // refresh is clock rate in microseconds
    // This corresponds to the movement speed of the ball

    // Check the arguments to see if host or client and call networking code accordingly
    // if arg is host:
    //      call server setup
    // else:
    //      call client setup
    // do we need to handle if client connects before server?



    int refresh;
    if(argc >= 2) {
        char *difficulty = argv[1];
        if(strcmp(difficulty, "easy") == 0) refresh = 80000;
        else if(strcmp(difficulty, "medium") == 0) refresh = 40000;
        else if(strcmp(difficulty, "hard") == 0) refresh = 20000;
        else {
            printf("ERROR: Difficulty should be one of easy, medium, hard.\n");
            exit(1);
        }
    } else {
        printf("Usage: ./pong DIFFICULTY\n");
        exit(0);
    }

    // Set up ncurses environment
    initNcurses();

    // Set starting game state and display a countdown
    reset();
    countdown("Starting Game");

    // Listen to keyboard input in a background thread
    pthread_t pth;
    pthread_create(&pth, NULL, listenInput, NULL);

    // Main game loop executes tock() method every REFRESH microseconds
    struct timeval tv;
    while(1) {
        gettimeofday(&tv,NULL);
        unsigned long before = 1000000 * tv.tv_sec + tv.tv_usec;
        tock(); // Update game state
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
