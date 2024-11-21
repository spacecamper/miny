#ifndef PLAYER_H
#define PLAYER_H

#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>

#include "Action.h"
#include "Field.h"
#include "scores.h"

class Player {
public:
    Field field;
    bool playbackOn;
    Score score;
	bool replayHasScore;
    int cursorX,cursorY; //player
    
    Player();
    
    void handleInput(int, int, int);
    bool playStep(bool); //player
    int loadReplay(const string& fname);
    
private:
    std::list<Action> data;
    std::list<Action>::iterator nextPlayed;
    
    void readFromFile(ifstream*); //player
    void takeAction(unsigned char, int, int);
    void takeAction(int, int, int);
    void refreshQueue();
};

#endif
