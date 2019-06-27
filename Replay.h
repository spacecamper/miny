#ifndef REPLAY_H
#define REPLAY_H

#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>

#include "Action.h"
#include "scores.h"


using namespace std;

class Replay {
public:

    bool recording;
    Score score;

    std::list<Action> data;

    Replay();

    void deleteData(); //recorder
    void recordEvent(int, int, int, long); //recorder
    void writeToFile(ofstream*, void*,Score*); //recorder
    void dump();
};

#endif
