/*
 * Miny
 * a minesweeper clone
 * (c) 2015-2016 spacecamper
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


#define MAX_HS 20

#define VERSION "0.4.1"

// TODO elapsed time isn't being redrawn while playing replay when there's a long pause between two 
//   events
// TODO set window class to use with some window managers


using namespace std;

int windowWidth, windowHeight, originalWidth, originalHeight;
int squareSize;

int gameState; // -1 - initialized, 0 - playing, 1 - lost, 2 - won
char option_char;

int hitMineX,hitMineY;  // when game is lost
string playerName;
char highScoreDir[100];
bool isFlagging;
bool gamePaused;

bool playReplay;


Timer timer;


class HiScore {
public:

    string name;
    int time;
    time_t timeStamp;
    char replayFile[100];
};


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


void displayHiScores(HiScore *hs, int count,int highlight) {

    unsigned int maxlen=0;

    for (int i=0;i<count;i++)
        if (hs[i].name.length()>maxlen)
            maxlen=hs[i].name.length();

    cout << endl<<"     " << setw(maxlen+1) << left << "Name"<<setw(9)<<right<< "Time"
        <<setw(10)<<right<< "Date"<<setw(25)<<right<< "Replay"<<endl<<endl;


    for (int i=0;i<count;i++) {
        string dateString;
        if (hs[i].timeStamp==0) {
            dateString="unknown";
        
        }
        else {
            struct tm *lt;
            lt=localtime(&hs[i].timeStamp);
            ostringstream stringStream;
            stringStream <<setw(2)<< setfill('0')<<lt->tm_mday
                <<'-'
                <<setw(2)<< setfill('0')<<lt->tm_mon+1
                <<'-'
                <<setw(4)<< setfill(' ')<<right<< lt->tm_year+1900
                <<' '         
                <<setw(2)<< setfill('0')<<lt->tm_hour
                <<':'
                <<setw(2)<< setfill('0')<<lt->tm_min
                <<':'
                <<setw(2)<< setfill('0')<<lt->tm_sec;

            dateString = stringStream.str();
        }

        
        cout << " " << setw(2) << setfill(' ') << right << i+1 
            << "  " << setw(maxlen+1) << left << hs[i].name
            << setw(9) << right << hs[i].time
            << (i==highlight?'*':' ') << "     "            
            << setw(19) << left << dateString
            << "    "<< (strcmp(hs[i].replayFile,"*")?hs[i].replayFile:"N/A");

        cout << endl;
    }
    cout<<endl;
}



void saveReplay(char *fname, Replay *r) {


    ofstream ofile;
    
    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
    cout << "Writing replay file " << fullpath <<endl;

    ofile.open (fullpath);

    r->writeToFile(&ofile);

    ofile.close();
    

}

int loadReplay(char *fname, Replay *r) {
    ifstream ifile;

    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
 //   cout << "Reading replay file " << fullpath <<endl;

    ifile.open (fullpath);
    if (!ifile.is_open()) {
        cerr<<"Error opening file."<<endl;
        return 1;
    }

    r->readFromFile(&ifile);


    ifile.close();

    return 0;

}



int readHiScoreFile(char *fname,HiScore *scores,int *count) {
    ifstream ifile;

    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
 //   cout << "Reading high score file " << fullpath <<endl;

    ifile.open (fullpath);
    if (!ifile.is_open()) {
//        cerr<<"Error opening file."<<endl;
        *count=0;   
        return 1;
    }

    
    string firstString;

    ifile >> firstString;


    int fileVersion;

    if (firstString=="miny-high-score-file-version:") {
        ifile >> fileVersion;
    }
    else {
        // TODO maybe implement a better check for whether the file is really of the 1st version
        fileVersion=1;

        ifile.clear();
        ifile.seekg(0, ios::beg);
    }

    switch(fileVersion) {
    case 3:
        *count=0;

        while (!ifile.eof()) {
            ifile >> scores[*count].name >> scores[*count].time >> scores[*count].timeStamp 
                >> scores[*count].replayFile;
            (*count)++;
            if ((*count)==MAX_HS) break;
        }

        if (ifile.eof())
            (*count)--;
        break;
        
    case 2:
        cout << "This high score file was created by an old version of the game and will become"<<endl<<"unreadable to that version after being written to."<<endl;
        *count=0;

        while (!ifile.eof()) {
            ifile >> scores[*count].name >> scores[*count].time >> scores[*count].timeStamp;
            strcpy(scores[*count].replayFile,"*");
            (*count)++;
            if ((*count)==MAX_HS) break;
        }

        if (ifile.eof())
            (*count)--;
        break;
    case 1:
        cout << "This high score file was created by an old version of the game and will become"<<endl<<"unreadable to that version after being written to."<<endl;

        *count=0;

        while (!ifile.eof()) {
            ifile >> scores[*count].name >> scores[*count].time;
            scores[*count].timeStamp=0;
            strcpy(scores[*count].replayFile,"*");
            (*count)++;
            if ((*count)==MAX_HS) break;
        }

        if (ifile.eof())
            (*count)--;
        break;
    default:
        cout << "Unknown high score file version. You are probably running an old version of the"<<endl<<"game. Please upgrade to the latest version." << endl;
        exit(1);
    }


    ifile.close();

    return 0;

}

void writeHiScoreFile(char *fname, HiScore *scores, int count) {


    ofstream ofile;
    
    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
 //   cout << "Writing high score file " << fullpath <<endl;

    ofile.open (fullpath);


    ofile << "miny-high-score-file-version: 3"<<endl;

    for (int i=0;i<count;i++) {
        ofile << scores[i].name << " " << scores[i].time << " " << scores[i].timeStamp << " "
             << scores[i].replayFile << endl;
    }

    ofile.close();
    

}



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







void unpauseGame() {
    gamePaused=false;
    timer.unpause();
    replay.resumeRecording();
    cout << "Game unpaused."<<endl;// Elapsed time: "<<calculateElapsedTime()<<" ms"<<endl;
}


void keyDown(unsigned char key, int x, int y) {

    switch (key) {
    case ' ':   
        if (!gamePaused and !playReplay) 
            field.init(); // restart game

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
        cout << field.width << "x" << field.height<<endl;
        break;
    case 'q':
    case 27:
        exit(0);
    

    }


}



void drawScene() {


    glClearColor(.7, .7, .7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_CULL_FACE);

    glClear(GL_DEPTH_BUFFER_BIT);


    if (playReplay) {
        // draw cursor
        glColor3f(1,1,0);
        glBegin(GL_TRIANGLES);

        glVertex2f(replay.cursorX,replay.cursorY);
        glVertex2f(replay.cursorX,replay.cursorY+20);
        glVertex2f(replay.cursorX+11,replay.cursorY+17);
        glEnd();
        



    }


/*
    // "smiley face" (actually a square)

    int radius=12;

    glColor3f(1,1,0);
    drawRect(originalWidth/2-radius,FIELD_Y/2-radius,radius*2,radius*2);
*/


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

    
    

    if (gamePaused) {    // hide field when game is paused

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y);
        glEnd();

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y+field.height*squareSize);
        glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+field.height*squareSize);
        glEnd();

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X,FIELD_Y+field.height*squareSize);
        glEnd();

        glBegin(GL_LINES);    
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

        glColor3f(.2,.2,.2);

        

        for (int i=0;i<field.height+1;i++) {
            glBegin(GL_LINES);    
            glVertex2f(FIELD_X,FIELD_Y+i*squareSize);
            glVertex2f(FIELD_X+field.width*squareSize,FIELD_Y+i*squareSize);
            glEnd();

        }

        for (int i=0;i<field.width+1;i++) {
            glBegin(GL_LINES);    
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y);
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y+field.height*squareSize);
            glEnd();

        }


        // squares
        

        for (int x=0;x<field.width;x++) 
            for (int y=0;y<field.height;y++) {
                int x1=FIELD_X+x*squareSize;
                int x2=FIELD_X+(x+1)*squareSize;
                int y1=FIELD_Y+y*squareSize;
                int y2=FIELD_Y+(y+1)*squareSize;

                
                if (field.state[x][y]>=0 and field.state[x][y]<=8) {    // revealed square
                    
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


                    // number
                    float zoom=1.5*squareSize/25;

                    drawDigit(field.state[x][y],x1+.5*squareSize-3.0*zoom,
                                y1+.5*squareSize-5.0*zoom,zoom);



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
                else if (field.state[x][y]==9 and (gameState==GAME_LOST or gameState==GAME_WON)
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



    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();

}



bool hiScoreTestAndWrite(char *fname,string name,int time, time_t timeStamp, char *replayFile) {
    // test and output a line saying whether player got a high score, if yes write it and 
    //  display new high score table

    int count=0;
    HiScore hs[MAX_HS];

 //   cout << "High score file: "<<fname<<endl;

    // TODO set replay file name for new high score


    if (readHiScoreFile(fname,hs,&count)) {
  //      cout << "no high scores yet"<<endl;
        hs[0].name=name;
        hs[0].time=time;
        hs[0].timeStamp=timeStamp;
        strcpy(hs[0].replayFile,replayFile);
        cout << "YOU GOT A HIGH SCORE."<<endl;
        displayHiScores(hs,1,0);
        writeHiScoreFile(fname,hs,1);
        return true;
    }
    else {

        bool written=false;

        for (int i=0;i<count;i++) {
            if (time<hs[i].time) {
                count++;
                if (count>MAX_HS) count=MAX_HS;

                for (int j=count-1;j>i;j--) 
                    hs[j]=hs[j-1];

                
              /*  cout << "hs after shift: "<<endl;
                displayHiScores(hs,count);
*/
                hs[i].name=name;
                hs[i].time=time;
                hs[i].timeStamp=timeStamp;
                strcpy(hs[i].replayFile,replayFile);
        //        cout << "high score inserted"<<endl;
                cout << "YOU GOT A HIGH SCORE."<<endl;
                displayHiScores(hs,count,i);

                writeHiScoreFile(fname,hs,count);
                written=true;
                return true;
            }


        }

        if (!written) {
            if (count<MAX_HS) {
                count++;
                hs[count-1].name=name;
                hs[count-1].time=time;
                hs[count-1].timeStamp=timeStamp;
                strcpy(hs[count-1].replayFile,replayFile);
       //         cout << "high score appended"<<endl;
                cout << "YOU GOT A HIGH SCORE."<<endl;
                displayHiScores(hs,count,count-1);

                writeHiScoreFile(fname,hs,count);
                return true;
            }
            else {
                cout << "You didn't get a high score."<<endl;
                return false;
               // displayHiScores(hs,count,-1);
            }
        }
    }
    return false;

}



void mouseClick(int button, int mState, int x, int y) {


    if (!gamePaused) {
        if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {

            if (x>FIELD_X and x<FIELD_X+field.width*squareSize 
                and y>FIELD_Y and y<FIELD_Y+field.height*squareSize) {
                
                if (mState==GLUT_DOWN) {
                    
                  //  cout << "mouse button at [" << squareX << ", " << squareY << "], state "
                  // << state[squareX][squareY] << endl;
                
                    field.click(x,y,button);
                }
            }

            glutPostRedisplay();
        }
        else if (!(x>FIELD_X and x<FIELD_X+field.width*squareSize 
            and y>FIELD_Y and y<FIELD_Y+field.height*squareSize)) {
            field.init();
        //    placeMines();
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

   // cout << timer.calculateTimeSinceStart() << endl;

    glutPostRedisplay(); 
	
    glutTimerFunc(50, update, 0);
}


void updateR(int value) {

    
     
	
    int delay=replay.playStep();

   // cout << "delay=="<<delay<<endl;
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
    sprintf(title+16+strlen(VERSION), "%s", playerName.c_str());

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
    sprintf(title+16+strlen(VERSION), "%s", playerName.c_str());

    glutCreateWindow(title);
           
    glEnable(GL_DEPTH_TEST);     
                                          
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyDown);
    glutReshapeFunc(handleResize);
 //   glutMouseFunc(mouseClick);
  //  glutPassiveMotionFunc(mouseMove);

}



int main(int argc, char** argv) {

    
    srand (time(NULL));

    glutInit(&argc, argv);


    field.height=16;
    field.width=16;
    field.mineCount=40;

    squareSize=25;
    gameState=GAME_INITIALIZED;
    gamePaused=false;

    char replayFileName[100];
    int playReplayPlace=-1;
    bool playReplayNf=false;


    cout<<"Miny v"<<VERSION<<" (c) 2015-2016 spacecamper"<<endl;
    cout << "See README for info and help."<<endl;

    // high score directory

    strcpy(highScoreDir,getenv("HOME"));
    strcat(highScoreDir,"/.miny/");

    //cout << "hsdir: " << highScoreDir<<endl;

    if (!directoryExists(highScoreDir)) {
        if (system("mkdir ~/.miny")) {
            cerr << "Error creating config directory. Exiting."<<endl;
            exit(1);
        }
    /*    else {
            cout << "Config directory created."<<endl;
        }*/

    }
 /*   else {
        cout<<"Config directory exists."<<endl;
    }*/



    playerName="";

  //  bool levelSelected=false;

    bool onlyListHiScores=false;

    while ((option_char = getopt (argc, argv, "d:s:w:h:m:n:lp:r:f")) != -1)
        switch (option_char) {  
            case 'd': {
                int level=atoi(optarg);
                switch(level) {
                    case 1:
            //            cout << "Level: Beginner" << endl;
                        field.height=9;
                        field.width=9;
                        field.mineCount=10;
            //            levelSelected=true;
                        break;
                    case 2:
            //            cout << "Level: Intermediate" << endl;
                        field.height=16;
                        field.width=16;
                        field.mineCount=40;
              //          levelSelected=true;
                        break;
                    case 3:
              //          cout << "Level: Expert" << endl;
                        field.height=16;
                        field.width=30;
                        field.mineCount=99;
                //        levelSelected=true;
                        break;
                }
                break;
            }
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
                playerName=optarg;
                
                break;
            case 'l':
                onlyListHiScores=true;
                
                break;
            case 'p':
                strcpy(replayFileName,optarg);
                playReplay=true;
                break;
            case 'r':
                playReplay=true;
                playReplayPlace=atoi(optarg)-1;
                break;
            case 'f':
                playReplayNf=true;
                break;
            case '?':
                //cerr << "Unknown option: -" << optopt << endl;
                exit(1);

        }




    if (playReplay) {
        if (playReplayPlace!=-1)  {     // select from high score table
            cout << "Playing replay from high score table."<<endl;
            HiScore hs[MAX_HS];
            char fname[100];
            int count;
            sprintf(fname,"%i-%i-%i%s.hiscore",field.width,field.height,field.mineCount,
                    playReplayNf?"-nf":"");
            readHiScoreFile(fname,hs,&count);
            cout << "High scores loaded."<<endl;
            displayHiScores(hs,count,-1);
            
            if (playReplayPlace<0 or playReplayPlace>=count) {
                cout << "Invalid place number."<<endl;
                exit(1);
            }

            if (strcmp(hs[playReplayPlace].replayFile,"*")==0) {
                cout << "The selected place doesn't have a replay file assigned."<<endl;
                exit(1);
            }
 
            cout << "Load replay " << hs[playReplayPlace].replayFile<<endl;
            strcpy(replayFileName,hs[playReplayPlace].replayFile);

        }

        
        if (loadReplay(replayFileName,&replay)) {
            exit(1);
        }
        field.init();
        cout << "Playing replay..." << endl;
        initGraphR();
        

        glutTimerFunc(1, updateR, 0);
    }
    else {

        if (squareSize<3) 
            squareSize=3;
        else if (squareSize>100) 
            squareSize=100;
        
        field.checkValues();



        cout<<"Current high scores for WIDTH: "<<field.width<<" HEIGHT: "<<field.height
            <<" MINES: "<<field.mineCount<<endl;

        cout << "Flagging:"<<endl;

        HiScore hs[MAX_HS];
        char fname[100];
        int count;
        sprintf(fname,"%i-%i-%i.hiscore",field.width,field.height,field.mineCount);
        readHiScoreFile(fname,hs,&count);
        

        if (count==0) {
            cout<<"No high scores yet."<<endl;
        }
        else {
            displayHiScores(hs,count,-1);
        }

        cout << "Non-flagging:"<<endl;
        sprintf(fname,"%i-%i-%i-nf.hiscore",field.width,field.height,field.mineCount);
        readHiScoreFile(fname,hs,&count);
        

        if (count==0) {
            cout<<"No high scores yet."<<endl;
        }
        else {
            displayHiScores(hs,count,-1);
        }


       // cout << "argv[0]=="<< argv[1]<<endl;
        
        if (onlyListHiScores)
            exit(0);
        

        if (playerName==""){
            cout << "No name entered with the -n option. "<<endl<<"Enter your name: "; 
            //In future you can enter your name with the -n option."<<endl<<"
            cin >> playerName;
        }    

        if (playerName.length()>20 or playerName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_") != std::string::npos
            ) {
            cout << "You entered an invalid name. Name can be max. 20 characters long and can only "
                <<endl<<"contain the characters a-z, A-Z, 0-9 and underscore (_)."<<endl;
            exit(1);
        }


        if (playerName=="") {   
          /*  cout << "Couldn't get player name. Using username as player name." << endl; 
            // this should set player name when prog not run from terminal but it doesn't work 
             // - uncomment the getlogin line and program exits when not run from terminal
            playerName=getlogin();*/
            cout << "Couldn't get player name. Using 'unnamed'." << endl;
            playerName="unnamed";
        }

        cout << "Your name: "<<playerName<<endl;

        initGraph();
        field.init();

        glutTimerFunc(50, update, 0);
    }

    glutMainLoop();    
    return 0;

}
