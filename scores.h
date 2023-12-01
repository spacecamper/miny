#ifndef SCORES_H
#define SCORES_H

#include <time.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>


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


    float getIOE() const;

    float get3BVs() const;

    void readFromFile(ifstream *f);
    static Score readNewFromFile(ifstream *f);
    
    void writeToFile(ofstream *f);
};

// XXX
typedef int ScoreCmpFunc(const Score& a,const Score& b);

ScoreCmpFunc compareByTime;
ScoreCmpFunc compareBy3BVs;
ScoreCmpFunc compareByIOE;

void sortScores(vector<Score>& scores, ScoreCmpFunc by);
vector<Score> filterScores(const vector<Score>& scores,int fla, int fin, int w, int h, int m, int ss, const string& pname);

void displayScores(const vector<Score>& scores, int limit,bool csv=false);

vector<Score> loadScores(const string& fname);

void appendScore(const string& fname, Score& score);

bool evalScore2(ostringstream *scoreString, Score& s, vector<Score>& scoresAll,const string& stringValueName,ScoreCmpFunc compareFunc,int scoreListLength,int *returnCountNF);

void evalScore(Score s, const vector<Score>& scores, int w, int h, int m, bool oldView,int scoreListLength);




//bool evalScore2(ostringstream *scoreString, Score s, Score *scoresAll,int countAll,char *stringValueName,int (*compareFunc)(const Score&,const Score&),int scoreListLength,int *returnCountNF   ) ;

#endif

