#include "common.h"
#include "scores.h"
#include "Config.h"

Score::Score() {
    timeStamp=0;
    name = "name-not-set";
    //strcpy(replayFile,"*");
    replayNumber=0;
    width=0;
    height=0;
    mines=0;
    time=0;
    val3BV=0;
    flagging=false;
    effectiveClicks=0;
    ineffectiveClicks=0;
    squareSize=0;
    gameWon=false;


}

float Score::getIOE() const {
    return (float)val3BV/(float)(effectiveClicks+ineffectiveClicks);
}

float Score::get3BVs() const {
    //cout << "val3BV=" << val3BV <<end;
    return 1000*(float)val3BV/time;
}

void Score::writeToFile(ofstream *f) {
    *f << timeStamp << " " << name << " " << replayNumber << " " << width << " " << height <<
         " " << mines << " " << time << " " << val3BV << " " << flagging << " " << effectiveClicks <<
         " " << ineffectiveClicks << " " << squareSize << " " << gameWon << endl;
}

void Score::readFromFile(ifstream *f) {
    *f >> timeStamp >> name >> replayNumber >> width >> height >> mines >> time >> val3BV >> flagging >> effectiveClicks >> ineffectiveClicks >> squareSize >> gameWon;
}
Score Score::readNewFromFile(ifstream *f) {
    Score ret;
    ret.readFromFile(f);
    return ret;
}

int compareByTime(const Score& a,const Score& b) {

    if ( a.time <  b.time ) return -1;
    if ( a.time >  b.time ) return 1;

    return 0;

}

int compareBy3BVs(const Score& a,const Score& b) {

    float v1=a.get3BVs();
    float v2=b.get3BVs();

    if ( v1 <  v2 ) return 1;
    if ( v1 >  v2 ) return -1;

    return 0;

}


int compareByIOE(const Score& a,const Score& b) {

    float v1=1000*a.getIOE();
    float v2=1000*b.getIOE();

    if ( v1 <  v2 ) return 1;
    if ( v1 >  v2 ) return -1;

    return 0;

}


vector<Score> filterScores(const vector<Score>& scores, Flagging fla, Finished fin, const string& pname) {
    vector<Score> filteredScores;
    for (int i=0;i<scores.size();i++) {        
        const Score& s = scores[i];
        Field& f = conf.player.field;
        auto matches = [] (bool flag, int val, int tru, int fals) {
          return (val & tru and flag) or (val & fals and not flag);
        };
        if (matches(s.flagging, (int)fla, (int)Flagging::FLAGGING, (int)Flagging::NO_FLAGGING)
            and matches(s.gameWon, (int)fin, (int)Finished::FINISHED, (int)Finished::UNFINISHED)
            and (pname == "" or pname == scores[i].name)
            // Use baseDifficulty here as getDifficulty() can't return 0.
            and (conf.baseDifficulty == 0
                 or (s.width == f.width and s.height == f.height and s.mines == f.mineCount))
        ) {
            filteredScores.push_back(s);
        }
    }
    return filteredScores;
}


unsigned int intLength(int n)
{    
   // cout << "length("<<n<<")==";

    int length=0;
    while(n!=0) {        
        n=n/10;
        length++;
    }

   // cout<<length<<endl;

    return length;
}

