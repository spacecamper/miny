#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>


using namespace std;

class Action {
public:
    
    int x, y;
    int button;
    long timeSinceStart;

    Action();

    Action(int, int, int, long);

    void dump();
};


void mouseClick(int,int,int,int);



class Replay {
public:

    std::list<Action> data; //both
    std::list<Action>::iterator nextPlayed; //both

    bool recording;


    int cursorX,cursorY; //player


    Replay();

    bool isRecording(); //recorder

    void startRecording(); //recorder

    void pauseRecording(); //recorder

    void resumeRecording(); //recorder

    void stopRecording(); //recorder

    void deleteData(); //recorder

    void recordEvent(int, int, int); //recorder

    void writeToFile(ofstream*, void*); //recorder

    void readFromFile(ifstream*, void*); //player

    void dump(); // player
    
    unsigned int playStep(); //player

};
/*
class Player {
public:
    std::list<Action> data;
    std::list<Action>::iterator nextPlayed;
    
}
*/
