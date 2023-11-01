#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

#include <sys/stat.h>
#include "Config.h"
#include "common.h"
#include "Field.h"
#include "Timer.h"
#include "Replay.h"
#include "scores.h"

using namespace std;

extern void redisplay();

extern Config conf;


Field::Field() {

    val3BV=0;
    
}


bool Field::replayFileNumberExists(long nr) {
    struct stat buffer;  
    char rfname[100];
        
    char tmp[110];
    strcpy(tmp,conf.cacheDirectory);
    sprintf(rfname,"%lu.replay",nr);

    strcat(tmp,rfname);
    
    if (stat(tmp, &buffer) != 0) {
        return false;
    }

    return true;
}

long Field::findLowestUnusedReplayNumber() {
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
        
        if (replayFileNumberExists(middle)) {
            lower=middle;
        }
        else {
            upper=middle;
        }
    }

    return upper;
}

void Field::unpauseGame() {
    conf.gamePaused=false;
    timer.unpause();
    replay.recording = true;
    cout << "Game unpaused."<<endl;
}

void Field::saveReplay(const char *fname, Score *score) {
    ofstream ofile;
    
    char fullpath[110];
    strcpy(fullpath,conf.cacheDirectory);
    strcat(fullpath,fname);

    ofile.open (fullpath);

    if (!ofile.is_open()) {
        cerr<<"Error opening file " << fullpath << " for writing replay."<<endl;
        return;
    }

    replay.writeToFile(&ofile, this, score);
    ofile.close();
}

void Field::ffmProc(int tmpField[MAX_WIDTH][MAX_HEIGHT],int i,int j) {
    if (tmpField[i][j]!=3) { 
        bool isZero=(tmpField[i][j]==0);

        tmpField[i][j]=3;

        if (isZero) {
            floodFillMark(tmpField,i,j);
        }
    }

}


void Field::floodFillMark(int tmpField[MAX_WIDTH][MAX_HEIGHT],int i,int j) {

    if (i>0) {
        if (j>0) ffmProc(tmpField,i-1,j-1);
        ffmProc(tmpField,i-1,j);
        if (j<height-1) ffmProc(tmpField,i-1,j+1);
        
    }

    if (j>0) ffmProc(tmpField,i,j-1);
    if (j<height-1) ffmProc(tmpField,i,j+1);

    if (i<width-1) {
        if (j>0) ffmProc(tmpField,i+1,j-1);
        ffmProc(tmpField,i+1,j);
        if (j<height-1) ffmProc(tmpField,i+1,j+1);
    }
    

}

int Field::calculate3BV() {


    int tmpField[MAX_WIDTH][MAX_HEIGHT];    // 0=0, 1=other number, 2=mine, 3=processed

    int tmp3BV=0;


    // temp zero all

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            tmpField[i][j]=0;


    // mark "numbers" as 1

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (mine[i][j]) {
                
                if (i>0) {
                    if (j>0) tmpField[i-1][j-1]=1;
                    tmpField[i-1][j]=1;
                    if (j<height-1) tmpField[i-1][j+1]=1;
                }
                
                if (j>0) tmpField[i][j-1]=1;
                if (j<height-1) tmpField[i][j+1]=1;
            
                if (i<width-1) {
                    if (j>0) tmpField[i+1][j-1]=1;
                    tmpField[i+1][j]=1;
                    if (j<height-1) tmpField[i+1][j+1]=1;
                }
            }

    // mark mines as 2

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (mine[i][j]) 
                tmpField[i][j]=2;

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (tmpField[i][j]==0) {
                tmp3BV++;
                tmpField[i][j]=3;

                floodFillMark(tmpField,i,j);
            }

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (tmpField[i][j]==1)
                tmp3BV++;

//    cout << "[ 3BV calculated as "<<tmp3BV<<"]"<<endl;

    return tmp3BV;

}

