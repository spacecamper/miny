#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Config.h"
#include "Timer.h"
#include "Player.h"
#include "Field.h"
#include "Action.h"
#include "scores.h"

extern Config conf;

int Player::loadReplay(const char *fname) {
    ifstream ifile;

    ifile.open(fname);
    if (!ifile.is_open()) {
        cerr<<"Error opening replay file '"<<fname<<"'."<<endl;
        return 1;
    }
    string content((istreambuf_iterator<char>(ifile) ), (istreambuf_iterator<char>()    )) ;
    if (string::npos != content.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.@- \x0d\x0a-:")) {
        cout << "Replay file contains invalid characters. Exiting."<<endl;
        exit(1);
    }

    ifile.close();
    ifile.open(fname);

    readFromFile(&ifile);

    ifile.close();

    return 0;
}

Player::Player() {
    refreshQueue();
    nextPlayed = data.begin();
}

void Player::refreshQueue() {
    Action* action = new Action(0, 0, -1, 0);
    data.push_back(*action);
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


    replayHasScore=false;

    switch(fileVersion) {
    case -1:
        cout << "Unknown replay file format. Exiting." << endl;
        exit(1);
    case 1: {

        string tmpname;
        
        *ifile >> field.playerName; 
        
        *ifile >> conf.squareSize;
        
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

            if (i==0)
                rp->timeSinceStart=0;

            data.push_back(*rp);
//            cout << "rp" << endl;
        }

        break;
        }
    case 2: {

        string tmpname;
        
        replayHasScore=true;
        score.readFromFile(ifile);
        strcpy(field.playerName,score.name);
        conf.squareSize=score.squareSize;
        field.width=score.width;
        field.height=score.height;


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

            if (i==0)
                rp->timeSinceStart=0;

            data.push_back(*rp);
        }

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
    
    if (nextPlayed->timeSinceStart!=-1){
        if (nextPlayed->timeSinceStart > field.timer.calculateElapsedTime()) {
            return true;
        }
        
        if ((*nextPlayed).button>=-1) {
            takeAction(nextPlayed->button, nextPlayed->x, nextPlayed->y);
        }
        else {
            takeAction((unsigned char)(-nextPlayed->button), nextPlayed->x, nextPlayed->y);
        }
        nextPlayed->timeSinceStart=-1;
    }

    if (next==data.end()) {
        if (conf.playReplay) {
            cout<<"End of Replay."<<endl;
            return false;
        }
    }
    else {
        nextPlayed=next;
    }
    return true;
}

void Player::takeAction(int button, int x, int y) {
    if ((conf.gameState==Config::GAME_INITIALIZED or conf.gameState==Config::GAME_PLAYING)) {
        // this should've been "x>=FIELD_X" and "y>=FIELD_Y", now
        // the top row and left column are 1 pixel narrower
        if (x>FIELD_X and x<FIELD_X+field.width*conf.squareSize 
            and y>FIELD_Y and y<FIELD_Y+field.height*conf.squareSize) { // field
            field.click(x, y, button);
        }

        else if (x>conf.targetWidth()/2-12-DISPLAY_BORDER_WIDTH/2 and
                x<conf.targetWidth()/2+12+DISPLAY_BORDER_WIDTH/2 and
                y>BORDER_WIDTH and
                y<BORDER_WIDTH+24+DISPLAY_BORDER_WIDTH and
                button!=-1) {
            field.newGame();
        }
        glutPostRedisplay();
    }
    else if (!(x>FIELD_X and
            x<FIELD_X+field.width*conf.squareSize and
            y>FIELD_Y and
            y<FIELD_Y+field.height*conf.squareSize) and
            button!=-1) { // outside of field - new game
        field.newGame();
    }
    cursorX=x;
    cursorY=y;
}

void Player::takeAction(unsigned char button, int x, int y) {
    switch (button) {
    case ' ':
        if (!conf.gamePaused and !conf.playReplay) {
            field.newGame();
        }
        break;
    case 'p':   // pause
        if (conf.gameState==Config::GAME_PLAYING and !conf.playReplay) {
            if (!conf.gamePaused) {

                conf.gamePaused=true;
                field.timer.pause(); 
                field.replay.recording = false;
                cout << "Game paused. Press P to continue. Elapsed time: "
                    <<field.timer.calculateElapsedTime()<<" ms"<<endl;
            }
            else {
                field.unpauseGame();
            }
        }
        break;
    case 'r':
        field.replay.dump();
        cout << field.calculate3BV() << endl;
        break;

    case 'q':
        exit(0);
    case 27:    // escape
        exit(0);
        break;
    }
}

void Player::handleInput(int button, int x, int y) {
    Action* input = new Action(x, y, button, 0);
    data.push_back(*input);
 //   field.replay.data.push_back(*(new Action(x,y,button,(gameState==Config::GAME_INITIALIZED ? 0 : field.timer.calculateElapsedTime()))));
 //   field.replay.recordEvent(x,y,button,field.timer.calculateElapsedTime());
}
