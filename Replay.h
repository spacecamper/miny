#include <list>
#include <fstream>

#include <iostream>
#include <iomanip>


using namespace std;

class ReplayPoint {
public:
    
    int x, y;
    int button;
    long timeSinceStart;

    ReplayPoint();

    ReplayPoint(int, int, int, long);

    void dump();

};


void mouseClick(int,int,int,int);



class Replay {
public:

    std::list<ReplayPoint> data;
    std::list<ReplayPoint>::iterator nextPlayed;

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


    void writeToFile(ofstream*);


    void readFromFile(ifstream*);

    void dump();
    
    unsigned int playStep();

};

