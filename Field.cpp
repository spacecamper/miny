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
#include "scores.h"

using namespace std;

extern void redisplay();

extern bool playReplay;
extern int gameState;
extern bool isFlagging;
extern Timer timer;
extern bool gamePaused;
extern bool anyRank;
extern unsigned short hitMineX, hitMineY;
extern int squareSize;
extern char playerName[21];
extern char highScoreDir[100];

extern void saveReplay(const char*, Replay*, Field*);
extern long findLowestUnusedReplayNumber();

void Field::ffmProc(int tmpField[MAX_WIDTH][MAX_HEIGHT],int i,int j) {

   // cout << "ffmproc(" << i << ", " << j << ")" << endl;

    if (tmpField[i][j]!=3) { 
        bool isZero=(tmpField[i][j]==0);

        tmpField[i][j]=3;

        if (isZero) 
            floodFillMark(tmpField,i,j);
        
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
        mineCount=height*width-1;
    else if (mineCount<1)
        mineCount=1;

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
    gameState=GAME_INITIALIZED;
    isFlagging=false;
    timer.reset();

    gamePaused=false;

    effectiveClicks=0;
    ineffectiveClicks=0;
}

void Field::newGame() {
    init();
    cout << "------------------------------------------------------------" << endl; 
}

void Field::endGame(const bool won) {
    timer.stop();
    replay.stopRecording();
    gameState=won ? GAME_WON : GAME_LOST;

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
    newScore.flagging=isFlagging;


    newScore.effectiveClicks=effectiveClicks;
    newScore.ineffectiveClicks=ineffectiveClicks;

    newScore.squareSize=squareSize;
    newScore.gameWon=won;

    if(!playReplay) {
        char fullpath[100];
        strcpy(fullpath,highScoreDir);
        strcat(fullpath,"scores.dat");
        if(won) {
            cout << endl<<"YOU WIN!"<<endl;

            cout <<setw(8)<<left << "IOE: " << setprecision(4)<<fixed<< newScore.getIOE()<<endl;
        
            cout << setw(8)<<left << "3BV: " << get3BV()<<endl;

            cout << setw(8)<<left << "Time: " << setprecision(3) << fixed << timeTaken/1000.0
                << " s" << endl;
            cout << setw(8)<<left << "3BV/s: " << setprecision(4)<< fixed<<newScore.get3BVs()<<endl;

            cout << "You played " << (isFlagging?"":"non-") << "flagging."<<endl;
            cout << endl;

            Score *scores;

            int count=loadScores(fullpath,&scores);

            evalScore(newScore,scores, count, width, height, mineCount,anyRank);

            free(scores); 

            // find the lowest unused replay file number
            
            long nr=1;

            nr=findLowestUnusedReplayNumber();
       
            newScore.replayNumber=nr;
            appendScore(fullpath,newScore);

            char rfname[100];

            char tmp[100];
            strcpy(tmp,highScoreDir);
            sprintf(rfname,"%lu.replay",nr);

            saveReplay(rfname,&replay, this);

            saveReplay("last.replay",&replay, this);
        } 
        else {
            cout << endl<< "YOU HIT A MINE. You played for " << setprecision(3) << fixed <<
                timer.calculateElapsedTime()/1000.0 <<" seconds." << endl << "3BV:  " 
                << setprecision(4) << fixed << get3BV() << endl;

            newScore.replayNumber=0;
            appendScore(fullpath,newScore);

            saveReplay("last.replay",&replay, this);
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

            if (gameState==GAME_INITIALIZED or gameState==GAME_PLAYING) {
                
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

    return;

    float progress=getGameProgress();
    
    if (progress==0) return;
    cout << "Est.:   "<<(int)(effectiveClicks/progress)<<" / "<<(int)(ineffectiveClicks/progress)<< " Time: " << (int)(timer.calculateElapsedTime()/progress)<<"  "<<(int)(progress*100)<<"%"<<endl;
}

void Field::startGame(int squareX, int squareY) {
    if (!playReplay)
        placeMines(squareX,squareY);

    timer.start();
    gameState=GAME_PLAYING;
}

void Field::click(int x,int y,int button) {
    viewClicks();
    replay.recordEvent(x, y, button);

    int squareX=(x-FIELD_X)/squareSize;
    int squareY=(y-FIELD_Y)/squareSize;

    if (!replay.isRecording()) {
        replay.deleteData();
        replay.startRecording();
    }

    if(gameState==GAME_INITIALIZED and button==GLUT_LEFT_BUTTON) {
        startGame(squareX, squareY);
    }
    
    if(state[squareX][squareY]==9 and (button==GLUT_LEFT_BUTTON or button==GLUT_RIGHT_BUTTON)) { // unrevealed
        if(button==GLUT_LEFT_BUTTON) {
            revealSquare(squareX, squareY);
            effectiveClicks++;
        }
        else if(button==GLUT_RIGHT_BUTTON) {
            state[squareX][squareY]=10;
            if (!isFlagging and !playReplay) {
                cout<<"You are now playing with flagging."<<endl;
                isFlagging=true;
            }
            effectiveClicks++;
        }
    }

    else if(state[squareX][squareY]==10 and button==GLUT_RIGHT_BUTTON){ // flag
        state[squareX][squareY]=9;
        effectiveClicks++;
    }
    
    else if (state[squareX][squareY]<=8 and state[squareX][squareY]!=0 and adjacentMinesFlagged(squareX,squareY) and (button==GLUT_LEFT_BUTTON or button==GLUT_RIGHT_BUTTON or button==GLUT_MIDDLE_BUTTON)) {
        effectiveClicks++;
        revealAround(squareX,squareY);
    }
    else {
        ineffectiveClicks++;
    }
}



float Field::getGameProgress() {

    int count=0;

    for (int x=0;x<width;x++)
        for (int y=0;y<height;y++)
            if (state[x][y]<=8)
                count++;

    if (count==0) return 0;

    return (float)count/(width*height-mineCount);

}
