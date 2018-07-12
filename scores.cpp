#include "common.h"
#include "scores.h"

Score::Score() {
    timeStamp=0;
    strcpy(name,"not-set-yet");
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

float Score::getIOE() {
    return (float)val3BV/(float)(effectiveClicks+ineffectiveClicks);
}

float Score::get3BVs() {
    return 1000*(float)val3BV/time;
}

void Score::writeToFile(ofstream *f) {

    *f << timeStamp << " " << name << " " << replayNumber << " " << width << " " << height <<
         " " << mines << " " << time << " " << val3BV << " " << flagging << " " << effectiveClicks <<
         " " << ineffectiveClicks << " " << squareSize << " " << gameWon << endl;


}

void Score::readFromFile(ifstream *f) {


    *f >> timeStamp >> name >> replayNumber >> width >> height >> mines >> time >> val3BV >> flagging >> effectiveClicks >> ineffectiveClicks >> squareSize >> gameWon;




//     cout << name << " "<<time<<endl;

}




int compareByTime(const void *a,const void *b) {

    if ( (*(Score*)a).time <  (*(Score*)b).time ) return -1;
    if ( (*(Score*)a).time >  (*(Score*)b).time ) return 1;

    return 0;

}

int compareBy3BVs(const void *a,const void *b) {

    float v1=(*(Score*)a).get3BVs();
    float v2=(*(Score*)b).get3BVs();

 /*   v1=1000*(*(Score*)a).val3BV/(*(Score*)a).time;
    v2=1000*(*(Score*)b).val3BV/(*(Score*)b).time;
*/
    if ( v1 <  v2 ) return 1;
    if ( v1 >  v2 ) return -1;

    return 0;

}


int compareByIOE(const void *a,const void *b) {

    float v1=1000*(*(Score*)a).getIOE();
    float v2=1000*(*(Score*)b).getIOE();

    if ( v1 <  v2 ) return 1;
    if ( v1 >  v2 ) return -1;

    return 0;

}


int filterScores(Score *scores, int count, Score **filteredScores, int fla, int fin, int w, int h, int m, int ss, char *pname) {

    *filteredScores=new Score[count];    // just allocate array of the same size for the filtered scores


    int counter=0;

    for (int i=0;i<count;i++) {        
            

		bool isBeg=scores[i].width==9 and scores[i].height==9 and scores[i].mines==10;
		bool isInt=scores[i].width==16 and scores[i].height==16 and scores[i].mines==40;
		bool isExp=scores[i].width==30 and scores[i].height==16 and scores[i].mines==99;
		bool isBegC=scores[i].width==8 and scores[i].height==8 and scores[i].mines==10;
	    bool isStandard=isBeg or isInt or isExp or isBegC;

		if (	// flagging
			((fla==0) or (fla==1 and scores[i].flagging) or (fla==2 and !scores[i].flagging))
        
	        and
				// finished
			((fin==0) or (fin==1 and scores[i].gameWon) or (fin==2 and !scores[i].gameWon))
        
			and 
					// difficulty
			/*((dif==0 and isStandard) or (dif==1 and isBeg) or (dif==2 and isInt) or (dif==3 and isExp)
				or (dif==4 and isBegC))*/
            (
                (scores[i].width==w and scores[i].height==h and scores[i].mines==m)
                or
                (w==0 and h==0 and m==0 and isStandard) // XXX BUG - for non-standard sizes scores can't be filtered for nf without specifying w,h,m
            )

			and 
				// square size
			(ss==0 or scores[i].squareSize==ss)

			and 
				// player name
			(pname[0]=='\0' or !strcmp(pname,scores[i].name))

		    ) {

		        (*filteredScores)[counter]=scores[i];
		        counter++;
            
        }
    }

    return counter;

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

void displayScores(Score *scores, int count,int limit,bool csv /*=false*/) {
    if (count==0) {
        if (!csv) 
            cout << "No scores for this game setup yet." << endl;
        return;
    }

    if (!csv) {

        int outputCount;

    
        if (limit==0)
            outputCount=count;
        else
            if (limit<count)
                outputCount=limit;
            else
                outputCount=count;


        int tw=terminalWidth();

        unsigned int maxRankLen=intLength(outputCount);
        unsigned int maxNameLen=4;
        unsigned int maxTimeLen=4;
        unsigned int max3BVLen=3;


        // find max. lengths

        for (int i=0;i<outputCount;i++) {

            if (strlen(scores[i].name)>maxNameLen)
                maxNameLen=strlen(scores[i].name);

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

            if (scores[i].width==8 and scores[i].height==8 and scores[i].mines==10)
                currentLine  <<"  "<< "beC";
            else if (scores[i].width==9 and scores[i].height==9 and scores[i].mines==10)
                currentLine  <<"  "<< "beg";
            else if (scores[i].width==16 and scores[i].height==16 and scores[i].mines==40)
                currentLine  <<"  "<< "int";
            else if (scores[i].width==30 and scores[i].height==16 and scores[i].mines==99)
                currentLine  <<"  "<< "exp";
            else
                currentLine  <<"  "<< "oth";

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

        for (int i=0;i<count;i++) {        
                    
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




int loadScores(char *fname, Score **scores) {


    *scores=NULL;
    std::ifstream inFile(fname); 

    if (!inFile.is_open())
        return 0;

    int version;

    //inFile.read((char *) &version, 4);

    inFile >> version;

    Score tmps;
    int count=0;
        
    switch(version) {
    case SCORE_FILE_VERSION:
    {
        string content((istreambuf_iterator<char>(inFile) ), (istreambuf_iterator<char>()    )) ;
        if (string::npos != content.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_ \x0d\x0a")) {
            cout << "Score file contains invalid characters. Exiting."<<endl;
            exit(1);
        }
        
        inFile.close();
        inFile.open(fname);
            
        inFile >> version;
        while (!inFile.eof()) {
            tmps.readFromFile(&inFile);
          //  inFile.read((char *) &tmphs, sizeof(Score));
          //  cout << "hs "<<count<< " replay: "<<tmphs.replayFile<<endl;
            count++; 
        }
        count--;
    //    cout << "Counted "<<count<<" scores in score file."<<endl;

        if (count==0)
            break;


        inFile.close();
        inFile.open(fname);

        //inFile.read((char *) &version, 4);


        inFile >> version;

        *scores=new Score[count];
        

        for (int i=0;i<count;i++) {
            (*scores)[i].readFromFile(&inFile);

        }
        
        break;
    }
    default:
        cerr << "Unsupported score file version. Try upgrading to the newest version of the program."<<endl;
        exit(1);
        count=0;
        break;
    }

    inFile.close();

    
    return count;

}


void appendScore(char *fname, Score score) {

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



bool evalScore2(ostringstream *scoreString, Score s, Score *scoresAll,int countAll,char *stringValueName,int (*compareFunc)(const void *,const void *),int anyRank=0) {

    qsort(scoresAll,countAll,sizeof(Score),compareFunc);

    

    int posAll,posNF;
    char suffixAll[3],suffixNF[3];

    for (posAll=0;posAll<countAll;posAll++) {
        if (compareFunc((const void*) &s,(const void*) &scoresAll[posAll])<0)
            break;
    }

    ordinalNumberSuffix(suffixAll,posAll+1);



    if (!s.flagging) {
        Score *scoresNF;
        char zero='\0';

        int countNF=filterScores(scoresAll, countAll,&scoresNF,2,1,s.width,s.height,s.mines,0,&zero); 

        for (posNF=0;posNF<countNF;posNF++) {
            if (compareFunc((const void*) &s,(const void*) &scoresNF[posNF])<0) {
                break;
            }
        }
        free(scoresNF);
    }
    else 
        posNF=MAX_HS+1;

    ordinalNumberSuffix(suffixNF,posNF+1);

    if (anyRank) {
        
        if (!s.flagging) {
            
            *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll
                                                 <<" ("<<posNF+1<<suffixNF<<" NF)"<<endl;
        }
        else {
            

            *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll<<endl;
        }
        return true;
    }
    else if (posAll<MAX_HS and posNF<MAX_HS) {
        *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll
                                                 <<" ("<<posNF+1<<suffixNF<<" NF)"<<endl;
        
        return true;
    }
    else if (posAll<MAX_HS) {
        *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll<<endl;
        return true;
    }
    else if (posNF<MAX_HS) {
        *scoreString <<setw(8)<<left<< stringValueName<<posNF+1<<suffixNF<<" NF"<<endl;
        return true;
    }
    return false;






}

void evalScore(Score s, Score *scores, int count, int w, int h, int m, bool anyRank/*=0*/) {

    // anyRank = display how score ranks even if it ranks below MAX_HS

    /*if (difficulty!=1 and difficulty!=2 and difficulty!=3 and difficulty!=4) {
		cout << "Scores for games with the current board dimensions and mine count aren't"<<endl<<"evaluated as potential high scores."<<endl<<endl;
        return;
	}*/


    Score *scoresFiltered;
    
    char zero='\0';

    // only won games from the current difficulty
    int countFiltered=filterScores(scores, count,&scoresFiltered,0,1,w,h,m,0,&zero); 
     


    
//    cout << "filtered scores: "<<countF<<endl;

    

    ostringstream scoreString;

    // TIME

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"Time",compareByTime,anyRank);
        

    // 3BV/s

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"3BV/s",compareBy3BVs,anyRank);


    // IOE

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"IOE",compareByIOE,anyRank);


    if (scoreString.str().length()!=0) {

        if (anyRank)
            cout << "Your score ranks as (out of "<<countFiltered+1<<" ";   // countFiltered+1 because current count + 1 new score
        else 
            cout << "High score reached (";

        if (w==9 and h==9 and m==10)
            cout << "Beginner";
        else if (w==16 and h==16 and m==40)
            cout << "Intermediate";
        else if (w==30 and h==16 and m==99)
            cout << "Expert";
        else if (w==8 and h==8 and m==10)
            cout << "Beginner classic";
        else
            //cout << "\"width: " << w << ", height: " << h << ", mines: " << m << "\"";
            cout << '\'' << w << "x" << h << ", " << m << " mines'";


        if (anyRank)
            cout << " scores";
        else            
            cout <<" top "<<MAX_HS;

        cout << "): "<<endl<<scoreString.str();
    }
    else {
        cout << "You didn't get a high score."<<endl;
    }
   // cout << endl;

   //   cout << "here3"<<endl;
    free(scoresFiltered);

}

