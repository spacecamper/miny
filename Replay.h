#ifndef REPLAY_H
#define REPLAY_H

#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>

#include "Action.h"

using namespace std;

class Replay {
public:

    std::list<Action> data;

    bool recording;


    Replay();

    bool isRecording(); //recorder
    void startRecording(); //recorder
    void pauseRecording(); //recorder
    void resumeRecording(); //recorder
    void stopRecording(); //recorder
    void deleteData(); //recorder
    void recordEvent(int, int, int); //recorder
    void writeToFile(ofstream*, void*); //recorder 
    void dump();
};

#endif
