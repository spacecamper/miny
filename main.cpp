/*
 * Miny
 * a minesweeper clone
 * (c) 2016 spacecamper
*/

#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include <iomanip>
#include <sstream>

#define BORDER_WIDTH 10
#define DISPLAY_BORDER_WIDTH 4
#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define FIELD_X 10
#define FIELD_Y 48

#define GAME_INITIALIZED -1
#define GAME_PLAYING 0
#define GAME_LOST 1
#define GAME_WON 2

#define MAX_HS 20

#define VERSION "0.3.2"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

 
using namespace std;

int windowWidth, windowHeight, originalWidth, originalHeight;
int fieldWidth, fieldHeight, squareSize, mineCount;
bool mine[MAX_WIDTH][MAX_HEIGHT];
int state[MAX_WIDTH][MAX_HEIGHT];  // 0-8 - adjacent mine count for revealed field, 9 - not revealed, 10 - flag
int gameState; // -1 - initialized, 0 - playing, 1 - lost, 2 - won
char option_char;
struct timeval timeStarted, timeFinished;
int hitMineX,hitMineY;  // when game is lost
string playerName;
char highScoreDir[100];
bool isFlagging;
bool gamePaused;
long totalTimePaused;
long pausedSince;

class hiScore {
public:

    string name;
    int time;
    time_t timeStamp;

};



int calculateRemainingMines();


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

void displayHiScores(hiScore *hs, int count,int highlight) {


    unsigned int maxlen=0;

    for (int i=0;i<count;i++)
        if (hs[i].name.length()>maxlen)
            maxlen=hs[i].name.length();

    cout << endl<<"     " << setw(maxlen+1) << left << "Name"<<setw(9)<<right<< "Time"<<setw(10)<<right<< "Date"<<endl<<endl;


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
            << dateString;

        cout << endl;
    }
    cout<<endl;
}





