#ifndef FIELD_H
#define FIELD_H

#include "Replay.h"
#include "Timer.h"
#include "common.h"

class Field {
public:
    short width, height;
    short mineCount;
    char playerName[21];

    Replay replay;
    Timer timer;

    bool mine[MAX_WIDTH][MAX_HEIGHT];
    int state[MAX_WIDTH][MAX_HEIGHT];  // 0-8 - adjacent mine count for revealed field, 9 - not revealed, 10 - flag
    int val3BV;
    unsigned int effectiveClicks;
    unsigned int ineffectiveClicks;

    bool isMine(int x, int y);
    void setMine(int x, int y);
    void init();
    void newGame();
    int calculateRemainingMines();
    void checkValues();
    void click(int, int, int);
    void unpauseGame();
private:
    void endGame(const bool);
    void viewClicks();
    int get3BV();
    void startGame(int, int);
    void placeMines(int firstClickX, int firstClickY);
    void saveReplay(const char*);
    long findLowestUnusedReplayNumber();
    void revealAround(int squareX, int squareY);
    bool adjacentMinesFlagged(int squareX,int squareY);
    void revealSquare(int squareX, int squareY);
    bool replayFileNumberExists(long);
    void floodFillMark(int [MAX_WIDTH][MAX_HEIGHT],int,int);
    void ffmProc(int [MAX_WIDTH][MAX_HEIGHT],int,int);
    int calculate3BV(); //Calculates minimum number of clicks to solve the board
};

#endif
