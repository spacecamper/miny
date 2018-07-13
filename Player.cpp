#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Timer.h"
#include "Player.h"
#include "Field.h"
#include "Action.h"

extern char playerName[21];
extern int squareSize;

void mouseClick(int,int,int,int);

int Player::loadReplay(const char *fname) {
    ifstream ifile;

    ifile.open(fname);
    if (!ifile.is_open()) {
        cerr<<"Error opening replay file '"<<fname<<"'."<<endl;
        return 1;
    }
    string content((istreambuf_iterator<char>(ifile) ), (istreambuf_iterator<char>()    )) ;
    if (string::npos != content.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_ \x0d\x0a-:")) {
        cout << "Replay file contains invalid characters. Exiting."<<endl;
        exit(1);
    }

    ifile.close();
    ifile.open(fname);

    readFromFile(&ifile);

    ifile.close();

    return 0;
}

void Player::readFromFile(ifstream *ifile) {

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
        
        *ifile >> field.playerName; 
        
        *ifile >> squareSize;
        
        *ifile >> field.width >> field.height;
        
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

bool Player::playStep(bool firstClick) {
    std::list<Action>::iterator next;
    next=nextPlayed;
    next++;

    if (next->timeSinceStart >= field.timer.calculateTimeSinceStart() and !firstClick and next!=data.end()) {
        return true;
    }
    if ((*nextPlayed).button!=-1) {
        mouseClick((*nextPlayed).button,GLUT_DOWN,(*nextPlayed).x,(*nextPlayed).y);
    }
    else {
        cursorX=(*nextPlayed).x;
        cursorY=(*nextPlayed).y;
    }

    if (next==data.end()) {
        return false;
    }
    else {
        nextPlayed=next;
        return true;
    }
}