int readHiScoreFile(char *fname,hiScore *scores,int *count) {
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
    case 2:
        *count=0;

        while (!ifile.eof()) {
            ifile >> scores[*count].name >> scores[*count].time >> scores[*count].timeStamp;
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

void writeHiScoreFile(char *fname, hiScore *scores, int count) {


    ofstream ofile;
    
    char fullpath[100];
    strcpy(fullpath,highScoreDir);
    strcat(fullpath,fname);
 //   cout << "Writing high score file " << fullpath <<endl;

    ofile.open (fullpath);


    ofile << "miny-high-score-file-version: 2"<<endl;

    for (int i=0;i<count;i++) {
        ofile << scores[i].name << " " << scores[i].time << " " << scores[i].timeStamp << endl;
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



long calculateTimeSinceStart() {
    long seconds, useconds;    

    struct timeval now;

    gettimeofday(&now, NULL);

    seconds  = now.tv_sec  - timeStarted.tv_sec;
    useconds = now.tv_usec - timeStarted.tv_usec;

    return ((seconds) * 1000 + useconds/1000.0) + 0.5;

}


long calculateTimePaused() {

    if (gameState==GAME_INITIALIZED) {
        return 0;
    }
    else if (gameState==GAME_PLAYING) {
        if (gamePaused) {
            return totalTimePaused+calculateTimeSinceStart()-pausedSince;
        }
        else {
            return totalTimePaused;
        }
    }
    else  { //if (gameState==GAME_LOST or gameState==GAME_WON) 
        return totalTimePaused;
    }


}

long calculateElapsedTime() {

    // calculates time from first click till now (when playing) or till game has ended, minus time when game was paused

    long seconds, useconds;    
    long elapsedTime;

    

    if (gameState==GAME_PLAYING) {


        elapsedTime=calculateTimeSinceStart()-calculateTimePaused();

    }
    else if (gameState==GAME_LOST or gameState==GAME_WON) {

        seconds  = timeFinished.tv_sec  - timeStarted.tv_sec;
        useconds = timeFinished.tv_usec - timeStarted.tv_usec;

        elapsedTime=((seconds) * 1000 + useconds/1000.0) + 0.5-totalTimePaused;

    }
    else
        elapsedTime=0;

    return elapsedTime;

        
}



void stopTimer() {
    gettimeofday(&timeFinished, NULL);
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


void placeMines(int firstClickX, int firstClickY) {

    for (int i=0;i<mineCount;i++) {
        int x=rand()%fieldWidth;
        int y=rand()%fieldHeight;
        if (mine[x][y] or (abs(firstClickX-x)<=0 and abs(firstClickY-y)<=0))
            i--;
        else
            mine[x][y]=true;
    }
    glutPostRedisplay();
  //  cout << "Mines placed." << endl;


}


void initField() {
    

    for (int i=0;i<fieldWidth;i++) {
        for (int j=0;j<fieldWidth;j++) {
            mine[i][j]=false;
            state[i][j]=9;
        }
    }
    gameState=GAME_INITIALIZED;
    glutPostRedisplay();
    isFlagging=false;
    totalTimePaused=0;
    gamePaused=false;

//    cout << "Field initialized." << endl;
}


void unpauseGame() {
    gamePaused=false;
    totalTimePaused+=calculateTimeSinceStart()-pausedSince;
    cout << "Game unpaused."<<endl;// Elapsed time: "<<calculateElapsedTime()<<" ms"<<endl;
}


void keyDown(unsigned char key, int x, int y) {

    switch (key) {
    case ' ':   
        if (!gamePaused) 
            initField(); // restart game

        break;
    case 'p':   // pause
        if (gameState==GAME_PLAYING) {
            if (!gamePaused) {

                gamePaused=true;
                pausedSince=calculateTimeSinceStart();
                cout << "Game paused. Press P to continue. Elapsed time: "<<calculateElapsedTime()<<" ms"<<endl;
            }
            else
                unpauseGame();
            
        }
        break;
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

/*
    // "smiley face" (actually a square)

    int radius=12;

    glColor3f(1,1,0);
    drawRect(originalWidth/2-radius,FIELD_Y/2-radius,radius*2,radius*2);
*/


    // number of remaining mines
    glColor3f(1,0,0);
    int rem=calculateRemainingMines();


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


    long etime=calculateElapsedTime()/1000;


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
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y);
        glEnd();

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y+fieldHeight*squareSize);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+fieldHeight*squareSize);
        glEnd();

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X,FIELD_Y+fieldHeight*squareSize);
        glEnd();

        glBegin(GL_LINES);    
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+fieldHeight*squareSize);
        glEnd();


        glColor3f(.5,.5,.5);

        glBegin(GL_TRIANGLES);
        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+fieldHeight*squareSize);

        glVertex2f(FIELD_X,FIELD_Y);
        glVertex2f(FIELD_X,FIELD_Y+fieldHeight*squareSize);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+fieldHeight*squareSize);


        
        glEnd();

    }
    else {

        // grid lines

        glColor3f(.2,.2,.2);

        

        for (int i=0;i<fieldHeight+1;i++) {
            glBegin(GL_LINES);    
            glVertex2f(FIELD_X,FIELD_Y+i*squareSize);
            glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+i*squareSize);
            glEnd();

        }

        for (int i=0;i<fieldWidth+1;i++) {
            glBegin(GL_LINES);    
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y);
            glVertex2f(FIELD_X+i*squareSize,FIELD_Y+fieldHeight*squareSize);
            glEnd();

        }


        // squares
        

        for (int x=0;x<fieldWidth;x++) 
            for (int y=0;y<fieldHeight;y++) {
                int x1=FIELD_X+x*squareSize;
                int x2=FIELD_X+(x+1)*squareSize;
                int y1=FIELD_Y+y*squareSize;
                int y2=FIELD_Y+(y+1)*squareSize;

                
                if (state[x][y]>=0 and state[x][y]<=8) {    // revealed square
                    
                    switch(state[x][y]) {
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

                    drawDigit(state[x][y],x1+.5*squareSize-3.0*zoom,y1+.5*squareSize-5.0*zoom,zoom);



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
                else if (state[x][y]==9 and (gameState==GAME_LOST or gameState==GAME_WON) and mine[x][y]) { // unflagged mine when game is over
                                        
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
                else if (state[x][y]==10) {  // flag

                    // cross out flag where there is no mine

                    if (gameState==GAME_LOST and !mine[x][y]) {
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



void revealSquare(int, int);
void squareClicked(int, int);


void revealAround(int squareX, int squareY) {

    // reveal squares around a square

    if (squareX>0) {
        if (squareY>0) squareClicked(squareX-1,squareY-1);
        squareClicked(squareX-1,squareY);
        if (squareY<fieldHeight-1) squareClicked(squareX-1,squareY+1);
        
    }

    if (squareY>0) squareClicked(squareX,squareY-1);
    if (squareY<fieldHeight-1) squareClicked(squareX,squareY+1);

    if (squareX<fieldWidth-1) {
        if (squareY>0) squareClicked(squareX+1,squareY-1);
        squareClicked(squareX+1,squareY);
        if (squareY<fieldHeight-1) squareClicked(squareX+1,squareY+1);
    }
    
}

void hiScoreTestAndWrite(char *fname,string name,int time, time_t timeStamp) {
    // test and output a line saying whether player got a high score, if yes write it and display new high score table

    int count=0;
    hiScore hs[MAX_HS];

 //   cout << "High score file: "<<fname<<endl;

    if (readHiScoreFile(fname,hs,&count)) {
  //      cout << "no high scores yet"<<endl;
        hs[0].name=name;
        hs[0].time=time;
        hs[0].timeStamp=timeStamp;
        cout << "YOU GOT A HIGH SCORE."<<endl;
        displayHiScores(hs,1,0);
        writeHiScoreFile(fname,hs,1);
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
        //        cout << "high score inserted"<<endl;
                cout << "YOU GOT A HIGH SCORE."<<endl;
                displayHiScores(hs,count,i);

                writeHiScoreFile(fname,hs,count);
                written=true;
                break;
            }


        }

        if (!written) {
            if (count<MAX_HS) {
                count++;
                hs[count-1].name=name;
                hs[count-1].time=time;
                hs[count-1].timeStamp=timeStamp;
       //         cout << "high score appended"<<endl;
                cout << "YOU GOT A HIGH SCORE."<<endl;
                displayHiScores(hs,count,count-1);

                writeHiScoreFile(fname,hs,count);
            }
            else {
                cout << "You didn't get a high score."<<endl;
               // displayHiScores(hs,count,-1);
            }
        }
    }


}


void revealSquare(int squareX, int squareY) {


    int adjacentMines=0;
    if (squareX>0) {
        if (squareY>0) adjacentMines+=mine[squareX-1][squareY-1]?1:0;
        adjacentMines+=mine[squareX-1][squareY]?1:0;
        if (squareY<fieldHeight) adjacentMines+=mine[squareX-1][squareY+1]?1:0;
        
    }

    if (squareY>0) adjacentMines+=mine[squareX][squareY-1]?1:0;
    if (squareY<fieldHeight-1) adjacentMines+=mine[squareX][squareY+1]?1:0;

    if (squareX<fieldWidth-1) {
        if (squareY>0) adjacentMines+=mine[squareX+1][squareY-1]?1:0;
        adjacentMines+=mine[squareX+1][squareY]?1:0;
        if (squareY<fieldHeight-1) adjacentMines+=mine[squareX+1][squareY+1]?1:0;
    }
    
  //  cout << "There are " << adjacentMines << " mines nearby." << endl;
    
    state[squareX][squareY]=adjacentMines;

    if (adjacentMines==0) {
        revealAround(squareX,squareY);
    }


    // test if game finished
    if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {
        bool notFinished=false;

        for (int i=0;i<fieldWidth;i++) 
            for (int j=0;j<fieldHeight;j++) 
                if (state[i][j]==9 and not mine[i][j])
                    notFinished=true;

        if (!notFinished) {
            gameState=GAME_WON;
            stopTimer();
            cout << "YOU WIN! It took you "<<calculateElapsedTime()<<" milliseconds." << endl;
            
            char fname[100];
            cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;
            sprintf(fname,"%i-%i-%i%s.hiscore",fieldWidth,fieldHeight,mineCount,isFlagging?"":"-nf");

            hiScoreTestAndWrite(fname,playerName,calculateElapsedTime(),time(NULL));

        }    
    }


}

void squareClicked(int squareX, int squareY) {

    // clicked (or asked to reveal) unrevealed square


    if (state[squareX][squareY]==9) {
        if (mine[squareX][squareY]) {
            //timeFinished=time(NULL);
            hitMineX=squareX;
            hitMineY=squareY;
            stopTimer();
            cout << "YOU HIT A MINE. You played for "<<calculateElapsedTime()<<" milliseconds." << endl;
            gameState=GAME_LOST;
        }
        else
            revealSquare(squareX,squareY);
    }
    
}

void checkAndRevealAround(int squareX,int squareY) {
    // count adjacent mines and possibly reveal adjacent squares

    int flaggedAdjacentMines=0;
    if (squareX>0) {
        if (squareY>0) flaggedAdjacentMines+=state[squareX-1][squareY-1]==10?1:0;
        flaggedAdjacentMines+=state[squareX-1][squareY]==10?1:0;
        if (squareY<fieldHeight) flaggedAdjacentMines+=state[squareX-1][squareY+1]==10?1:0;
        
    }

    if (squareY>0) flaggedAdjacentMines+=state[squareX][squareY-1]==10?1:0;
    if (squareY<fieldHeight) flaggedAdjacentMines+=state[squareX][squareY+1]==10?1:0;

    if (squareX<fieldWidth) {
        if (squareY>0) flaggedAdjacentMines+=state[squareX+1][squareY-1]==10?1:0;
        flaggedAdjacentMines+=state[squareX+1][squareY]==10?1:0;
        if (squareY<fieldHeight) flaggedAdjacentMines+=state[squareX+1][squareY+1]==10?1:0;
    }
    if (flaggedAdjacentMines==state[squareX][squareY]) {
        revealAround(squareX,squareY);
    }
}

int calculateRemainingMines() {

    int remaining=mineCount;
    
    for (int i=0;i<fieldWidth;i++) 
        for (int j=0;j<fieldWidth;j++) 
            if (state[i][j]==10)
                remaining--;

    return remaining;
}

void mouseClick(int button, int mState, int x, int y) {

 
    if (!gamePaused) {
        if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {

            if (x>FIELD_X and x<FIELD_X+fieldWidth*squareSize and y>FIELD_Y and y<FIELD_Y+fieldHeight*squareSize) {
                
                if (mState==GLUT_DOWN) {
                    int squareX=(x-FIELD_X)/squareSize;
                    int squareY=(y-FIELD_Y)/squareSize;

                  //  cout << "mouse button at [" << squareX << ", " << squareY << "], state " << state[squareX][squareY] << endl;
                
                    if (button==GLUT_LEFT_BUTTON) {
                        if (state[squareX][squareY]==9) {
                            if (gameState==GAME_INITIALIZED) {
                                placeMines(squareX,squareY);

                                // start timer
                                gettimeofday(&timeStarted, NULL);
                
                           //     cout << "Timer started." << endl;
                                gameState=GAME_PLAYING;
                            }
                            squareClicked(squareX,squareY);
                        }
                        else if (state[squareX][squareY]<=8)
                            checkAndRevealAround(squareX,squareY);
                    }
                    else if (button==GLUT_RIGHT_BUTTON) {
                        // toggle flag or check and reveal surrounding squares
                        if (state[squareX][squareY]==9) {
                            state[squareX][squareY]=10;
                            if (!isFlagging)
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
                        checkAndRevealAround(squareX,squareY);
                    }
                }
            }

            glutPostRedisplay();
        }
        else if (!(x>FIELD_X and x<FIELD_X+fieldWidth*squareSize and y>FIELD_Y and y<FIELD_Y+fieldHeight*squareSize)) {
            initField();
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


    glutPostRedisplay(); 
	
    glutTimerFunc(50, update, 0);
}




void initGraph() {

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    windowWidth=FIELD_X+fieldWidth*squareSize+BORDER_WIDTH;
    windowHeight=FIELD_Y+fieldHeight*squareSize+BORDER_WIDTH;

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
    
    
}




int main(int argc, char** argv) {

    
    srand (time(NULL));

    glutInit(&argc, argv);


    fieldHeight=16;
    fieldWidth=16;
    mineCount=40;

    squareSize=25;
    gameState=GAME_INITIALIZED;
    gamePaused=false;


    cout<<"Miny v"<<VERSION<<" (c) 2016 spacecamper"<<endl;
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

    while ((option_char = getopt (argc, argv, "d:s:w:h:m:n:l")) != -1)
        switch (option_char) {  
            case 'd': {
                int level=atoi(optarg);
                switch(level) {
                    case 1:
            //            cout << "Level: Beginner" << endl;
                        fieldHeight=9;
                        fieldWidth=9;
                        mineCount=10;
            //            levelSelected=true;
                        break;
                    case 2:
            //            cout << "Level: Intermediate" << endl;
                        fieldHeight=16;
                        fieldWidth=16;
                        mineCount=40;
              //          levelSelected=true;
                        break;
                    case 3:
              //          cout << "Level: Expert" << endl;
                        fieldHeight=16;
                        fieldWidth=30;
                        mineCount=99;
                //        levelSelected=true;
                        break;
                }
                break;
            }
            case 's': 
                squareSize=atoi(optarg);
                break;
            case 'm': 
                mineCount=atoi(optarg);
                break;
            case 'w': 
                fieldWidth=atoi(optarg);
                break;
            case 'h': 
                fieldHeight=atoi(optarg);
                break;
            case 'n':
                playerName=optarg;
                
                break;
            case 'l':
                onlyListHiScores=true;
                
                break;
          //  case '?': cout<<"usage: "<<argv[0]<<" [d 1-3]\n";
        }


/*
    if (!levelSelected) {
        cout << "You can select level with the -d option. (-d1, -d2 or -d3)"<<endl;

        
    }
*/

    
    if (squareSize<3) 
        squareSize=3;
    else if (squareSize>100) 
        squareSize=100;

    if (fieldHeight<2) 
        fieldHeight=2;
    else if (fieldHeight>MAX_HEIGHT) 
        fieldHeight=MAX_HEIGHT;
    

    if (fieldWidth<2) 
        fieldWidth=2;
    else if (fieldWidth>MAX_WIDTH) 
        fieldWidth=MAX_WIDTH;
    
    if (mineCount>=fieldHeight*fieldWidth)
        mineCount=fieldHeight*fieldWidth-1;
    else if (mineCount<1)
        mineCount=1;



    cout<<"Current high scores for WIDTH: "<<fieldWidth<<" HEIGHT: "<<fieldHeight<<" MINES: "<<mineCount<<endl;

    cout << "Flagging:"<<endl;

    hiScore hs[MAX_HS];
    char fname[100];
    int count;
    sprintf(fname,"%i-%i-%i.hiscore",fieldWidth,fieldHeight,mineCount);
    readHiScoreFile(fname,hs,&count);
    

    if (count==0) {
        cout<<"No high scores yet."<<endl;
    }
    else {
        displayHiScores(hs,count,-1);
    }

    cout << "Non-flagging:"<<endl;
    sprintf(fname,"%i-%i-%i-nf.hiscore",fieldWidth,fieldHeight,mineCount);
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
        cout << "No name entered with the -n option. "<<endl<<"Enter your name: "; //In future you can enter your name with the -n option."<<endl<<"
        cin >> playerName;
    }    

    if (playerName.length()>20 or playerName.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_") != std::string::npos) {
        cout << "You entered an invalid name. Name can be max. 20 characters long and can only "<<endl<<"contain the characters a-z, A-Z, 0-9 and underscore (_)."<<endl;
        exit(1);
    }


    if (playerName=="") {   
      /*  cout << "Couldn't get player name. Using username as player name." << endl; // this should set player name when prog not run from terminal but it doesn't work 
                                                                                      // - uncomment the getlogin line and program exits when not run from terminal
        playerName=getlogin();*/
        cout << "Couldn't get player name. Using 'unnamed'." << endl;
        playerName="unnamed";
    }

    cout << "Your name: "<<playerName<<endl;



    initGraph();

    initField();

    glutTimerFunc(50, update, 0);

    

    glutMainLoop();    
    return 0;

}
