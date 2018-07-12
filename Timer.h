#ifndef TIMER_H
#define TIMER_H

#include <stdlib.h>
#include <sys/time.h>

class Timer {
public:

    struct timeval timeStarted, timeFinished;
    long totalTimePaused;
    long pausedSince;


    long calculateTimeSinceStart();

    long calculateElapsedTime();

    void start();

    void stop();

    void reset();

    void pause();

    void unpause();
    
private:
    long calculateTimePaused();
};
#endif
