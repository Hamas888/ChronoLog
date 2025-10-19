#include <iostream>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define CHRONOLOG_PRO_FEATURES 1

#include "../include/ChronoLog.h"

using namespace std;

// Function to check for keyboard input without blocking
bool keyboardHit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
}

int main() {
    ChronoLogger logger("myLogger", CHRONOLOG_LEVEL_PRO_FEATURES);

    logger.debug("Desktop (linux/windows/macos) Logging Example Starting...");
    logger.info("Logger initialized successfully at DEBUG level");

    while(true) {
        logger.debug("This is a debug message.");
        logger.info("This is an info message.");
        logger.warn("This is a warning message.");
        logger.error("This is an error message.");
        logger.fatal("This is a fatal message.");

        for(int i = 0; i <= 100; i += 10) {
            logger.progress(i, 100, "Processing");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));            // Simulate some work
        }   

        if(keyboardHit()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));               // Wait 5 seconds before repeating
    }
    return 0;
}