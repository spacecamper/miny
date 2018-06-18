/*
 * Miny
 * a minesweeper clone
 * (c) 2015-2018 spacecamper
 */

#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
#include <iomanip>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif



#include "Timer.h"
#include "Replay.h"
#include "common.h"
#include "Field.h"
#include "scores.h"


#define VERSION "0.5.11"


// TODO elapsed time isn't being redrawn while playing replay when there's a long pause between two 
//   events
// TODO prevent buffer overflows (strcpy and strcat)
// TODO free allocated memory (after using scores from loadScores and filterScores is finished)


using namespace std;

int windowWidth, windowHeight, originalWidth, originalHeight;
int squareSize;

int gameState; // -1 - initialized, 0 - playing, 1 - lost, 2 - won
char option_char;

unsigned short hitMineX,hitMineY;  // when game is lost
char playerName[21];
char highScoreDir[100];
bool isFlagging;
bool gamePaused;
bool playReplay;
bool anyRank;

Timer timer;

Replay replay;

Field field;

void redisplay() {
    glutPostRedisplay();
}


bool directoryExists( const char* pzPath )
{
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
    }

    return bExists;
}



void newGame() {

    field.init();
    cout << "------------------------------------------------------------" << endl; 


}


// ---------------------- REPLAY ------------------//
void saveReplay(const char *fname, Replay *r) {


    ofstream ofile;
    
    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
  //  cout << "Writing replay file " << fullpath <<"..."<<flush;

    ofile.open (fullpath);

    if (!ofile.is_open()) {
        cerr<<"Error opening file " << fullpath << " for writing replay."<<endl;
        return;
    }

    r->writeToFile(&ofile);

    ofile.close();

 //   cout << "Done."<<endl;
    

}

int loadReplay(char *fname, Replay *r) {
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

    r->readFromFile(&ifile);


    ifile.close();

    return 0;

}





// ----------------------- GRAPHICS -------------------- //


float mySin(float v) {
    return sin(v/ 180 * 3.141592654f);
}


float myCos(float v) {
    return cos(v/ 180 * 3.141592654f);
}

float myTan(float v) {
    return tan(v/ 180 * 3.141592654f);
}


 

void drawRect(float x, float y, float w, float h) {

    glBegin(GL_TRIANGLES);

    glVertex2f(x,y);
    glVertex2f(x+w,y);
    glVertex2f(x,y+h);

    glVertex2f(x,y+h);
    glVertex2f(x+w,y+h);
    glVertex2f(x+w,y);

    glEnd();
}

void drawDigitRect(int i, int x, int y, float zoom=1) {
    // draws a rectangle that's part of a digit

    float u;

    u=2*zoom; 

    
    switch(i) {
    case 0: drawRect(x,y,3*u,u); break;
    case 1: drawRect(x,y,u,3*u); break;
    case 2: drawRect(x+2*u,y,u,3*u); break;
    case 3: drawRect(x,y+2*u,3*u,u); break;
    case 4: drawRect(x,y+2*u,u,3*u); break;
    case 5: drawRect(x+2*u,y+2*u,u,3*u); break;
    case 6: drawRect(x,y+4*u,3*u,u); break;
    case 7: drawRect(x+u,y,u,3*u); break;
    case 8: drawRect(x+u,y+2*u,u,3*u); break;
    
    }


}


