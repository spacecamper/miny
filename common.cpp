#include "common.h"
 
using namespace std;

bool isValidName(char *n) {

    char *c=n;
    char *validChars="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.@-";
            

    while(*c) {
        if (!strchr(validChars,*c)) {
            return false;
        }
        c++;
    }


    if (strlen(n)>MAX_NAME_LENGTH) {
        return false;
    }

    return true;
    

}



void ordinalNumberSuffix(char *suffix, int n) {

    int rem100=n%100;

    if (rem100==11 or rem100==12 or rem100==13) {
        strcpy(suffix,"th");
        return;
    }

    switch(n%10) {
    case 1: strcpy(suffix,"st"); break;
    case 2: strcpy(suffix,"nd"); break;
    case 3: strcpy(suffix,"rd"); break;
    default: strcpy(suffix,"th"); break;
    }
    
}


unsigned int terminalWidth() {

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

  //  printf ("lines %d\n", w.ws_row);
   // printf ("columns %d\n", w.ws_col);

    return w.ws_col;


}


bool outputLine(string l,int w) {

//    int tw=terminalWidth();
    bool truncated=false;

    if (l.size()>w) {
        l.resize(w);
        truncated=true;
    }

    cout << l << endl;
    return truncated;
}

