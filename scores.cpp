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



int filterScores(Score *scores, int count,Score **filteredScores,int fla, int fin, int dif, int ss, char *pname) {



    *filteredScores=new Score[count];    // just allocate array of the same size for the filtered scores


    int counter=0;

    for (int i=0;i<count;i++) {        
            
        bool displayThisOne=((fla==0) 
                            or (fla==1 and scores[i].flagging)
                             or (fla==2 and !scores[i].flagging));
        
        displayThisOne=(displayThisOne and 
                            ((fin==0) 
                                or (fin==1 and scores[i].gameWon) 
                                or (fin==2 and !scores[i].gameWon)));
        
        bool isStandard=(scores[i].width==9 and scores[i].height==9 and scores[i].mines==10)
                        or (scores[i].width==16 and scores[i].height==16 and scores[i].mines==40)
                        or (scores[i].width==30 and scores[i].height==16 and scores[i].mines==99);


        displayThisOne=(displayThisOne and 
                        ((dif==0 and isStandard)
                        or (dif==1 
                            and scores[i].width==9 and scores[i].height==9 and scores[i].mines==10)
                        or (dif==2 
                            and scores[i].width==16 and scores[i].height==16 and scores[i].mines==40)
                        or (dif==3 
                            and scores[i].width==30 and scores[i].height==16 and scores[i].mines==99)
                        or dif==4
                        ));

        displayThisOne=(displayThisOne and (ss==0 or scores[i].squareSize==ss));

        displayThisOne=(displayThisOne and (pname[0]=='\0' or !strcmp(pname,scores[i].name)));
     //   cout << scores[i].name << endl;


        if (displayThisOne) {

            (*filteredScores)[counter]=scores[i];
            counter++;

            
        }
    }

    return counter;

}




void displayScores(Score *scores, int count,int limit) {


 //   cout << pname << endl;


    // TODO stop line wrapping


    unsigned int maxlen=0;

    for (int i=0;i<count;i++)
        if (strlen(scores[i].name)>maxlen)
            maxlen=strlen(scores[i].name);

    if (maxlen<4) maxlen=4;

    cout << endl<<"     " << setw(maxlen+1) << left << "Name"
        <<setw(9)<<right<< "Time"
        <<"  "<<setw(6)<<right<< "3BV/s"
        <<"  "<<setw(4)<<right<< "3BV"
        <<"  "<<setw(6)<<right<< "IOE"
        <<"  "<<setw(3)<<right<< "Fla"
        <<"  "<<setw(3)<<right<< "Fin"
        <<"  "<<setw(3)<<right<< "Dif"
        <<"  "<<setw(19)<<left<< "Date"
        <<"  "<<setw(6)<<left<< "Replay"<<endl<<endl;


   

    
  //  cout << count << " "<<countFiltered<< " " <<limit<<endl;
    int min;

    min=count;


    int outputCount;//=((limit==0)?min:limit);


    if (limit==0)
        outputCount=min;
    else
        if (limit<min)
            outputCount=limit;
        else
            outputCount=min;


    
        

    for (int i=0;i<outputCount;i++) {        
            
        // TODO add something to select/display/format columns




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

        cout << " " << setw(2) << setfill(' ') << right << i+1
            << "  " << setw(maxlen+1) << left << scores[i].name
            << setw(9) << right << scores[i].time;
             
        cout <<fixed;

        if (val3BVs==0)
            cout <<"   "<<setw(6)<< setprecision(4)<<right<<".";
        else
            cout <<"   "<<setw(6)<< setprecision(4)<<right<<val3BVs;


        if (scores[i].val3BV==0)
            cout <<"  "<<setw(4)<<right<< setfill(' ')<< ".";
        else
            cout <<" "<<setw(4)<<right<< setfill(' ')<< scores[i].val3BV;
        
        cout <<"  "<<setw(6)<<right<< setfill(' ')<< scores[i].getIOE();
        
        cout <<"  "<<setw(3)<<right<< (scores[i].flagging?"yes":"no");
        cout <<"  "<<setw(3)<<right<< (scores[i].gameWon?"yes":"no");

        if (scores[i].width==9 and scores[i].height==9 and scores[i].mines==10)
            cout  <<"  "<< "beg";
        else if (scores[i].width==16 and scores[i].height==16 and scores[i].mines==40)
            cout  <<"  "<< "int";
        else if (scores[i].width==30 and scores[i].height==16 and scores[i].mines==99)
            cout  <<"  "<< "exp";
        else
            cout  <<"  "<< "oth";

        cout  << "  " << setw(19) << left << dateString;

        if (scores[i].replayNumber!=0)
            cout<< "  " << scores[i].replayNumber;
        else
            cout<< "  " << ".";
        cout << endl;
        
    }


}