void drawDigit(int n, int x, int y, float zoom) {
    // draw a digit 10 px high, 6 px wide (if zoom==1)

    switch(n) {
    case 0:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(4,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 1:
        drawDigitRect(7,x,y,zoom);
        drawDigitRect(8,x,y,zoom);
        break;
    
    case 2:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(4,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 3:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 4:
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        break;
    case 5:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 6:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(4,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 7:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        break;

    case 8:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(4,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    case 9:
        drawDigitRect(0,x,y,zoom);
        drawDigitRect(1,x,y,zoom);
        drawDigitRect(2,x,y,zoom);
        drawDigitRect(3,x,y,zoom);
        drawDigitRect(5,x,y,zoom);
        drawDigitRect(6,x,y,zoom);
        break;
    default:
        drawDigitRect(3,x,y,zoom);
        
    }

}






void drawScene() {
    glClearColor(.7, .7, .7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 

    if (playReplay) {
        // draw cursor
        glColor3f(1,1,0);
        glBegin(GL_TRIANGLES);

        glVertex2f(replay.cursorX,replay.cursorY);
        glVertex2f(replay.cursorX,replay.cursorY+20);
        glVertex2f(replay.cursorX+11,replay.cursorY+17);
        glEnd();
    }




    // number of remaining mines
    glColor3f(1,0,0);
    int rem=field.calculateRemainingMines();


    if (rem>999) rem=999;

    int dxy=BORDER_WIDTH+DISPLAY_BORDER_WIDTH;

    for (int i=0;i<3;i++) {
        int digit=rem%10;
        rem/=10;
        drawDigit(digit,32-16*i+dxy,dxy,2);
    }

    glColor3f(0,0,0);
    drawRect(BORDER_WIDTH,
            BORDER_WIDTH,
            48+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);



    // elapsed time

    glColor3f(1,0,0);


    long etime=timer.calculateElapsedTime()/1000;


    if (etime>999) etime=999;

    int dx=originalWidth-BORDER_WIDTH-16;
    int dy=BORDER_WIDTH+DISPLAY_BORDER_WIDTH;

    for (int i=0;i<3;i++) {
        int digit=etime%10;
        etime/=10;
        drawDigit(digit,-16*i+dx,dy,2);
    }

    glColor3f(0,0,0);
    drawRect(dx-32-DISPLAY_BORDER_WIDTH,
            BORDER_WIDTH,
            16*3+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);



    
    // new game button

    glColor3f(1,1,0);
    drawRect(originalWidth/2-12-DISPLAY_BORDER_WIDTH/2,
            BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);




    if (gamePaused) {    // hide field when game is paused


        glColor3f(.3,.3,.3);


        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y);
  
        glVertex2f(FIELD_X,FIELD_Y+field.height*squareSize);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+field.height*squareSize);
   
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X,FIELD_Y+field.height*squareSize);
   
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+field.height*squareSize);
        glEnd();


        glColor3f(.5,.5,.5);

        glBegin(GL_TRIANGLES);
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+field.height*squareSize);

        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X,FIELD_Y+field.height*squareSize);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+field.height*squareSize);
       
        glEnd();

    }
    else {

        // grid lines

/*
        float bg=.5;
        float target=.7;

        float c=1-.5*((1-target)-(1-bg))*squareSize;
        
cout << c << endl;
        if (c>.7) c=.7; else if (c<0) c=0;



        glColor3f(c,c,c);
  */  
        glColor3f(.3,.3,.3);
      

        glBegin(GL_LINES); 

        for (int i=0;i<field.height+1;i++) {
            glVertex2f(FIELD_X,FIELD_Y+i*squareSize);
            glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+i*squareSize);
        }

        for (int i=0;i<field.width+1;i++) {
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y);
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y+field.height*squareSize);
        }

        glEnd();

        // squares
        

        for (int x=0;x<field.width;x++) 
            for (int y=0;y<field.height;y++) {
                int x1=FIELD_X+x*squareSize;
                int x2=FIELD_X+(x+1)*squareSize-1;
                int y1=FIELD_Y+y*squareSize;
                int y2=FIELD_Y+(y+1)*squareSize-1;

                
                if (field.state[x][y]>=0 and field.state[x][y]<=8) {    // revealed square
                  
                   /* glColor3f(.6,.6,.6);

                    glBegin(GL_LINES);
                    glVertex2f(x1,y1);
                    glVertex2f(x2,y1);

                    glVertex2f(x1,y1);
                    glVertex2f(x1,y2);
                    glEnd();
  
                    glColor3f(.8,.8,.8);

                    glBegin(GL_LINES);
                    glVertex2f(x2,y2);
                    glVertex2f(x2,y1);

                    glVertex2f(x2,y2);
                    glVertex2f(x1,y2);
                    glEnd();*/





                    switch(field.state[x][y]) {
                    case 0: glColor3f(.5,.5,.5); break;
                    case 1: glColor3f(0,0,1); break;
                    case 2: glColor3f(0,1,0); break;
                    case 3: glColor3f(1,0,0); break;
                    case 4: glColor3f(0,0,.5); break;
                    case 5: glColor3f(.5,0,0); break;
                    case 6: glColor3f(0,1,1); break;
                    case 7: glColor3f(0,0,0); break;
                    case 8: glColor3f(.7,.7,.7); break;

                    default: glColor3f(0,0,0);
                    }


                    if (field.state[x][y]!=0) {
                        // number
                        float zoom=1.5*squareSize/25;

                        drawDigit(field.state[x][y],x1+.5*squareSize-3.0*zoom,
                                    y1+.5*squareSize-5.0*zoom,zoom);
                    }


                    // background
                    glColor3f(.5,.5,.5);

                    glBegin(GL_TRIANGLES);
                    glVertex2f(x1,y1);
                    glVertex2f(x2,y1);
                    glVertex2f(x2,y2);

                    glVertex2f(x1,y1);
                    glVertex2f(x1,y2);
                    glVertex2f(x2,y2);
          

                    
                    glEnd();
                }
                else {
                    
                   /* glColor3f(.8,.8,.8);

                    glBegin(GL_LINES);
                    glVertex2f(x1,y1);
                    glVertex2f(x2,y1);

                    glVertex2f(x1,y1);
                    glVertex2f(x1,y2);
                    glEnd();
  
                    glColor3f(.6,.6,.6);

                    glBegin(GL_LINES);
                    glVertex2f(x2,y2);
                    glVertex2f(x2,y1);

                    glVertex2f(x2,y2);
                    glVertex2f(x1,y2);
                    glEnd();*/


                    if (field.state[x][y]==9 
                         and (gameState==GAME_LOST or gameState==GAME_WON /* or true*/)
                         and field.isMine(x,y)
                        ) { // unflagged mine when game is over
                                        
                        glColor3f(0,0,0);
                        glBegin(GL_TRIANGLES);
                        glVertex2f(x1+.5*squareSize,y1+.1*squareSize);
                        glVertex2f(x1+.1*squareSize,y1+.5*squareSize);
                        glVertex2f(x1+.5*squareSize,y2-.1*squareSize);

                        glVertex2f(x1+.5*squareSize,y1+.1*squareSize);
                        glVertex2f(x2-.1*squareSize,y1+.5*squareSize);
                        glVertex2f(x1+.5*squareSize,y2-.1*squareSize);
                        
                        glEnd();

                        float gap=.25*squareSize;

                        glBegin(GL_TRIANGLES);
                        glVertex2f(x1+gap,y1+gap);
                        glVertex2f(x2-gap,y1+gap);
                        glVertex2f(x2-gap,y2-gap);

                        glVertex2f(x1+gap,y1+gap);
                        glVertex2f(x1+gap,y2-gap);
                        glVertex2f(x2-gap,y2-gap);
                
                        glEnd();
                    }
                    else if (field.state[x][y]==10) {  // flag

                        // cross out flag where there is no mine

                        if (gameState==GAME_LOST and !field.isMine(x,y)) {
                            float crossGap=.1*squareSize;
                            glColor3f(0,0,0);
                            glBegin(GL_LINES);
                            glVertex2f(x1+crossGap,y1+crossGap);
                            glVertex2f(x2-crossGap,y2-crossGap);

                            glVertex2f(x2-crossGap,y1+crossGap);
                            glVertex2f(x1+crossGap,y2-crossGap);
                            glEnd();
                        }


                        // flag


                        glColor3f(1,0,0);
                        glBegin(GL_TRIANGLES);
                        glVertex2f(x1+.1*squareSize,y1+.3*squareSize);
                        glVertex2f(x1+.6*squareSize,y1+.1*squareSize);
                        glVertex2f(x1+.6*squareSize,y1+.5*squareSize);
                        glColor3f(0,0,0);
                        glVertex2f(x1+.6*squareSize,y1+.6*squareSize);
                        glVertex2f(x1+.15*squareSize,y1+.9*squareSize);
                        glVertex2f(x1+.9*squareSize,y1+.9*squareSize);


                        glEnd();


                    }

                }

            }
    }

    // background for hit mine

    int hitMineDisplayX1=FIELD_X+hitMineX*squareSize;
    int hitMineDisplayX2=FIELD_X+(hitMineX+1)*squareSize;
    int hitMineDisplayY1=FIELD_Y+hitMineY*squareSize;
    int hitMineDisplayY2=FIELD_Y+(hitMineY+1)*squareSize;
    


    if (gameState==GAME_LOST) {
        glColor3f(1,0,0);

        glBegin(GL_TRIANGLES);
        glVertex2f(hitMineDisplayX1,hitMineDisplayY1);
        glVertex2f(hitMineDisplayX2,hitMineDisplayY1);
        glVertex2f(hitMineDisplayX2,hitMineDisplayY2);

        glVertex2f(hitMineDisplayX1,hitMineDisplayY1);
        glVertex2f(hitMineDisplayX1,hitMineDisplayY2);
        glVertex2f(hitMineDisplayX2,hitMineDisplayY2);
        glEnd();
    }




    glutSwapBuffers();

}


