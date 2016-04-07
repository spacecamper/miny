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
extern bool isFlagging;
extern Timer timer;
extern bool gamePaused;
extern Replay replay;
extern unsigned short hitMineX, hitMineY;
extern int squareSize;

extern void saveReplay(char *, Replay *);
 

extern void endGameWon();
extern void endGameLost();



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

/*
    // debug output

    for (int j=0;j<height;j++) {
        for (int i=0;i<width;i++)
            cout << tmpField[i][j] << " ";
        cout << endl;
    }*/

    // count

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (tmpField[i][j]==0) {
                tmp3BV++;
                tmpField[i][j]=3;

                floodFillMark(tmpField,i,j);
            }
    /*
    // debug output

    for (int j=0;j<height;j++) {
        for (int i=0;i<width;i++)
            cout << tmpField[i][j] << " ";
        cout << endl;
    }*/

    // count

    for (int i=0;i<width;i++) 
        for (int j=0;j<height;j++) 
            if (tmpField[i][j]==1)
                tmp3BV++;
    

   // cout << "3bv==" << tmp3BV << endl;

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

   // calculate3BV();
    val3BV=0;

    redisplay();

  //  cout << "Mines placed." << endl;


}

void Field::init() {
    

    for (int i=0;i<width;i++) {
        for (int j=0;j<height;j++) {
            state[i][j]=9;
        }
    }
    gameState=GAME_INITIALIZED;
    //glutPostRedisplay();
    isFlagging=false;
    timer.reset();

    gamePaused=false;

    effectiveClicks=0;
    ineffectiveClicks=0;

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
            gameState=GAME_LOST;
            if (!playReplay) {
                endGameLost();
            }
            

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
                        if ((state[i][j]==9 or state[i][j]==10) and not mine[i][j])
                            notFinished=true;

                if (!notFinished) {
                   // redisplay();
                    endGameWon();
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


void Field::click(int x,int y,int button) {

    int squareX=(x-FIELD_X)/squareSize;
    int squareY=(y-FIELD_Y)/squareSize;
    
    if (button==GLUT_LEFT_BUTTON) {
        
        if (state[squareX][squareY]==9) {   // unrevealed
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
            effectiveClicks++;
            viewClicks();
            replay.recordEvent(x,y,GLUT_LEFT_BUTTON);
            revealSquare(squareX,squareY);
            
        }
        else if (state[squareX][squareY]<=8) {  // number
            replay.recordEvent(x,y,GLUT_LEFT_BUTTON);
        
            if (state[squareX][squareY]!=0 and adjacentMinesFlagged(squareX,squareY)) {
                effectiveClicks++;
                viewClicks();
                revealAround(squareX,squareY);
                
            }
            else {
                ineffectiveClicks++;
                viewClicks();
            }
        }
        else {  // flag?
            replay.recordEvent(x,y,GLUT_LEFT_BUTTON);
            ineffectiveClicks++; 
            viewClicks();
        }
        
    }
    else if (button==GLUT_RIGHT_BUTTON) {

        if (!replay.isRecording()) {
            replay.deleteData();
            replay.startRecording();
        }

        replay.recordEvent(x,y,GLUT_RIGHT_BUTTON);
        // toggle flag or check and reveal surrounding squares
        if (state[squareX][squareY]==9) {   // unrevealed
            state[squareX][squareY]=10;
            if (!isFlagging and !playReplay)
                cout<<"You are now playing with flagging."<<endl;

            isFlagging=true;
   //         cout << "Remaining mines: " << calculateRemainingMines() << endl;

            effectiveClicks++;
            viewClicks();
        }
        else if (state[squareX][squareY]==10) { // flag
            effectiveClicks++;
            viewClicks();
            state[squareX][squareY]=9;
   //         cout << "Remaining mines: " << calculateRemainingMines() << endl;
        }
        else {  // number
            if (state[squareX][squareY]!=0 and adjacentMinesFlagged(squareX,squareY)) {
                effectiveClicks++;
                viewClicks();
                revealAround(squareX,squareY);
                
            }
            else {
                ineffectiveClicks++;
                viewClicks();
            }
        }
            
        
    }
    else if (button==GLUT_MIDDLE_BUTTON) {
        if (state[squareX][squareY]<=8) {  // number      
            replay.recordEvent(x,y,GLUT_MIDDLE_BUTTON);
            
            if (state[squareX][squareY]!=0 and adjacentMinesFlagged(squareX,squareY)) {
                effectiveClicks++;
                revealAround(squareX,squareY);
            }
            else {
                ineffectiveClicks++;
            }
            viewClicks();   
        }
        else {
            ineffectiveClicks++;
            viewClicks();   

        }
    }

    

}



float Field::getGameProgress() {

    int count=0;

    for (int x=0;x<width;x++)
        for (int y=0;y<height;y++)
            if (state[x][y]<=8)
                count++;

    if (count==0) return 0;

    //cout<<count<<endl;
    return (float)count/(width*height-mineCount);

}
