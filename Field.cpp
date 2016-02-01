#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include <iomanip>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#include "common.h"
#include "Field.h"
#include "Timer.h"
#include "Replay.h"

using namespace std;

extern void redisplay();

extern bool playReplay;
extern int gameState;
extern string playerName;
extern bool isFlagging;
extern Timer timer;
extern bool gamePaused;
extern Replay replay;
extern int hitMineX, hitMineY,squareSize;

extern void saveReplay(char *, Replay *);


extern bool hiScoreTestAndWrite(char *,string ,int , time_t, char* );



void Field::checkValues() {



    if (height<2) 
        height=2;
    else if (height>MAX_HEIGHT) 
        height=MAX_HEIGHT;
    

    if (width<2) 
        width=2;
    else if (width>MAX_WIDTH) 
        width=MAX_WIDTH;
    
    if (mineCount>=height*width)
        mineCount=height*width-1;
    else if (mineCount<1)
        mineCount=1;

}


bool Field::isMine(int x, int y) {

    if (x>0 && x>=width) 
        cerr << "x=="<<x<<" out of bounds"<<endl;
    else if (y<0 && y>=height)
        cerr << "y=="<<y<<" out of bounds"<<endl;
    else
        return mine[x][y];


    return false;
}

void Field::setMine(int x, int y) {

    mine[x][y]=true;
}


void Field::placeMines(int firstClickX, int firstClickY) {

    for (int i=0;i<width;i++) {
        for (int j=0;j<width;j++) {
            mine[i][j]=false;
        }
    }


    for (int i=0;i<mineCount;i++) {
        int x=rand()%width;
        int y=rand()%height;
        if (mine[x][y] or (abs(firstClickX-x)<=0 and abs(firstClickY-y)<=0))
            i--;
        else
            mine[x][y]=true;
    }
    redisplay();
  //  cout << "Mines placed." << endl;


}

void Field::init() {
    

    for (int i=0;i<width;i++) {
        for (int j=0;j<width;j++) {
            state[i][j]=9;
        }
    }
    gameState=GAME_INITIALIZED;
    //glutPostRedisplay();
    isFlagging=false;
    timer.reset();

    gamePaused=false;

    cout << endl;   // empty line in terminal between games


//    cout << "Field initialized." << endl;
}

void Field::revealAround(int squareX, int squareY) {

    // reveal squares around a square

    if (squareX>0) {
        if (squareY>0) revealSquare(squareX-1,squareY-1);
        revealSquare(squareX-1,squareY);
        if (squareY<height-1) revealSquare(squareX-1,squareY+1);
        
    }

    if (squareY>0) revealSquare(squareX,squareY-1);
    if (squareY<height-1) revealSquare(squareX,squareY+1);

    if (squareX<width-1) {
        if (squareY>0) revealSquare(squareX+1,squareY-1);
        revealSquare(squareX+1,squareY);
        if (squareY<height-1) revealSquare(squareX+1,squareY+1);
    }
    
}


void Field::revealSquare(int squareX, int squareY) {

    // clicked (or asked to reveal) unrevealed square


    if (state[squareX][squareY]==9) {
        if (mine[squareX][squareY]) {
            //timeFinished=time(NULL);
            hitMineX=squareX;
            hitMineY=squareY;
            timer.stop();
            replay.stopRecording();
            if (!playReplay) {
                cout << "YOU HIT A MINE. You played for "<<timer.calculateElapsedTime()
                    <<" milliseconds." << endl;
                saveReplay("last.replay",&replay);
            }
            
            gameState=GAME_LOST;
        }
        else {

            int adjacentMines=0;
            if (squareX>0) {
                if (squareY>0) adjacentMines+=mine[squareX-1][squareY-1]?1:0;
                adjacentMines+=mine[squareX-1][squareY]?1:0;
                if (squareY<height) adjacentMines+=mine[squareX-1][squareY+1]?1:0;
                
            }

            if (squareY>0) adjacentMines+=mine[squareX][squareY-1]?1:0;
            if (squareY<height-1) adjacentMines+=mine[squareX][squareY+1]?1:0;

            if (squareX<width-1) {
                if (squareY>0) adjacentMines+=mine[squareX+1][squareY-1]?1:0;
                adjacentMines+=mine[squareX+1][squareY]?1:0;
                if (squareY<height-1) adjacentMines+=mine[squareX+1][squareY+1]?1:0;
            }
            
          //  cout << "There are " << adjacentMines << " mines nearby." << endl;
            
            state[squareX][squareY]=adjacentMines;

            if (adjacentMines==0) {
                revealAround(squareX,squareY);
            }


            // test if game finished
            if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {
                bool notFinished=false;

                for (int i=0;i<width;i++) 
                    for (int j=0;j<height;j++) 
                        if (state[i][j]==9 and not mine[i][j])
                            notFinished=true;

                if (!notFinished) {
                    gameState=GAME_WON;
                    timer.stop();
                    replay.stopRecording();
                    
                    

                    if (!playReplay) {
                        cout << "YOU WIN! It took you "<<timer.calculateElapsedTime()
                            <<" milliseconds." << endl;
                        
                        cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;

                        char fname[100];
                        sprintf(fname,"%i-%i-%i%s.hiscore",width,height,mineCount,isFlagging?"":"-nf");
                        
                        char rfname[100];
                        long ts=time(NULL);
                        sprintf(rfname,"%lu.replay",ts);
                        
                        
                        if (hiScoreTestAndWrite(fname,playerName,timer.calculateElapsedTime(),
                            ts,rfname)) {
            
                            saveReplay(rfname,&replay);
                        }

                        saveReplay("last.replay",&replay);
                        
                    }


                    
                }    
            }
    
        }
    } 
}