// ------------------------- GAME ---------------------- //

void unpauseGame() {
    gamePaused=false;
    timer.unpause();
    replay.resumeRecording();
    cout << "Game unpaused."<<endl;// Elapsed time: "<<calculateElapsedTime()<<" ms"<<endl;
}



void endGameLost() {

    redisplay();

    cout << endl<< "YOU HIT A MINE. You played for " << setprecision(3) << fixed <<
        timer.calculateElapsedTime()/1000.0 <<" seconds." << endl << "3BV:  " 
        << setprecision(4) << fixed << field.get3BV() << endl;


    long timeTaken=timer.calculateElapsedTime();
    long ts=time(NULL);
    
    Score newScore;

    newScore.timeStamp=ts;
    strcpy(newScore.name,playerName);
    newScore.width=field.width;
    newScore.height=field.height;
    newScore.mines=field.mineCount;

    newScore.time=timeTaken;
    newScore.val3BV=field.get3BV();
    newScore.flagging=isFlagging;


    newScore.effectiveClicks=field.effectiveClicks;
    newScore.ineffectiveClicks=field.ineffectiveClicks;

    newScore.squareSize=squareSize;
    newScore.gameWon=false;
    

    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,"scores.dat");


    newScore.replayNumber=0;
    appendScore(fullpath,newScore);

    saveReplay("last.replay",&replay);

}