int Field::get3BV() {
    if (val3BV==0)
        val3BV=calculate3BV();

    return val3BV;

}


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
        mineCount=height*width-2;
    else if (mineCount<2)
        mineCount=2;

}


bool Field::isMine(int x, int y) {

    if (x<0 || x>=width) {
        cerr << "x=="<<x<<" out of bounds"<<endl;
        exit(1);
    }
    else if (y<0 || y>=height) {
        cerr << "y=="<<y<<" out of bounds"<<endl;
        exit(1);
    }
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
        if (mine[x][y] or (firstClickX==x and firstClickY==y))
            i--;
        else
            mine[x][y]=true;
    }

    val3BV=0;

    redisplay();


}

void Field::init() {
    

    for (int i=0;i<width;i++) {
        for (int j=0;j<height;j++) {
            state[i][j]=9;
        }
    }
    conf.gameState=Config::GAME_INITIALIZED;
    conf.isFlagging=false;
    timer.reset();

    conf.gamePaused=false;

    effectiveClicks=0;
    ineffectiveClicks=0;
}

void Field::newGame() {
    init();
    replay.deleteData();
    cout << "------------------------------------------------------------" << endl; 
}

void Field::showStatistics(bool won, Score score, bool playingReplay) {

    if (won) {

        
        cout << endl<<"YOU WIN!"<<endl;

        if (playingReplay) {
            cout << setw(8)<<left << "Time: " << setprecision(3) << fixed << score.time/1000.0
                << " s" << endl;
            cout << setw(8)<<left << "3BV/s: " << setprecision(4)<< fixed<<score.get3BVs()<<endl;

            cout <<setw(8)<<left << "IOE: " << setprecision(4)<<fixed<< score.getIOE()<<endl;
        
            cout << setw(8)<<left << "3BV: " << get3BV()<<endl;
        }
        
        cout << "You played " << (conf.isFlagging?"":"non-") << "flagging."<<endl;
        cout << endl;
    }
    else {
        cout << endl<< "YOU HIT A MINE. You played for " << setprecision(3) << fixed <<
            score.time/1000.0 <<" seconds." << endl << "3BV: " 
            << setprecision(4) << fixed << score.val3BV << endl;
    }

}

void Field::endGame(const bool won) {
    timer.stop();
    replay.recording = false;
    conf.gameState=won ? Config::GAME_WON : Config::GAME_LOST;

    redisplay();

    const long timeTaken=timer.calculateElapsedTime();
    const long ts=time(NULL);
    
    Score newScore;

    newScore.timeStamp=ts;
    strcpy(newScore.name,playerName);
    newScore.width=width;
    newScore.height=height;
    newScore.mines=mineCount;

    newScore.time=timeTaken;
    newScore.val3BV=get3BV();
    newScore.flagging=conf.isFlagging;


    newScore.effectiveClicks=effectiveClicks;
    newScore.ineffectiveClicks=ineffectiveClicks;

    newScore.squareSize=conf.squareSize;
    newScore.gameWon=won;

    viewClicks();



    
    
    
    if(!conf.playReplay) {
        char fullpath[110];
        strcpy(fullpath,conf.cacheDirectory);
        strcat(fullpath,"scores.dat");
 
        showStatistics(won,newScore,conf.playReplay);
        
        if(won) {
      //      cout << endl << "YOU WIN!" << endl;
      //      cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;
            
            int oldFinalResultDisplay = conf.player->field.oldFinalResultDisplay;

            Score *scores;

            int count=loadScores(fullpath,&scores);


            evalScore(newScore,scores, count, width, height, mineCount,oldFinalResultDisplay,conf.scoreListLength); // XXX

            free(scores); 

            // find the lowest unused replay file number
            
            long nr=1;

            nr=findLowestUnusedReplayNumber();
            newScore.replayNumber=nr;
            appendScore(fullpath,newScore);

            char rfname[100];

            char tmp[110];
            strcpy(tmp,conf.cacheDirectory);
            sprintf(rfname,"%lu.replay",nr);

            saveReplay(rfname,&newScore);

            saveReplay("last.replay",&newScore);
        } 
        else {
       //     cout << endl << "YOU HIT A MINE!" << endl;
        //    cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;
            
            newScore.replayNumber=0;
            appendScore(fullpath,newScore);

            saveReplay("last.replay", &newScore  );
        }
    }
    else {
        //showStatistics(won,newScore);


        if (conf.player->replayHasScore) {
            showStatistics(won, conf.player->score, conf.playReplay);
        }
    }
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

            state[squareX][squareY] = 11;            
            
            endGame(false);
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
            
            state[squareX][squareY]=adjacentMines;

            if (adjacentMines==0) {
                revealAround(squareX,squareY);
            }

            if (conf.gameState==Config::GAME_INITIALIZED or conf.gameState==Config::GAME_PLAYING) {
                
                bool notFinished=false;

                for (int i=0;i<width;i++) 
                    for (int j=0;j<height;j++) 
                        if ((state[i][j]==9 or state[i][j]==10) and not mine[i][j])
                            notFinished=true;

                if (!notFinished) {
                    endGame(true);
                }    
            }
    
        }
    } 
}

