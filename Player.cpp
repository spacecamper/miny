#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "common.h"
#include "Timer.h"
#include "Player.h"
#include "Field.h"
#include "Action.h"

extern char playerName[21];
extern int squareSize;
extern Timer timer;

void mouseClick(int,int,int,int);

void Player::readFromFile(ifstream *ifile, void* fieldPtr) {

    Field* field = (Field*)fieldPtr;

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
        
        *ifile >> field->width >> field->height;
        
        int mineCount=0;
        bool tmpmine;
        for (int j=0;j<field->height;j++) 
            for (int i=0;i<field->width;i++) {
                
                *ifile >> tmpmine;
                if (tmpmine) {
                    field->setMine(i,j);
                    mineCount++;    
                }
            }

        field->mineCount=mineCount;

        int count;
        *ifile>>count;
        Action *rp;

        for (int i=0;i<count;i++) {
            rp=new Action();

            *ifile >> rp->timeSinceStart >> rp->x >> rp->y >> rp->button;
            data.push_back(*rp);
        }

        nextPlayed=data.begin();
        break;
        }
    default:
        cout << "Unknown replay file version. You are probably running an old version of the"<<endl<<"game. Please upgrade to the latest version." << endl;
        exit(1);
    }



}


void Player::dump() {
    cout << "Dumping replay data."<<endl;

    
    std::list<Action>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        (*iter).dump();

    }
}

unsigned int Player::playStep() {

    cursorX=(*nextPlayed).x;
    cursorY=(*nextPlayed).y;

    if ((*nextPlayed).button!=-1) {
        
        mouseClick((*nextPlayed).button,GLUT_DOWN,(*nextPlayed).x,(*nextPlayed).y);

    }

    std::list<Action>::iterator next;

    next=nextPlayed;

    next++;

    if (next==data.end()) {
        return -1;
    }
    else {

        int ret=(*(next)).timeSinceStart-timer.calculateTimeSinceStart();
        
        if (ret<0) ret=0;

        nextPlayed=next;
        
        return ret;
    }
}