bool replayFileNumberExists(long nr) {


        struct stat buffer;  
char rfname[100];
        
    char tmp[100];
    strcpy(tmp,highScoreDir);
    sprintf(rfname,"%lu.replay",nr);

    strcat(tmp,rfname);
    
    if (stat(tmp, &buffer) != 0)
        return false;


    return true;


    

}



long findLowestUnusedReplayNumber() {


    long lower=1;
    
    long upper=1;
    
    while (true) {

        if (!replayFileNumberExists(upper)) break;

        lower=upper;
        upper*=2;


    }

    long middle=0;



    // binary search

    while (lower<upper-1) {

        middle=(lower+upper)/2;
        
        if (replayFileNumberExists(middle))
            lower=middle;
        else
            upper=middle;
    }


    return upper;



}



long findLowestUnusedReplayNumber_old() {


    // linear search

    long nr=1;


    while (true) {

        if (!replayFileNumberExists(nr))
            break;

        nr++;

    }


    return nr;
}



void endGameWon() {

    redisplay();

    gameState=GAME_WON;
    timer.stop();
    replay.stopRecording();


    long timeTaken;

    timeTaken=timer.calculateElapsedTime();

    long ts=time(NULL);

    Score newScore;

    newScore.timeStamp=ts;
    strcpy(newScore.name,playerName);
    newScore.width=field.width;
    newScore.height=field.height;
    newScore.mines=field.mineCount;

    newScore.time=timeTaken;
    newScore.val3BV=field.get3BV();
    newScore.flagging=isFlagging;

    newScore.effectiveClicks=field.effectiveClicks;
    newScore.ineffectiveClicks=field.ineffectiveClicks;

    newScore.squareSize=squareSize;
    newScore.gameWon=true;


    cout << endl<<"YOU WIN!"<<endl;
    
    if (!playReplay) {
        cout << setw(8)<<left << "Time: " << setprecision(3) << fixed << timeTaken/1000.0
            << " s" << endl;
        cout << setw(8)<<left << "3BV/s: " << setprecision(4)<< fixed<<newScore.get3BVs()<<endl;
    }

    cout <<setw(8)<<left << "IOE: " << setprecision(4)<<fixed<< newScore.getIOE()<<endl;
    
    cout << setw(8)<<left << "3BV: " << field.get3BV()<<endl;

    cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;
    cout << endl;



    if (!playReplay) {
        
        

        long nr=1;

       
        

        
        

        char fullpath[100];
        strcpy(fullpath,highScoreDir);
        strcat(fullpath,"scores.dat");

        Score *scores;

        int count=loadScores(fullpath,&scores);

        evalScore(newScore,scores, count,field.width,field.height,field.mineCount,anyRank);

        free(scores); 
        

        // find the lowest unused replay file number

    //    cout << "Finding lowest unused replay number..."<<endl;
 

        nr=findLowestUnusedReplayNumber();    
   
        newScore.replayNumber=nr;
        appendScore(fullpath,newScore);


        char rfname[100];

        char tmp[100];
        strcpy(tmp,highScoreDir);
        sprintf(rfname,"%lu.replay",nr);

        saveReplay(rfname,&replay);

        saveReplay("last.replay",&replay);

    }


}



