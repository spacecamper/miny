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



extern bool playReplay;
extern int gameState;
extern Timer timer;
extern char playerName[21];
extern int squareSize;
extern Field field;


ReplayPoint::ReplayPoint() {}

ReplayPoint::ReplayPoint(int px, int py, int pb, long pp) {
    x=px;
    y=py;
    button=pb;
    timeSinceStart=pp;
}

void ReplayPoint::dump() {
    cout <<setw(7)<<timeSinceStart<<setw(7) <<x<<setw(7)<<y<<setw(7)<<button<< endl;
}



void mouseClick(int,int,int,int);





Replay::Replay() {
    recording=false;
    endOfReplay=false;
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

    if (!playReplay) data.clear();
}

void Replay::recordEvent(int x, int y, int button) {

    if (recording) {
     //   cout << "Recording event " << x << " " << y << " " << button << "." << endl;
      //  data.push_back(*(new ReplayPoint(x,y,button,(gameState==GAME_INITIALIZED ? 0 : timer.calculateTimeSinceStart()))));
        data.push_back(*(new ReplayPoint(x,y,button,(gameState==GAME_INITIALIZED ? 0 : timer.calculateElapsedTime()))));

    }

}


void Replay::writeToFile(ofstream *file) {
   // cout << "width=" << field.width << endl;
    *file << "miny-replay-file-version: 1" << endl;

    *file << playerName << endl << squareSize << endl; 

    *file << field.width << " " << field.height << endl;

    for (int j=0;j<field.height;j++) {
        for (int i=0;i<field.width;i++) 
            *file << field.isMine(i,j) << " ";
        *file << endl;
    }

    *file << endl;

    *file << data.size() << endl;

    std::list<ReplayPoint>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        *file << (*iter).timeSinceStart << " " << (*iter).x << " " << (*iter).y << " " << (*iter).button << endl;

    }

}


void Replay::readFromFile(ifstream *ifile) {


    string firstString;

    *ifile >> firstString;

    

    int fileVersion;

    if (firstString=="miny-replay-file-version:") {
        *ifile >> fileVersion;
    }
    else {
        
        fileVersion=-1;

    }

    switch(fileVersion) {
    case -1:
        cout << "Unknown replay file format. Exiting." << endl;
        exit(1);
    case 1: {

        string tmpname;
        
        *ifile >> playerName; 
        
        *ifile >> squareSize;
        
        *ifile >> field.width >> field.height;

      //  cout << "Reading mines."<<endl;

        
        int mineCount=0;
        bool tmpmine;
        for (int j=0;j<field.height;j++) 
            for (int i=0;i<field.width;i++) {
                
                *ifile >> tmpmine;
                if (tmpmine) {
                    field.setMine(i,j);
                    mineCount++;    
                }
            }

        field.mineCount=mineCount;

        int count;
        *ifile>>count;
    //    cout << "Reading " << count << " events." << endl;
        ReplayPoint *rp;

        for (int i=0;i<count;i++) {
            rp=new ReplayPoint();

            *ifile >> rp->timeSinceStart >> rp->x >> rp->y >> rp->button;
            data.push_back(*rp);
     //       cout << ".";
        }
    //    cout << endl;            
   //     cout << "Loaded " << data.size() << " events."<<endl;
        

       /* while (!ifile.eof()) {
            ifile >> scores[*count].name >> scores[*count].time >> scores[*count].timeStamp;
            (*count)++;
            if ((*count)==MAX_HS) break;
        }

        if (ifile.eof())
            (*count)--;*/

        nextPlayed=data.begin();
        break;
        }
    default:
        cout << "Unknown replay file version. You are probably running an old version of the"<<endl<<"game. Please upgrade to the latest version." << endl;
        exit(1);
    }



}


void Replay::dump() {
    cout << "Dumping replay data."<<endl;
       // std::mem_fun(&ReplayPoint::dump)
  //  std::for_each(  data.begin(), data.end(), std::mem_fun(&ReplayPoint::dump));

    
    std::list<ReplayPoint>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        (*iter).dump();

    }
}

unsigned int Replay::playStep() {

 //   cout << "Playing step."<<endl;

    cursorX=(*nextPlayed).x;
    cursorY=(*nextPlayed).y;

    if ((*nextPlayed).button!=-1) {
        
        mouseClick((*nextPlayed).button,GLUT_DOWN,(*nextPlayed).x,(*nextPlayed).y);

    }

    std::list<ReplayPoint>::iterator next;

    next=nextPlayed;

    next++;

    if (next==data.end()) {
        return -1;
    }
    else {

        int ret=(*(next)).timeSinceStart-timer.calculateTimeSinceStart();//-(*nextPlayed).timeSinceStart;
        
        if (ret<0) ret=0;

        nextPlayed=next;
        
        return ret;
    }
}



