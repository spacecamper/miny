

#include "Timer.h"
#include "common.h"


extern int gameState;
extern bool gamePaused;



long Timer::calculateTimeSinceStart() {
    
    if (gameState==GAME_INITIALIZED)
        return 0;
    else {

        long seconds, useconds;    

        struct timeval now;

        gettimeofday(&now, NULL);

        seconds  = now.tv_sec  - timeStarted.tv_sec;
        useconds = now.tv_usec - timeStarted.tv_usec;

        return ((seconds) * 1000 + useconds/1000.0) + 0.5;
    }

}


long Timer::calculateTimePaused() {

    if (gameState==GAME_INITIALIZED) {
        return 0;
    }
    else if (gameState==GAME_PLAYING) {
        if (gamePaused) {
            return totalTimePaused+calculateTimeSinceStart()-pausedSince;
        }
        else {
            return totalTimePaused;
        }
    }
    else  { //if (gameState==GAME_LOST or gameState==GAME_WON) 
        return totalTimePaused;
    }


}

long Timer::calculateElapsedTime() {

    // calculates time from first click till now (when playing) or till game has ended,
    //  minus the time when game was paused

    long seconds, useconds;    
    long elapsedTime;

    

    if (gameState==GAME_PLAYING) {


        elapsedTime=calculateTimeSinceStart()-calculateTimePaused();

    }
    else if (gameState==GAME_LOST or gameState==GAME_WON) {

        seconds  = timeFinished.tv_sec  - timeStarted.tv_sec;
        useconds = timeFinished.tv_usec - timeStarted.tv_usec;

        elapsedTime=((seconds) * 1000 + useconds/1000.0) + 0.5-totalTimePaused;

    }
    else
        elapsedTime=0;

    return elapsedTime;

        
}

void Timer::start() {

    gettimeofday(&timeStarted, NULL);
}

void Timer::stop() {
    gettimeofday(&timeFinished, NULL);
}

void Timer::reset() {
    totalTimePaused=0;
}

void Timer::pause() {
    pausedSince=calculateTimeSinceStart();
}

void Timer::unpause() {
    totalTimePaused+=calculateTimeSinceStart()-pausedSince;
}


