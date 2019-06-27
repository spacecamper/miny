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
#include <GL/freeglut_ext.h>
#endif

#include "Replay.h"
#include "Player.h"
#include "Timer.h"
#include "common.h"
#include "scores.h"


#define VERSION "0.5.12+"


// TODO prevent buffer overflows (strcpy and strcat)
// TODO free allocated memory (after using scores from loadScores and filterScores is finished)


using namespace std;

struct Config {
    Player* player;
    int windowWidth;
    int windowHeight;
    int originalWidth;
    int originalHeight;
    int squareSize;
};

int windowWidth, windowHeight, originalWidth, originalHeight;
int squareSize;

int gameState; // -1 - initialized, 0 - playing, 1 - lost, 2 - won
char option_char;
char highScoreDir[100];
bool isFlagging;
bool gamePaused;
bool playReplay;
bool anyRank;
bool boolDrawCursor;

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

// ----------------------- GRAPHICS -------------------- //

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
    // Draws a segment of a number
    float u=2*zoom; 
    
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

void drawFlag(int squareSize, int x, int y) {
    glColor3f(1,0,0);
    glBegin(GL_TRIANGLES);
    glVertex2f(x+.1*squareSize,y+.3*squareSize);
    glVertex2f(x+.6*squareSize,y+.1*squareSize);
    glVertex2f(x+.6*squareSize,y+.5*squareSize);
    glColor3f(0,0,0);
    glVertex2f(x+.6*squareSize,y+.6*squareSize);
    glVertex2f(x+.15*squareSize,y+.9*squareSize);
    glVertex2f(x+.9*squareSize,y+.9*squareSize);

    glEnd();
}

void drawBackground(int fieldWidth, int fieldHeight) {
    // highlight boxes for in game statistics    
    glColor3f(0,0,0);
    drawRect(BORDER_WIDTH,
            BORDER_WIDTH,
            48+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);
    glColor3f(0,0,0);
    drawRect(originalWidth-(BORDER_WIDTH+DISPLAY_BORDER_WIDTH+48),
            BORDER_WIDTH,
            48+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);
    // new game button
    glColor3f(1,1,0);
    drawRect(originalWidth/2-12-DISPLAY_BORDER_WIDTH/2,
            BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);
    // grid lines
    glColor3f(.3,.3,.3);
    glBegin(GL_LINES); 
    for (int i=0;i<fieldHeight+1;i++) {
        glVertex2f(FIELD_X,FIELD_Y+i*squareSize);
        glVertex2f(FIELD_X+fieldWidth*squareSize,FIELD_Y+i*squareSize);
    }
    for (int i=0;i<fieldWidth+1;i++) {
        glVertex2f(FIELD_X+i*squareSize,FIELD_Y);
        glVertex2f(FIELD_X+i*squareSize,FIELD_Y+fieldHeight*squareSize);
    }
    glEnd();
}

void drawMine(int x, int y, int squareSize) {
    glColor3f(0,0,0);
    glBegin(GL_TRIANGLES);
    glVertex2f((FIELD_X+x*squareSize)+.5*squareSize,(FIELD_Y+y*squareSize)+.1*squareSize);
    glVertex2f((FIELD_X+x*squareSize)+.1*squareSize,(FIELD_Y+y*squareSize)+.5*squareSize);
    glVertex2f((FIELD_X+x*squareSize)+.5*squareSize,(FIELD_Y+(y+1)*squareSize-1)-.1*squareSize);

    glVertex2f((FIELD_X+x*squareSize)+.5*squareSize,(FIELD_Y+y*squareSize)+.1*squareSize);
    glVertex2f((FIELD_X+(x+1)*squareSize-1)-.1*squareSize,(FIELD_Y+y*squareSize)+.5*squareSize);
    glVertex2f((FIELD_X+x*squareSize)+.5*squareSize,(FIELD_Y+(y+1)*squareSize-1)-.1*squareSize);

    const float gap=.25*squareSize;

    glBegin(GL_TRIANGLES);
    glVertex2f((FIELD_X+x*squareSize)+gap,(FIELD_Y+y*squareSize)+gap);
    glVertex2f((FIELD_X+(x+1)*squareSize-1)-gap,(FIELD_Y+y*squareSize)+gap);
    glVertex2f((FIELD_X+(x+1)*squareSize-1)-gap,(FIELD_Y+(y+1)*squareSize-1)-gap);

    glVertex2f((FIELD_X+x*squareSize)+gap,(FIELD_Y+y*squareSize)+gap);
    glVertex2f((FIELD_X+x*squareSize)+gap,(FIELD_Y+(y+1)*squareSize-1)-gap);
    glVertex2f((FIELD_X+(x+1)*squareSize-1)-gap,(FIELD_Y+(y+1)*squareSize-1)-gap);

    glEnd();
}