bool Field::adjacentMinesFlagged(int squareX,int squareY) {
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

    if (flaggedAdjacentMines==state[squareX][squareY]) 
        return true;
    else
        return false;
}

int Field::calculateRemainingMines() {
    int remaining=mineCount;
    
    for (int i=0;i<width;i++) 
        for (int j=0;j<width;j++) 
            if (state[i][j]==10)
                remaining--;

    return remaining;
}

void Field::viewClicks() {
    cout << "Clicks: "<<effectiveClicks<<" / "<<ineffectiveClicks<<endl;
}

void Field::startGame(int squareX, int squareY) {
    if (!conf.playReplay)
        placeMines(squareX,squareY);

    timer.start();
    conf.gameState=Config::GAME_PLAYING;
}

void Field::click(int x,int y,int button) {

    int squareX=(x-FIELD_X)/conf.squareSize;
    int squareY=(y-FIELD_Y)/conf.squareSize;


    if (!replay.recording and !conf.playReplay and !conf.gamePaused) {
        replay.recording = true;
    }    

    if (conf.gameState==Config::GAME_INITIALIZED and button==GLUT_LEFT_BUTTON) {
        startGame(squareX, squareY);
    }
    
    replay.recordEvent(x, y, button, timer.calculateElapsedTime());   
    

    if (button!=-1) {

        if(state[squareX][squareY]==9 and (button==GLUT_LEFT_BUTTON or button==GLUT_RIGHT_BUTTON)) { // unrevealed
            if(button==GLUT_LEFT_BUTTON) {
                effectiveClicks++;
                revealSquare(squareX, squareY);
            }
            else if(button==GLUT_RIGHT_BUTTON) {
                state[squareX][squareY]=10;
                if (!conf.isFlagging) {
                    cout<<"You are now playing with flagging."<<endl;
                    conf.isFlagging=true;
                }
                effectiveClicks++;
            }
        }

        else if(state[squareX][squareY]==10 and button==GLUT_RIGHT_BUTTON){ // flag
            state[squareX][squareY]=9;
            effectiveClicks++;
        }
        
        else if (state[squareX][squareY]<=8 and state[squareX][squareY]!=0 and
                adjacentMinesFlagged(squareX,squareY) and
                (button==GLUT_LEFT_BUTTON or button==GLUT_RIGHT_BUTTON or button==GLUT_MIDDLE_BUTTON)) {
            effectiveClicks++;
            revealAround(squareX,squareY);
        }
        else if (button==GLUT_LEFT_BUTTON or button==GLUT_RIGHT_BUTTON or button==GLUT_MIDDLE_BUTTON) {
            ineffectiveClicks++;
        }
        
        if (conf.gameState==0)
            viewClicks();
    }
}
