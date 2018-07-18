#ifndef PLAYER_H
#define PLAYER_H

#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>

#include "Action.h"
#include "Field.h"

class Player {
public:
    std::list<Action> data;
    std::list<Action>::iterator nextPlayed;
    
    Field field;
    bool playbackOn;
    
    int cursorX,cursorY; //player
    
    Player();
    
    void handleInput(int, int, int);
    bool playStep(bool); //player
    int loadReplay(const char* fname);
    void refreshQueue();
    
private:
    void readFromFile(ifstream*); //player
    void takeAction(unsigned char, int, int);
    void takeAction(int, int, int);
};

#endif