void drawField(Field field, int squareSize){
    for (int x=0;x<field.width;x++) {
        for (int y=0;y<field.height;y++) {
            const int x1=FIELD_X+x*squareSize;
            const int x2=FIELD_X+(x+1)*squareSize;
            const int y1=FIELD_Y+y*squareSize;
            const int y2=FIELD_Y+(y+1)*squareSize;

            const float zoom=1.5*squareSize/25;

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
                if (field.state[x][y]!=0) {
                    // number
                    drawDigit(field.state[x][y],x1+.5*squareSize-3.0*zoom,
                                y1+.5*squareSize-5.0*zoom,zoom);
                }
                //square background

                glColor3f(.5,.5,.5);
                
                drawRect(x1, y1, squareSize-1, squareSize-1);
            }
            if ((field.state[x][y]==9 or field.state[x][y]==11) 
                 and (gameState==GAME_LOST or gameState==GAME_WON)
                 and field.isMine(x,y)
                ) { // unflagged mine after game
                                
                drawMine(x, y, squareSize);
                
            }
            if(field.state[x][y]==11) { // Hit mine
                glColor3f(1,0,0);
                drawRect(x1, y1, squareSize-1, squareSize-1);
            }
            if (field.state[x][y]==10) {  // flag
                // cross out flag where there is no mine
                if (gameState==GAME_LOST and !field.isMine(x,y)) {
                    const short crossGap=.1*squareSize;
                    
                    glColor3f(0,0,0);

                    glBegin(GL_LINES);

                    glVertex2f(x1+crossGap,y1+crossGap);
                    glVertex2f(x2-crossGap,y2-crossGap);

                    glVertex2f(x2-crossGap,y1+crossGap);
                    glVertex2f(x1+crossGap,y2-crossGap);

                    glEnd();
                }
                drawFlag(squareSize, x1, y1);
            }
        }
    }
}

void displayRemainingMines(int rem) {
    glColor3f(1,0,0);

    if (rem>999) rem=999;

    int dxy=BORDER_WIDTH+DISPLAY_BORDER_WIDTH;

    for (int i=0;i<3;i++) {
        const int digit=rem%10;
        rem/=10;
        drawDigit(digit,32-16*i+dxy,dxy,2);
    }
}

void displayElapsedTime(long etime) {
    glColor3f(1,0,0);

    if (etime>999) etime=999;

    const int dx=originalWidth-BORDER_WIDTH-16;
    const int dy=BORDER_WIDTH+DISPLAY_BORDER_WIDTH;

    for (int i=0;i<3;i++) {
        int digit=etime%10;
        etime/=10;
        drawDigit(digit,-16*i+dx,dy,2);
    }
}

void drawCursor(int x, int y) {
    glColor3f(1,1,0);
    glBegin(GL_TRIANGLES);

    glVertex2f(x,y);
    glVertex2f(x,y+20);
    glVertex2f(x+11,y+17);
    glEnd();
}

