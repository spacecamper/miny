/*
 * Miny
 * a minesweeper clone
 * (c) 2015-2019, 2021, 2023 spacecamper
 */

#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <list>
#include <vector>
#include <iomanip>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#include "Config.h"
#include "Replay.h"
#include "Player.h"
#include "common.h"

#include "Timer.h"

#include "scores.h"


#define VERSION "0.6.0"

using namespace std;

Config conf;

void redisplay() {
    glutPostRedisplay();
}


bool fileExists(string& path) {
    return access(path.c_str(), F_OK) == 0;
}
bool directoryExists(const string& str)
{
    const char* pzPath = str.c_str();
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
    glRectf(x, y, x+w, y+h);
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
    drawRect(conf.targetWidth()-(BORDER_WIDTH+DISPLAY_BORDER_WIDTH+48),
            BORDER_WIDTH,
            48+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);
    // new game button
    glColor3f(1,1,0);
    drawRect(conf.targetWidth()/2-12-DISPLAY_BORDER_WIDTH/2,
            BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH,
            24+DISPLAY_BORDER_WIDTH);

    // grid lines
    glColor3f(.3,.3,.3);
    glBegin(GL_LINES); 
    for (int i=0;i<fieldHeight+1;i++) {
        glVertex2f(FIELD_X+0.5,FIELD_Y+i*conf.squareSize+0.5);
        glVertex2f(FIELD_X+fieldWidth*conf.squareSize+0.5,FIELD_Y+i*conf.squareSize+0.5);
    }
    for (int i=0;i<fieldWidth+1;i++) {
        glVertex2f(FIELD_X+i*conf.squareSize+0.5,FIELD_Y+0.5);
        glVertex2f(FIELD_X+i*conf.squareSize+0.5,FIELD_Y+fieldHeight*conf.squareSize+0.5);
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

/*
            // pretty square borders
            if (field.state[x][y]>=0 and field.state[x][y]<=8) {    // revealed square
                glColor3f(.5,.5,.5);
                glBegin(GL_LINES); 

                // top
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+y*squareSize);
                
                // left
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+(y+1)*squareSize-1);


                // bottom
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+(y+1)*squareSize-1);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+(y+1)*squareSize-1);
                
                // right
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+(y+1)*squareSize-1);
                

                
                glEnd();

            }
            else if (field.state[x][y]==9 or field.state[x][y]==10) {    // unrevealed or flag
                
                glColor3f(.85,.85,.85);
                glBegin(GL_LINES); 

                // top
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+y*squareSize);

                glVertex2f(FIELD_X+x*squareSize+1,FIELD_Y+y*squareSize+1);
                glVertex2f(FIELD_X+(x+1)*squareSize-2,FIELD_Y+y*squareSize+1);
                
                // left
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+(y+1)*squareSize-1);

                glVertex2f(FIELD_X+x*squareSize+1,FIELD_Y+y*squareSize+1);
                glVertex2f(FIELD_X+x*squareSize+1,FIELD_Y+(y+1)*squareSize-2);

                glEnd();

                glColor3f(.55,.55,.55);
                glBegin(GL_LINES); 

                // bottom
                glVertex2f(FIELD_X+x*squareSize,FIELD_Y+(y+1)*squareSize-1);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+(y+1)*squareSize-1);

                glVertex2f(FIELD_X+x*squareSize+1,FIELD_Y+(y+1)*squareSize-2);
                glVertex2f(FIELD_X+(x+1)*squareSize-2,FIELD_Y+(y+1)*squareSize-2);
                
                // right
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+y*squareSize);
                glVertex2f(FIELD_X+(x+1)*squareSize-1,FIELD_Y+(y+1)*squareSize-1);
                
                glVertex2f(FIELD_X+(x+1)*squareSize-2,FIELD_Y+y*squareSize+1);
                glVertex2f(FIELD_X+(x+1)*squareSize-2,FIELD_Y+(y+1)*squareSize-2);
                
                
                glEnd();
            }
*/


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
                
                glRectf(x1 + 1, y1 + 1, x2, y2);
            }
            if ((field.state[x][y]==9 or field.state[x][y]==11) 
                 and (conf.gameState==Config::GAME_LOST or conf.gameState==Config::GAME_WON)
                 and field.isMine(x,y)
                ) { // unflagged mine after game
                                
                drawMine(x, y, squareSize);
                
            }
            if(field.state[x][y]==11) { // Hit mine
                glColor3f(1,0,0);
                glRectf(x1 + 1, y1 + 1, x2, y2);
            }
            if (field.state[x][y]==10) {  // flag
                // cross out flag where there is no mine
                if (conf.gameState==Config::GAME_LOST and !field.isMine(x,y)) {
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

    const int dx=conf.targetWidth()-BORDER_WIDTH-16;
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

    glOrtho(0.0, conf.windowWidth, conf.windowHeight, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (conf.boolDrawCursor) {
        drawCursor(conf.player.cursorX, conf.player.cursorY);
    }
    
    displayRemainingMines(conf.player.field.calculateRemainingMines());

    displayElapsedTime(conf.player.field.timer.calculateElapsedTime()/1000);

    if (conf.gamePaused) {    // hide field when game is paused

        glColor3f(.5,.5,.5);
        
        drawRect(FIELD_X + .5, FIELD_Y + .5, conf.player.field.width*conf.squareSize - 1, conf.player.field.height*conf.squareSize - 1);

    }
    else {
        drawField(conf.player.field, conf.squareSize);
    }

    drawBackground(conf.player.field.width, conf.player.field.height);

    glutSwapBuffers();
}

// -------------------------- GLUT ----------------------- //

void handleResize(int w, int h) {
    conf.windowWidth=w;
    conf.windowHeight=h;
    glViewport(0, 0, conf.windowWidth, conf.windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void keyDown(unsigned char key, int x, int y) {
    conf.handleInput(key);
    conf.player.handleInput(-((int)key), 0, 0);
}

void mouseClick(int button, int mState, int x, int y) {
    if (!conf.gamePaused and mState==GLUT_DOWN) {
        conf.player.handleInput(button, x, y);
    }
}

void mouseMove(int x, int y) {
    conf.player.handleInput(-1, x, y);
}


void update(int value) {
    glutPostRedisplay();
    
    if(!(conf.player.playStep(false))){
        conf.playReplay=false;
    }

    // On my computer the first argument here being zero causes the game to register two events close to each other about every 16 ms instead of one event about every 8 ms. This might worsen the experience especially on monitors with higher refresh rate.
    glutTimerFunc(0, update, 0); 
}


void initGraph() {

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    conf.windowWidth=conf.targetWidth();
    conf.windowHeight=conf.targetHeight();

    glutInitWindowSize(conf.windowWidth, conf.windowHeight);
        
    string title = "Miny v" VERSION ". Player: ";
    title += conf.player.field.playerName;

    glutCreateWindow(title.c_str());
           
    glEnable(GL_DEPTH_TEST);
                            
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(keyDown);
    glutReshapeFunc(handleResize);

    if(!conf.playReplay) {
        glutMouseFunc(mouseClick);
        glutPassiveMotionFunc(mouseMove);
        glutMotionFunc(mouseMove);
    }
}

void displayReplay() {
    if (conf.player.loadReplay(conf.replayFileName)) {
        exit(1);
    }

    conf.player.field.init();
    cout << "Playing replay..." << endl;
    initGraph();

    conf.player.playStep(true);
    glutTimerFunc(0, update, 0);
}


void beginGame() {
    conf.player.field.checkValues();

    initGraph();
    conf.player.field.init();

    glutTimerFunc(50, update, 0);
}

bool findArgFile(string& file) {
    // Try $XDG_CONFIG_HOME/miny
    char* confdir = getenv("XDG_CONFIG_HOME");
    if (confdir) {
        file = confdir;
        file += "/miny/default.args";
        if (fileExists(file)) return true;
    }

    // Try ~/.config/miny
    file = getenv("HOME");
    file += "/.config/miny/default.args";
    if (fileExists(file)) return true;

    // Try ~/.miny
    file = getenv("HOME");
    file += "/.miny/default.args";
    if (fileExists(file)) return true;

    return false;
}

void handleArgs(int argc, char* const argv[]) {
    const static option long_opts[2] = { {"help", 0, NULL, 'H'}, {} };

    optind = 1;
    char option_char;
    while ((option_char = getopt_long(argc, argv, "Hd:s:w:h:m:n:p:t3f:cg:il:C:o", long_opts, NULL)) != -1) {
        conf.handleOption(option_char, optarg);
        if (option_char == 'H')
           exit(0);
        else if(option_char == '?')
           exit(1);
    }
    
}

int main(int argc, char** argv) {
    srand (time(NULL));

    glutInit(&argc, argv);

    string home = getenv("HOME");
    char *xdgDataHome = getenv("XDG_DATA_HOME");
    if (xdgDataHome) {
        conf.cacheDirectory = xdgDataHome;
    } else {
        conf.cacheDirectory = home + "/.local/share";
    }
    bool configExists = directoryExists(conf.cacheDirectory);
    conf.cacheDirectory += "/miny/";
    // default to the old file if it exists and the new one doesn't
    // also check that config_home exists before putting stuff there
    if (!directoryExists(conf.cacheDirectory) ) {
        string oldMinyDir = home;
        oldMinyDir += "/.miny/";
        if (!configExists || directoryExists(oldMinyDir)) {
            conf.cacheDirectory = oldMinyDir;
        }
    }

    conf.player.field.playerName[0]='\0';

    conf.player.field.oldFinalResultDisplay=false;

    string fileName;
    if (findArgFile(fileName)) {
        std::ifstream file(fileName, ios_base::in);
        string str;
        getline(file, str);
        vector<string> argsStrs;
        vector<char*> args { nullptr };
        const char* space = " \t";
        size_t j = str.find_first_of(space);
        if (j == string::npos && str.size() > 0) j = str.size();
        for (size_t i = 0; i != string::npos && j != string::npos;) {
            argsStrs.push_back(str.substr(i, j - i));
            cout << argsStrs.back() << endl;
            args.push_back(&argsStrs.back()[0]);
            i = str.find_first_not_of(space, j);
            if (i == string::npos) break;
            j = str.find_first_of(space, i);
            if (j == string::npos) j = str.size();
        }
        handleArgs((int)args.size(), &args[0]);
    }

    handleArgs(argc, argv);

    if (conf.player.field.playerName != "" && !isValidName(conf.player.field.playerName)) {
        cout << "Name can can only contain the characters a-z, A-Z, 0-9, underscore (_), dot" << endl
             << "(.), at sign (@) and dash (-)." << endl;
        exit(1);   
    }


    if (conf.scoreListType!=Config::List::EXPORT_CSV) {
        cout<<"Miny v"<<VERSION<<" (c) 2015-2019, 2021, 2023 spacecamper"<<endl;
        cout << "See README or --help for info and help."<<endl;
    }

    // config directory

    if (!directoryExists(conf.cacheDirectory)) {
        if (conf.defaultCacheDirectory) {
            if (execlp("mkdir", "mkdir", conf.cacheDirectory.c_str(), NULL)) {
                cerr << "Error creating config directory. Exiting." << endl;
                exit(1);
            }
        } else {
            cout << "Specified config directory doesn't exist. Please create it first." << endl;
            exit(1);
        }
    }

    if (conf.printScores) { // list scores
        conf.listScores();
        exit(0);
    }
    if (conf.playReplay) {
        displayReplay();
    } else {
        if (conf.player.field.playerName == "") {
            if (isValidName(getenv("USER"))) {
                conf.player.field.playerName = getenv("USER");
            } else {
                conf.player.field.playerName = "unnamed";
            }
        }
        beginGame();
    }
    glutMainLoop();
    return 0;
}
