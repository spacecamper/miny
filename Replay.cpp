#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "common.h"
#include "Timer.h"
#include "Replay.h"
#include "Field.h"
#include "Action.h"

extern bool playReplay;
extern int gameState;
extern Timer timer;
extern char playerName[21];
extern int squareSize;
extern Field field;

Replay::Replay() {
    recording=false;
}

bool Replay::isRecording() {
    return recording;
}


void Replay::startRecording() {

    if (!playReplay) recording=true;

}


void Replay::pauseRecording() {

    if (!playReplay) recording=false;

}

void Replay::resumeRecording() {

    if (!playReplay) recording=true;

}



void Replay::stopRecording() {

    if (!playReplay) recording=false;

}

void Replay::deleteData() {
    if (!playReplay) {
        data.clear();
    }
}

void Replay::recordEvent(int x, int y, int button, long elapsedTime) {
    if (recording) {
        data.push_back(*(new Action(x,y,button,(gameState==GAME_INITIALIZED ? 0 : elapsedTime))));
    }
}

void Replay::writeToFile(ofstream *file, void* fieldPtr) {

    Field* field = (Field*)fieldPtr;    
    
   // cout << "width=" << field.width << endl;
    *file << "miny-replay-file-version: 1" << endl;

    *file << playerName << endl << squareSize << endl; 

    *file << field->width << " " << field->height << endl;

    for (int j=0;j<field->height;j++) {
        for (int i=0;i<field->width;i++) 
            *file << field->isMine(i,j) << " ";
        *file << endl;
    }

    *file << endl;

    *file << data.size() << endl;

    std::list<Action>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        *file << (*iter).timeSinceStart << " " << (*iter).x << " " << (*iter).y << " " << (*iter).button << endl;
    }
}

void Replay::dump() {
    cout << "Dumping replay data."<<endl;
    
    std::list<Action>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        (*iter).dump();
    }
}