void drawScene() {
    glClearColor(.7, .7, .7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glOrtho(0.0, windowWidth, windowHeight, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Config* config = (Config*)glutGetWindowData();

    if(!config) { // if the field hasn't yet been configured
        return;
    }

    if (boolDrawCursor) {
        drawCursor(config->player->cursorX, config->player->cursorY);
    }
    
    displayRemainingMines(config->player->field.calculateRemainingMines());

    displayElapsedTime(config->player->field.timer.calculateElapsedTime()/1000);

    if (gamePaused) {    // hide field when game is paused

        glColor3f(.5,.5,.5);
        
        drawRect(FIELD_X + .5, FIELD_Y + .5, config->player->field.width*squareSize - 1, config->player->field.height*squareSize - 1);

    }
    else {
        drawField(config->player->field, squareSize);
    }

    drawBackground(config->player->field.width, config->player->field.height);    

    glutSwapBuffers();
}

// -------------------------- GLUT ----------------------- //

void handleResize(int w, int h) {
    windowWidth=w;
    windowHeight=h;
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void keyDown(unsigned char key, int x, int y) {
    
    Config* config = (Config*)glutGetWindowData();

    config->player->handleInput(-((int)key), 0, 0);
}

void mouseClick(int button, int mState, int x, int y) {
    if (!gamePaused and mState==GLUT_DOWN) {
        Config* config = (Config*)glutGetWindowData();
        config->player->handleInput(button, x, y);
    }
}

void mouseMove(int x, int y) {
    Config* const config = (Config*)glutGetWindowData();
    config->player->handleInput(-1, x, y);
}


void update(int value) {
    glutPostRedisplay();
    
    Player* player = ((Config*)glutGetWindowData())->player;
    
    if(!(player->playStep(false))){
        playReplay=false;
    }
	
    glutTimerFunc(0, update, 0);
}
void initGraph(Config* config) {

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    windowWidth=FIELD_X+config->player->field.width*config->squareSize+BORDER_WIDTH;
    windowHeight=FIELD_Y+config->player->field.height*config->squareSize+BORDER_WIDTH;

    originalWidth=windowWidth;
    originalHeight=windowHeight;

    glutInitWindowSize(windowWidth, windowHeight);
        
    char title[100];

    strcpy(title,"Miny v");
    strcpy(title+6,VERSION);
    strcpy(title+6+strlen(VERSION),". Player: ");
    strcpy(title+16+strlen(VERSION),config->player->field.playerName);

    glutCreateWindow(title);
           
    glEnable(GL_DEPTH_TEST);
                            
    glutSetWindowData((void*)config);
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyDown);
    glutReshapeFunc(handleResize);

    if(!playReplay) {
        glutMouseFunc(mouseClick);
        glutPassiveMotionFunc(mouseMove);
        glutMotionFunc(mouseMove);
    }
}

void displayReplay(char replayFileName[100], Config* config) {
    if (config->player->loadReplay(replayFileName)) {
        exit(1);
    }
    config->player->field.init();
    cout << "Playing replay..." << endl;
    initGraph(config);

    config->player->playStep(true);
    glutTimerFunc(0, update, 0);
}

void listScores(int listScoresType, int scoreListLength, int listFlagging, int listFinished, int difficulty, Config* config) {
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
                cout << config->player->field.width << "x" << config->player->field.height << ", " << config->player->field.mineCount << " mines" << endl;
            


            cout << setw(16)<<left<<"Square size: ";
            if (squareSize!=0)
                cout <<squareSize<<endl;
            else
                cout << "all"<<endl;

            cout << setw(16)<<left<<"Player name: ";
            if (!strcmp(config->player->field.playerName,""))
                cout<<"all"<<endl;
            else
                cout<<config->player->field.playerName<<endl;

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
            config->player->field.width, config->player->field.height, config->player->field.mineCount, squareSize,config->player->field.playerName);

        

        displayScores(filteredScores,count,scoreListLength,listScoresType==4);

        cout<<endl;
        free(scores);
    }
}

void beginGame(Config* config) {
    
    config->player->field.checkValues();    

    initGraph(config);
    config->player->field.init();

    glutTimerFunc(50, update, 0);
}

