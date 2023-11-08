#include "common.h"
 
using namespace std;

bool isValidName(const string& s) {
   return string::npos == s.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.@-");
}

const char* ordinalNumberSuffix(int n) {
    int rem100=n%100;
    if (rem100==11 or rem100==12 or rem100==13) {
        return "th";
    }

    switch(n%10) {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    default: return "th";
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

