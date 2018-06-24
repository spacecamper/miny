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

    std::list<Action> data;
    std::list<Action>::iterator nextPlayed;

    bool recording;
    bool endOfReplay;


    int cursorX,cursorY;


    Replay();

    bool isRecording();

    void startRecording();

    void pauseRecording();

    void resumeRecording();

    void stopRecording();

    void deleteData();

    void recordEvent(int, int, int);

    void writeToFile(ofstream*, void*);

    void readFromFile(ifstream*, void*);

    void dump();
    
    unsigned int playStep();

};