void configureSize(int difficulty, Field* field) {
    if (field->width!=0 and field->height!=0 and field->mineCount!=0) {  // if these values were specified on the command line
        difficulty=-1;      // prevent altering them in the switch
    }
    switch(difficulty) {
        case 0:
            field->height=0;
            field->width=0;
            field->mineCount=0;
            break;
        case 1:
            field->height=9;
            field->width=9;
            field->mineCount=10;
            break;
        case 2:
            field->height=16;
            field->width=16;
            field->mineCount=40;
            break;
        case 3:
            field->height=16;
            field->width=30;
            field->mineCount=99;
            break;
        case 4:
            field->height=8;
            field->width=8;
            field->mineCount=10;
            break;
    } 
}

int main(int argc, char** argv) {
    srand (time(NULL));

    glutInit(&argc, argv);

    Player player;

    player.field.height=0;
    player.field.width=0;
    player.field.mineCount=0;

    squareSize=0;

    gameState=GAME_INITIALIZED;
    gamePaused=false;
	boolDrawCursor=false;

    char replayFileName[100];
    int listScoresType=0; // 0 - none, 1 - time, 2 - 3bv/s, 3 - ioe, 4 - export as csv

    int difficulty=2;   // 0-unspecified 1-beg 2-int 3-exp 4-beg classic
    int listFlagging=0;  // 0-both, 1-flagging, 2-nf
    int listFinished=1; //  0-both, 1-finished, 2-unfinished
    int scoreListLength=MAX_HS;        // how many scores to display

    strcpy(highScoreDir,getenv("HOME"));
    strcat(highScoreDir,"/.miny/");

    // TODO allow specifying different config directory

    player.field.playerName[0]='\0';

    strcpy(player.field.playerName,"");

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
                player.field.mineCount=atoi(optarg);
                break;
            case 'w': 
                player.field.width=atoi(optarg);
                break;
            case 'h': 
                player.field.height=atoi(optarg);
                break;
            case 'n':
                if (strlen(optarg)<20)
                    strcpy(player.field.playerName,optarg);
                else
                    strncpy(player.field.playerName,optarg,20);
                
                break;
            case 'p':
                strcpy(replayFileName,highScoreDir);       
                strcat(replayFileName,optarg);
                playReplay=true;
				boolDrawCursor=true;
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

    if (strlen(player.field.playerName)!=0 && !isValidName(player.field.playerName)) {
        cout << "You entered an invalid name. Name can be max. 20 characters long and can only "
        <<endl<<"contain the characters a-z, A-Z, 0-9 and underscore (_)."<<endl;
        exit(1);   
    }

    // set player name to username if not entered with -n and username is a valid name, else set it to "unnamed"

    if (strlen(player.field.playerName)==0) {      
        if (isValidName(getenv("USER")))       
            if (strlen(getenv("USER"))>20) {
                strncpy(player.field.playerName,getenv("USER"),20);
                player.field.playerName[21]='\0';
            }
            else {
                strcpy(player.field.playerName,getenv("USER"));
            }
        else {
            strcpy(player.field.playerName,"unnamed");
        }
    }

    if (listScoresType!=4) {
        cout<<"Miny v"<<VERSION<<" (c) 2015-2019 spacecamper"<<endl;
        cout << "See README for info and help."<<endl;
    }

    // high score directory

    if (!directoryExists(highScoreDir)) {
        if (system("mkdir ~/.miny")) {
            cerr << "Error creating config directory. Exiting."<<endl;
            exit(1);
        }
    }

    if (squareSize==0) {
        squareSize=35;
    }

    if (squareSize<3) {
        squareSize=3;
    }
    else if (squareSize>100) {
        squareSize=100;
    }

    Config config;

    config.windowWidth=windowWidth;
    config.windowHeight=windowHeight;
    config.originalWidth=originalWidth;
    config.originalHeight=originalHeight;
    config.squareSize=squareSize;
    config.player=&player;
    
    if (playReplay) {
        displayReplay(replayFileName, &config);
    }
    else { 

        configureSize(difficulty, &(player.field));
        
        if (listScoresType!=0) { // list scores
            listScores(listScoresType, scoreListLength, listFlagging, listFinished, difficulty, &config);
        }
        else {
            // play
            beginGame(&config);
        }
    }

    if(listScoresType==0){
        glutMainLoop();
    }

    return 0;

}