int loadScores(char *fname, Score **scores) {



    std::ifstream inFile(fname); 

    if (!inFile.is_open())
        return 0;

    int version;

    //inFile.read((char *) &version, 4);

    inFile >> version;

  //  cout << "Detected score file version: " << version<<endl;

    Score tmps;
    int count=0;
        
    switch(version) {
    case SCORE_FILE_VERSION:
        
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
       //     cout << "Reading high score "<<i+1<<endl;
            (*scores)[i].readFromFile(&inFile);
       //     inFile >> (*scores)[i].name >> (*scores)[i].time;
       //     (*scores[i]).readFromFile(&inFile);
       //     inFile.read((char *) &(*scores)[i], sizeof(Score)); // maybe this pointer is wrong
      //      cout << "Time=="<<(*scores)[i].time<<endl;

        }
      //  cout<< "Finished reading."<<endl;
     
        break;
    default:
        cerr << "Unsupported score file version. Try upgrading to the newest version of the program."<<endl;
        count=0;
        break;
    }

    inFile.close();


    
    return count;


}


void appendScore(char *fname, Score score) {

    cout << "Opening score file... "<<flush;
    
    std::ifstream inFile(fname); 

    if (inFile.is_open()) {
        cout << "Score file exists."<<endl;
        int version;
        inFile >> version;
        if (version!=SCORE_FILE_VERSION) {
            cerr << "Error saving score: unsupported score file version."<<endl;
            inFile.close();
            return;
        }  
        inFile.close();
        std::ofstream outFile(fname,ios_base::app); 
        cout << "Writing..."<<flush;
    //    outFile.write((const char *) &score, sizeof(Score));
        score.writeToFile(&outFile);
        cout << "done."<<endl;
        outFile.close();
    }
    else {
        cout << "Score file doesn't exist."<<endl;
        std::ofstream outFile(fname,ios_base::app); 
        int version=5;
        //outFile.write((const char *) &version, 4);    
        //outFile.write((const char *) &score, sizeof(Score));
        outFile << version <<endl;
        score.writeToFile(&outFile);
        outFile.close();
    }


    
}



bool evalScore2(ostringstream *scoreString, Score s, Score *scoresAll,int countAll,char *stringValueName,int (*compareFunc)(const void *,const void *)) {

    qsort(scoresAll,countAll,sizeof(Score),compareFunc);
 //   displayScores(scoresFiltered, countFiltered, MAX_HS);

    

    int posAll,posNF;
    char suffixAll[3],suffixNF[3];

    for (posAll=0;posAll<countAll;posAll++) {
        if (compareFunc((const void*) &s,(const void*) &scoresAll[posAll])<0)
            break;
    }


    if (!s.flagging) {
        Score *scoresNF;
        char zero='\0';

        // only nf games
        int countNF=filterScores(scoresAll, countAll,&scoresNF,2,1,0,0,&zero); 
      //  cout << "nf scores"<<endl;
       // displayScores(scoresNF, countNF, MAX_HS);
        for (posNF=0;posNF<countNF;posNF++) {
            if (compareFunc((const void*) &s,(const void*) &scoresNF[posNF])<0) {
                break;
            }
        }
        free(scoresNF);
    }
    else 
        posNF=MAX_HS+1;


    
    if (posAll<MAX_HS and posNF<MAX_HS) {
        ordinalNumberSuffix(suffixAll,posAll+1);
        ordinalNumberSuffix(suffixNF,posNF+1);
        *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll
                                                 <<" ("<<posNF+1<<suffixNF<<" NF)"<<endl;
        
        return true;
    }
    else if (posAll<MAX_HS) {
        ordinalNumberSuffix(suffixAll,posAll+1);
        *scoreString <<setw(8)<<left<< stringValueName<<posAll+1<<suffixAll<<endl;
        return true;
    }
    else if (posNF<MAX_HS) {
        ordinalNumberSuffix(suffixNF,posNF+1);
        *scoreString <<setw(8)<<left<< stringValueName<<posNF+1<<suffixNF<<" NF"<<endl;
        return true;
    }
    return false;

}

void evalScore(Score s, Score *scores, int count,int difficulty) {

    if (difficulty!=1 and difficulty!=2 and difficulty!=3)
        return;


    Score *scoresFiltered;
    
    char zero='\0';

    // only won games from the current difficulty
    int countFiltered=filterScores(scores, count,&scoresFiltered,0,1,difficulty,0,&zero); 
    


    
//    cout << "filtered scores: "<<countF<<endl;

    

    ostringstream scoreString;

    // TIME

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"Time",compareByTime);
        

    // 3BV/s

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"3BV/s",compareBy3BVs);

    // IOE

    evalScore2(&scoreString,s,scoresFiltered,countFiltered,"IOE",compareByIOE);


    if (scoreString.str().length()!=0) {
        cout << "High score reached (";
        switch(difficulty) {
        case 1:
            cout<<"Beginner";
            break;
        case 2:
            cout<<"Intermediate";
            break;
        case 3:
            cout<<"Expert";
            break;
        }
        cout <<" top "<<MAX_HS<<"): "<<endl<<scoreString.str();
    }
    else {
        cout << "You didn't get a high score."<<endl;
    }
    cout << endl;

   //   cout << "here3"<<endl;
    free(scoresFiltered);

}
