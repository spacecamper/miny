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

    void readFromFile(ifstream*, void*); //player

    void dump(); // player
    
    unsigned int playStep(); //player
};
