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

    bool recording;
    
    std::list<Action> data;

    Replay();

    //bool isRecording(); //recorder
    void startRecording(); //recorder
    void stopRecording(); //recorder
    void deleteData(); //recorder
    void recordEvent(int, int, int, long); //recorder
    void writeToFile(ofstream*, void*); //recorder
    void dump();
};

#endif
