#ifndef TIMER_H
#define TIMER_H

#include <stdlib.h>
#include <sys/time.h>

class Timer {
public:
    long calculateElapsedTime();

    void start();

    void stop();

    void reset();

    void pause();

    void unpause();
    
private:
    struct timeval timeStarted, timeFinished;
    
    long pausedSince;
    
    long totalTimePaused;
    
    long calculateTimePaused();
    
    long calculateTimeSinceStart();
};
#endif