void Field::checkAndRevealAround(int squareX,int squareY) {
    // count adjacent mines and possibly reveal adjacent squares

    int flaggedAdjacentMines=0;
    if (squareX>0) {
        if (squareY>0) flaggedAdjacentMines+=state[squareX-1][squareY-1]==10?1:0;
        flaggedAdjacentMines+=state[squareX-1][squareY]==10?1:0;
        if (squareY<height) flaggedAdjacentMines+=state[squareX-1][squareY+1]==10?1:0;
        
    }

    if (squareY>0) flaggedAdjacentMines+=state[squareX][squareY-1]==10?1:0;
    if (squareY<height) flaggedAdjacentMines+=state[squareX][squareY+1]==10?1:0;

    if (squareX<width) {
        if (squareY>0) flaggedAdjacentMines+=state[squareX+1][squareY-1]==10?1:0;
        flaggedAdjacentMines+=state[squareX+1][squareY]==10?1:0;
        if (squareY<height) flaggedAdjacentMines+=state[squareX+1][squareY+1]==10?1:0;
    }
    if (flaggedAdjacentMines==state[squareX][squareY]) {
        revealAround(squareX,squareY);
    }
}

int Field::calculateRemainingMines() {

    int remaining=mineCount;
    
    for (int i=0;i<width;i++) 
        for (int j=0;j<width;j++) 
            if (state[i][j]==10)
                remaining--;

    return remaining;
}


void Field::click(int x,int y,int button) {

    int squareX=(x-FIELD_X)/squareSize;
    int squareY=(y-FIELD_Y)/squareSize;
    
    if (button==GLUT_LEFT_BUTTON) {
        
        if (state[squareX][squareY]==9) {
            if (gameState==GAME_INITIALIZED) {
                if (!playReplay)
                    placeMines(squareX,squareY);

               

                timer.start();
                gameState=GAME_PLAYING;

                if (!replay.isRecording()) {
                    replay.deleteData();
                    replay.startRecording();
                }                                
            }
            replay.recordEvent(x,y,GLUT_LEFT_BUTTON);
            revealSquare(squareX,squareY);
        }
        else if (state[squareX][squareY]<=8) {
            replay.recordEvent(x,y,GLUT_LEFT_BUTTON);
            checkAndRevealAround(squareX,squareY);
        }

        
    }
    else if (button==GLUT_RIGHT_BUTTON) {

        if (!replay.isRecording()) {
            replay.deleteData();
            replay.startRecording();
        }

        replay.recordEvent(x,y,GLUT_RIGHT_BUTTON);
        // toggle flag or check and reveal surrounding squares
        if (state[squareX][squareY]==9) {
            state[squareX][squareY]=10;
            if (!isFlagging and !playReplay)
                cout<<"You are now playing with flagging."<<endl;

            isFlagging=true;
   //         cout << "Remaining mines: " << calculateRemainingMines() << endl;
        }
        else if (state[squareX][squareY]==10) {
            state[squareX][squareY]=9;
   //         cout << "Remaining mines: " << calculateRemainingMines() << endl;
        }
        else {
            checkAndRevealAround(squareX,squareY);
        }
            
        
    }
    else if (button==GLUT_MIDDLE_BUTTON and state[squareX][squareY]<=8) {
        replay.recordEvent(x,y,GLUT_MIDDLE_BUTTON);
        checkAndRevealAround(squareX,squareY);
    }

}
