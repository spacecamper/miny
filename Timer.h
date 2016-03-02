
#include <stdlib.h>
#include <sys/time.h>

class Timer {
public:

    struct timeval timeStarted, timeFinished;
    long totalTimePaused;
    long pausedSince;


    long calculateTimeSinceStart();


    long calculateTimePaused();

    long calculateElapsedTime();

    void start();

    void stop();

    void reset();

    void pause();

    void unpause();

};
