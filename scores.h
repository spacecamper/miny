#ifndef SCORES_H
#define SCORES_H

#include <time.h>
#include <fstream>

#include <iostream>

#include <iomanip>

#include <sstream>

#include <algorithm>


using namespace std;


class Score {
public:

    time_t timeStamp;
    string name;
    unsigned int replayNumber;

    unsigned short width,height;
    unsigned short mines;     
    unsigned int time;
    unsigned short val3BV;
    bool flagging;
    unsigned int effectiveClicks;
    unsigned int ineffectiveClicks;
    unsigned short squareSize;
    bool gameWon;

    Score();


    float getIOE();

    float get3BVs();

    void readFromFile(ifstream *f);
    
    void writeToFile(ofstream *f);
};

// XXX

int compareByTime(const void *a,const void *b);

int compareBy3BVs(const void *a,const void *b);

int compareByIOE(const void *a,const void *b);

int filterScores(Score *scores, int count,Score **filteredScores,int fla, int fin, int w, int h, int m, int ss, const string& pname);

void displayScores(Score *scores, int count,int limit,bool csv=false);

int loadScores(const string& fname, Score **scores);

void appendScore(const string& fname, Score score);

bool evalScore2(ostringstream *scoreString, Score s, Score *scoresAll,int countAll,const string& stringValueName,int (*compareFunc)(const void *,const void *),int scoreListLength,int *returnCountNF);

void evalScore(Score s, Score *scores, int count, int w, int h, int m, bool oldView,int scoreListLength);




//bool evalScore2(ostringstream *scoreString, Score s, Score *scoresAll,int countAll,char *stringValueName,int (*compareFunc)(const void *,const void *),int scoreListLength,int *returnCountNF   ) ;

#endif