// -------------------------- GLUT ----------------------- //

void keyDown(unsigned char key, int x, int y) {

    switch (key) {
    case ' ':   
        if (!gamePaused and !playReplay) 
            newGame();

        break;
    case 'p':   // pause
        if (gameState==GAME_PLAYING and !playReplay) {
            if (!gamePaused) {

                gamePaused=true;
                timer.pause(); 
                replay.pauseRecording();
                cout << "Game paused. Press P to continue. Elapsed time: "
                    <<timer.calculateElapsedTime()<<" ms"<<endl;
            }
            else
                unpauseGame();
            
        }
        break;
    case 'r':
        replay.dump();
        break;
    case 'd':
        cout << sizeof(Score)<<endl;
        break;

    case 'q':
    case 27:    // escape
        exit(0);
    

    }


}


void mouseClick(int button, int mState, int x, int y) {


    if (!gamePaused) {
        if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {

            if (x>FIELD_X and x<FIELD_X+field.width*squareSize 
                and y>FIELD_Y and y<FIELD_Y+field.height*squareSize) {

                // field

                if (mState==GLUT_DOWN) {
                
                    field.click(x,y,button);
                }
            }

            else if (x>originalWidth/2-12-DISPLAY_BORDER_WIDTH/2 && x<originalWidth/2+12+DISPLAY_BORDER_WIDTH/2 &&
                y>BORDER_WIDTH && y<BORDER_WIDTH+24+DISPLAY_BORDER_WIDTH) {

                // new game button
                if (mState==GLUT_DOWN) {
        
                    newGame();
                }

            }

            glutPostRedisplay();
        }
        else if (!(x>FIELD_X and x<FIELD_X+field.width*squareSize 
            and y>FIELD_Y and y<FIELD_Y+field.height*squareSize)) {

            // outside of field - new game

            newGame();
        }
    }

}

 
void handleResize(int w, int h) {

    windowWidth=w;
    windowHeight=h;
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

}



void update(int value) {
    glutPostRedisplay(); 
	
    glutTimerFunc(50, update, 0);
}


void updateR(int value) {
    int delay=replay.playStep();

    glutPostRedisplay();

    if (delay>=0) // if replay hasn't ended
        glutTimerFunc(delay, updateR, 0);
    else {
        cout << "End of replay."<<endl;
        glutTimerFunc(delay, update, 0);    // call the update function without replay functionality
    }

}



void mouseMove(int x, int y) {
    replay.recordEvent(x,y,-1);
}




void initGraph() {

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    windowWidth=FIELD_X+field.width*squareSize+BORDER_WIDTH;
    windowHeight=FIELD_Y+field.height*squareSize+BORDER_WIDTH;

    originalWidth=windowWidth;
    originalHeight=windowHeight;

    glutInitWindowSize(windowWidth, windowHeight);
        
    char title[100];

    strcpy(title,"Miny v");
    strcpy(title+6,VERSION);
    strcpy(title+6+strlen(VERSION),". Player: ");
    strcpy(title+16+strlen(VERSION),playerName);

    glutCreateWindow(title);
           
    glEnable(GL_DEPTH_TEST);     
                                          
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyDown);
    glutReshapeFunc(handleResize);
    glutMouseFunc(mouseClick);
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);
    
}