void displayScores(const vector<Score>& scores, int limit,bool csv /*=false*/) {
    if (scores.size()==0) {
        if (!csv) 
            cout << "No scores for this game setup yet." << endl;
        return;
    }

    if (!csv) {

        int outputCount;

    
        if (limit==0)
            outputCount=scores.size();
        else
            if (limit<scores.size())
                outputCount=limit;
            else
                outputCount=scores.size();


        int tw=terminalWidth();

        unsigned int maxRankLen=intLength(outputCount);
        unsigned int maxNameLen=4;
        unsigned int maxTimeLen=4;
        unsigned int max3BVLen=3;


        // find max. lengths

        for (int i=0;i<outputCount;i++) {

            if (scores[i].name.size()>maxNameLen)
                maxNameLen=scores[i].name.size();

            int currentTimeLen=intLength(scores[i].time);
            if (currentTimeLen>maxTimeLen) 
                maxTimeLen=currentTimeLen;

            int current3BVLen=intLength(scores[i].val3BV);
            if (current3BVLen>maxTimeLen) 
                max3BVLen=current3BVLen;

        }

        maxTimeLen++;   // for decimal point

/*
        if (maxNameLen<4) maxNameLen=4;
        if (maxTimeLen<4) maxTimeLen=4;
        if (max3BVLen<3) max3BVLen=3;

        */

        

        ostringstream headerLine;

        headerLine << setw(maxRankLen) << right << ""
            <<"  "<<setw(maxNameLen) << left << "Name"
            <<"  "<<setw(maxTimeLen)<<right<< "Time"
            <<"  "<<setw(6)<<right<< "3BV/s"
            <<"  "<<setw(max3BVLen)<<right<< "3BV"
            <<"  "<<setw(6)<<right<< "IOE"
            <<"  "<<setw(3)<<right<< "Fla"
          //  <<"  "<<setw(3)<<right<< "Fin"
            <<"  "<<setw(3)<<right<< "Dif"
            <<"  "<<setw(19)<<left<< "Date"
            <<"  "<<left<< "Rep";


        // output table header

        bool truncated=false;
        if (outputLine(headerLine.str(),tw)) {
            truncated=true;
        }

        cout<<endl;

    


        // table rows


        for (int i=0;i<outputCount;i++) {        
                    
            ostringstream currentLine;

            string dateString;
            if (scores[i].timeStamp==0) {
                dateString=".";
            
            }
            else {
                struct tm *lt;
                lt=localtime(&scores[i].timeStamp);
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

            
            float val3BVs=scores[i].get3BVs();//(float)1000*scores[i].val3BV/scores[i].time;

            
            // name and time

            currentLine << setw(maxRankLen) << setfill(' ') << right << i+1
                << "  " << setw(maxNameLen) << left << scores[i].name
                << "  " << setw(maxTimeLen) << right << setprecision(3) << fixed 
                << scores[i].time/1000.0;
                 
            currentLine <<fixed;


            // 3BV/s
            // assume 3BV/s will always be of the format X.XXXX
            if (val3BVs==0)
                currentLine <<"  "<<setw(5)<< setprecision(4)<<fixed<<right<<".";
            else
                currentLine <<"  "<<setw(5)<< setprecision(4)<<fixed<<right<<val3BVs;

            // 3BV
            if (scores[i].val3BV==0)
                currentLine <<"  "<<setw(max3BVLen)<<right<< setfill(' ')<< ".";
            else
                currentLine <<"  "<<setw(max3BVLen)<<right<< setfill(' ')<< scores[i].val3BV;
            
            // IOE
            currentLine <<"  "<<setw(6)<<right<< setfill(' ')<< setprecision(4)<<fixed
                << scores[i].getIOE();
            
            // flagging
            currentLine <<"  "<<setw(3)<<right<< (scores[i].flagging?"yes":"no");
          
            // difficulty

            switch (Config::getDifficulty(scores[i].width, scores[i].height, scores[i].mines)) {
                case 1: currentLine  <<"  "<< "beg"; break;
                case 2: currentLine  <<"  "<< "int"; break;
                case 3: currentLine  <<"  "<< "exp"; break;
                case 4: currentLine  <<"  "<< "beC"; break;
                default: currentLine  <<"  "<< "oth"; break;
            }

            // date and time

            currentLine  << "  " << setw(19) << left << dateString;

            // replay number

            if (scores[i].replayNumber!=0)
                currentLine<< "  " << scores[i].replayNumber;
            else
                currentLine<< "  " << ".";

            if (outputLine(currentLine.str(),tw))
                truncated=true;


        }


        if (truncated)
            cout <<endl<< "Lines truncated. To see the full output resize this terminal."<<endl;

    }
    else {
        cout << "dateOfGame,timeOfGame,width,height,mines,difficulty,won,time,3BV/s,IOE,3BV,flagging,name,effectiveClicks,ineffectiveClicks,squareSize,replay"<<endl;

        for (int i=0;i<scores.size();i++) {        
                    
            ostringstream currentLine;

            string dateString;
            if (scores[i].timeStamp==0) {
                dateString=".";
            
            }
            else {
                struct tm *lt;
                lt=localtime(&scores[i].timeStamp);
                ostringstream stringStream;
                stringStream <<setw(4)<< setfill(' ')<<right<< lt->tm_year+1900
                    <<'-'
                    <<setw(2)<< setfill('0')<<lt->tm_mon+1
                    <<'-'
                    <<setw(2)<< setfill('0')<<lt->tm_mday
                    <<','         
                    <<setw(2)<< setfill('0')<<lt->tm_hour
                    <<':'
                    <<setw(2)<< setfill('0')<<lt->tm_min
                    <<':'
                    <<setw(2)<< setfill('0')<<lt->tm_sec;

                dateString = stringStream.str();
            }

            
            
            float val3BVs=scores[i].get3BVs();//(float)1000*scores[i].val3BV/scores[i].time;

            // date and time of game

            cout << dateString << ',';


            // difficulty (in numbers and string)

            cout << scores[i].width << ',' << scores[i].height << ',' << scores[i].mines << ',';

            if (scores[i].width==8 and scores[i].height==8 and scores[i].mines==10)
                cout << "beC";
            else if (scores[i].width==9 and scores[i].height==9 and scores[i].mines==10)
                cout << "beg";
            else if (scores[i].width==16 and scores[i].height==16 and scores[i].mines==40)
                cout << "int";
            else if (scores[i].width==30 and scores[i].height==16 and scores[i].mines==99)
                cout << "exp";
            else
                cout << "oth";

            cout << ',';



            // won game

            cout << (scores[i].gameWon?"1":"0") << ',';
 
            // time taken

            cout << scores[i].time << ',';


            // 3BV/s                 
            if (val3BVs==0)
                cout <<".,";
            else
                cout<<val3BVs<<',';


            // IOE
            cout<< scores[i].getIOE()<<',';

            // 3BV
            if (scores[i].val3BV==0)
                cout<< ".,";
            else
                cout<< scores[i].val3BV<<',';
            
            
            // flagging
            cout<< (scores[i].flagging?"1":"0") << ',';


            // name, effective, ineffective clicks, square size
            cout << scores[i].name << ',' << scores[i].effectiveClicks << ',' << scores[i].ineffectiveClicks << ',' << scores[i].squareSize << ',';

            // replay number

            if (scores[i].replayNumber!=0)
                cout << scores[i].replayNumber;
            else
                cout << ".";
            
            cout << endl;

        }


    }

}

vector<Score> loadScores(const string& fname) {
    vector<Score> scores;
    std::ifstream inFile(fname); 

    if (!inFile.is_open()) {
        cout << "File read error." << endl;
        exit(1);
    }
  
    int version;

    //inFile.read((char *) &version, 4);

    inFile >> version;

    switch(version) {
    case SCORE_FILE_VERSION:
    {
        string content((istreambuf_iterator<char>(inFile) ), (istreambuf_iterator<char>()    )) ;
        if (string::npos != content.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.@- \x0d\x0a")) {
            cout << "Score file contains invalid characters. Exiting."<<endl;
            exit(1);
        }
        
        inFile.close();
        inFile.open(fname);
            
        inFile >> version;

        // Need to peek before checking EOF because eof() only returns true
        // after the EOF char has been read from the stream.
        while (inFile.peek(), !inFile.eof()) {
            scores.push_back(Score::readNewFromFile(&inFile));
            inFile.ignore(999, '\n'); // remove newline
        }
        
        break;
    }
    default:
        cerr << "Unsupported score file version. Try upgrading to the newest version of the program."<<endl;
        exit(1);
        break;
    }

    inFile.close();
    return scores;
}


void appendScore(const string& fname, Score& score) {

 //   cout << "Opening score file... "<<flush;
    
    std::ifstream inFile(fname); 

    if (inFile.is_open()) {
   //     cout << "Score file exists."<<endl;
        int version;
        inFile >> version;
        if (version!=SCORE_FILE_VERSION) {
            cerr << "Error saving score: unsupported score file version."<<endl;
            inFile.close();
            return;
        }  
        inFile.close();
        std::ofstream outFile(fname,ios_base::app); 
  //      cout << "Writing..."<<flush;
        score.writeToFile(&outFile);
  //      cout << "done."<<endl;
        outFile.close();
    }
    else {
  //      cout << "Score file doesn't exist."<<endl;
        std::ofstream outFile(fname,ios_base::app); 
        int version=5;
        outFile << version <<endl;
        score.writeToFile(&outFile);
        outFile.close();
    }


    
}



void sortScores(vector<Score>& scores, ScoreCmpFunc by) {
    sort(scores.begin(), scores.end(), [&] (const Score& a, const Score& b) { return by(a, b) < 0; });
}

void evalScore2v2(ostringstream *scoreString, Score& s, vector<Score>& scoresAll,ScoreCmpFunc compareFunc,int scoreListLength) {
    // prints the place and percentile of s
     
    sortScores(scoresAll, compareFunc);

    int position;

    for (position=0;position<scoresAll.size();position++) {   // "position" is the position of the score currently being evaluated among scoresAll
        if (compareFunc(s, scoresAll[position])<0)
            break;
    }

    float percentile = 100*(1 - (float)position/(scoresAll.size() - 1));
                                                                                
                     
    *scoreString << right << fixed << 
        setw(7) << setprecision(0) << (position+1) <<   // position
       /* setw(2) <<*/     ordinalNumberSuffix(position+1) <<                            // suffix (-st, -th)
        setw(10) << setprecision(3) << percentile << " ";       // percentile

   // *scoreString << ' ';
    
    //    | IOE   |   0.667   |    847th   100.000   |    470th   100.000   | 
}


void evalScoreMid(ostringstream *scoreString,string criterionName, Score s,vector<Score>& scoresFiltered,vector<Score>& scoresFilteredNF,int (*compareFunc)(const Score&,const Score&),int scoreListLength) {

    // compare how this score ranks against older ones based on compareFunc 



    // label on the left (time, 3bv/s, ioe)
    
    *scoreString<<"| "<<left<<setw(5)<<criterionName<<" | ";

//    | Time  |   6.317 s |    296th    78.180   |    259th    68.727   | 

    
   // *scoreString<<" "<<left<<fixed<<setw(7)<<setprecision(3);
    *scoreString<<setw(7)<<right;
    
    
    // values
    
    // this game
    if (criterionName=="Time") {
        *scoreString<<setw(7)<<right<<s.time/1000.0<<" s";
    }
    else if (criterionName=="3BV/s") {
        *scoreString<<setw(7)<<right<<s.get3BVs()<<"  ";
    }
    else if (criterionName=="IOE") {
        *scoreString<<setw(7)<<right<<s.getIOE()<<"  ";
    }

    *scoreString<<" |";
    // f+nf
    evalScore2v2(scoreString,s,scoresFiltered,compareFunc,scoreListLength) ;   
    
    
    
    if (!s.flagging) {
        // nf only
        *scoreString<<"  |";
        evalScore2v2(scoreString,s,scoresFilteredNF,compareFunc,scoreListLength);
    }
            
    *scoreString<<"  | "<<endl;
}
    


void evalScore(Score s, const vector<Score>& scores, bool oldFinalResultDisplay, int scoreListLength) {

    // compare how this score ranks against older ones

    //cout<<oldFinalResultDisplay<<endl;
    if (oldFinalResultDisplay) {
  //      cout<<"YOU WIN!"<<endl;
        cout<<"Time:   "<<s.time/1000.0<<" s"<<endl;
        cout<<"3BV/s:  "<<s.get3BVs()<<endl;
        cout<<"IOE:    "<<s.getIOE()/1.0<<endl;
        cout<<"3BV:    "<<s.val3BV/1.0<<endl;

        return;
    }
        
        
    cout<<"3BV: "<<s.val3BV/1.0<<endl;

    // only won games from the current difficulty
    vector<Score> scoresFiltered = filterScores(scores, Flagging::BOTH, Finished::FINISHED, "");
    // same but only NF scores
    vector<Score> scoresFilteredNF = filterScores(scores, Flagging::NO_FLAGGING, Finished::FINISHED, "");
    

    ostringstream scoreString;

    char strAll[50], strNF[50];
    
    
    sprintf(strAll,"all games (%d)",(int)scoresFiltered.size());
    
    
    if (!s.flagging)
        sprintf(strNF,"all NF games (%d)",(int)scoresFilteredNF.size());
    else
        strNF[0]='\0';
        
        
    cout << endl << "Your result's ranking (among won ";


    switch (conf.getDifficulty()) {
        case 1: cout << "Beginner";
        case 2: cout << "Intermediate";
        case 3: cout << "Expert";
        case 4: cout << "Beginner classic";
        default:
          Field& f = conf.player.field;
          cout << "'" << f.width << "x" << f.height << ", " << f.mineCount << " mines\'";
    }
    cout << " results)"<<endl<<endl;
    
    cout << "Percentiles are approximate, see README for details." << endl << endl ;
    
    scoreString << "        +-----------+----------------------"<<((!s.flagging)?"-----------------------":"")<<"+"<<endl;
    scoreString << "        |           |   compared to        "<<((!s.flagging)?"                       ":"")<<"|"<<endl;
    scoreString << "        |   this    +----------------------"<<((!s.flagging)?"+----------------------":"")<<"+"<<endl;
    scoreString << "        |   game    |"<<setw(19) <<right<<strAll<<"   |"<<setw(20) <<right<<(!s.flagging? strNF : "")<< "  |"<<endl;
    scoreString << "        |           |    place    perc.    |"<<((!s.flagging)?"    place    perc.    |":"")<<endl;
    scoreString << "+-------+-----------+----------------------+"<<((!s.flagging)?"----------------------+":"")<<endl;

//                  | Time  |   6.317 s |    296th    78.180   |    259th    68.727   | 

    evalScoreMid(&scoreString,"Time",s,scoresFiltered,scoresFilteredNF,compareByTime,scoreListLength);
    evalScoreMid(&scoreString,"3BV/s",s,scoresFiltered,scoresFilteredNF,compareBy3BVs,scoreListLength);
    evalScoreMid(&scoreString,"IOE",s,scoresFiltered,scoresFilteredNF,compareByIOE,scoreListLength);
    
    
    
    
    scoreString << "+-------+-----------+----------------------+"<<((!s.flagging)?"----------------------+":"")<<endl;
    
    
        
    
    cout<<scoreString.str();
}