void initGraphR() {

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    windowWidth=FIELD_X+field.width*squareSize+BORDER_WIDTH;
    windowHeight=FIELD_Y+field.height*squareSize+BORDER_WIDTH;

    originalWidth=windowWidth;
    originalHeight=windowHeight;

    glutInitWindowSize(windowWidth, windowHeight);
        
    char title[100];

    strcpy(title,"Miny v");
    strcpy(title+6,VERSION);
    strcpy(title+6+strlen(VERSION),". Player: ");
    strcpy(title+16+strlen(VERSION),playerName);

    glutCreateWindow(title);
           
    glEnable(GL_DEPTH_TEST);     
                                          
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyDown);
    glutReshapeFunc(handleResize);

}

void displayReplay(char replayFileName[100]) {
    if (loadReplay(replayFileName,&replay)) {
        exit(1);
    }
    field.init();
    cout << "Playing replay..." << endl;
    initGraphR();   

    glutTimerFunc(1, updateR, 0);
}

void listScores(int listScoresType, int scoreListLength, int listFlagging, int listFinished, int difficulty) {
    // TODO 'other' setups may produce too high 3BV/s etc and break layout

    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,"scores.dat");


    Score *scores;
    int count=loadScores(fullpath,&scores);
    

    if (count==0) {      // no scores in score file
        if (listScoresType!=4)
            cout<<"No high scores yet."<<endl;
    }
    else {
        if (listScoresType!=4) {    // if not display as csv, then info about filter and sort
            cout << endl << "Displaying scores."<<endl;
            cout << setw(16)<<left<<"Sorted by: ";

            int (*compareFunc)(const void *,const void *)=NULL;
            switch(listScoresType) {
            case 1:
                cout << "time" << endl;
                compareFunc=compareByTime; break;
            case 2:
                cout << "3BV/s" << endl;
                compareFunc=compareBy3BVs; break;
            case 3:
                cout << "IOE" << endl;
                compareFunc=compareByIOE; break;
            }

            cout << setw(16)<<left<<"Flagging: ";
            switch (listFlagging) {
            case 0: cout << "all"<<endl; break;
            case 1: cout << "flagging only"<<endl; break;
            case 2: cout << "non-flagging only"<<endl; break;
            }

            cout << setw(16)<<left<<"Finished: ";
            switch (listFinished) {
            case 0: cout << "all"<<endl; break;
            case 1: cout << "finished only"<<endl; break;
            case 2: cout << "unfinished only"<<endl; break;
            }

            cout << setw(16)<<left<<"Difficulty: ";

            
            bool standardDifficulty=false;

            switch (difficulty) {
            case 0: cout << "beginner, intermediate, expert, beginner classic"<<endl; standardDifficulty=true; break;
            case 1: cout << "beginner only"<<endl; standardDifficulty=true; break;
            case 2: cout << "intermediate only"<<endl; standardDifficulty=true; break;
            case 3: cout << "expert only"<<endl; standardDifficulty=true; break;
            case 4: cout << "beginner classic only"<<endl; standardDifficulty=true; break;
            }

            if (!standardDifficulty) 
                cout << field.width << "x" << field.height << ", " << field.mineCount << " mines" << endl;
            


            cout << setw(16)<<left<<"Square size: ";
            if (squareSize!=0)
                cout <<squareSize<<endl;
            else
                cout << "all"<<endl;

            cout << setw(16)<<left<<"Player name: ";
            if (!strcmp(playerName,""))
                cout<<"all"<<endl;
            else
                cout<<playerName<<endl;

            cout << setw(16)<<left<<"Count: ";
            if (scoreListLength!=0)
                cout <<scoreListLength<<endl;
            else
                cout << "all"<<endl;
        
            cout<<endl;

            qsort(scores,count,sizeof(Score),compareFunc);
        }

        


        Score *filteredScores;


        count=filterScores(scores, count, &filteredScores,listFlagging, listFinished,
            field.width, field.height, field.mineCount, squareSize,playerName);

        

        displayScores(filteredScores,count,scoreListLength,listScoresType==4);

        cout<<endl;
        free(scores);
    }
}

void beginGame(Field* field) {
    if (squareSize==0)
        squareSize=35;

    if (squareSize<3) 
        squareSize=3;
    else if (squareSize>100) 
        squareSize=100;
    
    (*field).checkValues();

    if (strlen(playerName)!=0 && !isValidName(playerName)) {
        cout << "You entered an invalid name. Name can be max. 20 characters long and can only "
        <<endl<<"contain the characters a-z, A-Z, 0-9 and underscore (_)."<<endl;
        exit(1);   
    }

    // set player name to username if not entered with -n and username is a valid name, else set it to "unnamed"

    if (strlen(playerName)==0) {      
        if (isValidName(getenv("USER")))       
            if (strlen(getenv("USER"))>20) {
                strncpy(playerName,getenv("USER"),20);
                playerName[21]='\0';
            }
            else {
                strcpy(playerName,getenv("USER"));
            }
        else {
            strcpy(playerName,"unnamed");
        }
    }

    initGraph();
    (*field).init();

    glutTimerFunc(50, update, 0);
}

void configureSize(int difficulty, Field* field) {
    if ((*field).width!=0 and (*field).height!=0 and (*field).mineCount!=0) {  // if these values were specified on the command line
        difficulty=-1;      // prevent altering them in the switch
    }
    switch(difficulty) {
        case 0:
            (*field).height=0;
            (*field).width=0;
            (*field).mineCount=0;
            break;
        case 1:
            (*field).height=9;
            (*field).width=9;
            (*field).mineCount=10;
            break;
        case 2:
            (*field).height=16;
            (*field).width=16;
            (*field).mineCount=40;
            break;
        case 3:
            (*field).height=16;
            (*field).width=30;
            (*field).mineCount=99;
            break;
        case 4:
            (*field).height=8;
            (*field).width=8;
            (*field).mineCount=10;
            break;
    } 
}

int main(int argc, char** argv) {

    
    srand (time(NULL));

    glutInit(&argc, argv);


    field.height=0;
    field.width=0;
    field.mineCount=0;

    squareSize=0;
    gameState=GAME_INITIALIZED;
    gamePaused=false;

    char replayFileName[100];
    int listScoresType=0; // 0 - none, 1 - time, 2 - 3bv/s, 3 - ioe, 4 - export as csv

    int difficulty=2;   // 0-unspecified 1-beg 2-int 3-exp 4-beg classic
    int listFlagging=0;  // 0-both, 1-flagging, 2-nf
    int listFinished=1; //  0-both, 1-finished, 2-unfinished
    int scoreListLength=MAX_HS;        // how many scores to display

    strcpy(highScoreDir,getenv("HOME"));
    strcat(highScoreDir,"/.miny/");

    // TODO allow specifying different config directory


    playerName[0]='\0';


    strcpy(playerName,"");

    anyRank=false;

    while ((option_char = getopt (argc, argv, "d:s:w:h:m:n:p:t3f:cg:il:a")) != -1) {
        switch (option_char) {  
            case 'd': 
                difficulty=atoi(optarg);
                break;
            case 's': 
                squareSize=atoi(optarg);
                break;
            case 'm': 
                field.mineCount=atoi(optarg);
                break;
            case 'w': 
                field.width=atoi(optarg);
                break;
            case 'h': 
                field.height=atoi(optarg);
                break;
            case 'n':
                if (strlen(optarg)<20)
                    strcpy(playerName,optarg);
                else
                    strncpy(playerName,optarg,20);
                
                break;
            case 'p':
                strcpy(replayFileName,highScoreDir);       
                strcat(replayFileName,optarg);
                playReplay=true;
                break;
            case 'l':
                scoreListLength=atoi(optarg);
                break;
            case '3':
                listScoresType=2;
                break;
            case 't':
                listScoresType=1;
                break;
            case 'i':
                listScoresType=3;
                break;

            case 'f':
                listFlagging=optarg[0]-'0';
                break;
            case 'g':
                listFinished=optarg[0]-'0';
                break;
            case 'c':
                listScoresType=4;

            case 'a':
                anyRank=true;
                break;
            case '?':
                exit(1);

        }
    }

    if (listScoresType!=4) {
        cout<<"Miny v"<<VERSION<<" (c) 2015-2017 spacecamper"<<endl;
        cout << "See README for info and help."<<endl;
    }

    // high score directory

    if (!directoryExists(highScoreDir)) {
        if (system("mkdir ~/.miny")) {
            cerr << "Error creating config directory. Exiting."<<endl;
            exit(1);
        }
    }

    if (playReplay) {
        displayReplay(replayFileName);
    }
    else { 

        configureSize(difficulty, &field);
        
        if (listScoresType!=0) { // list scores
            listScores(listScoresType, scoreListLength, listFlagging, listFinished, difficulty);
        }
        else {
            // play
            beginGame(&field);
        }
    }

    glutMainLoop(); 
    return 0;

}
