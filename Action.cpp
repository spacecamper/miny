#include <iostream>
#include <iomanip>
#include "Action.h"

using namespace std;

Action::Action() {}

Action::Action(int px, int py, int pb, long pp) {
    x=px;
    y=py;
    button=pb;
    timeSinceStart=pp;
}

void Action::dump() {
    cout <<setw(7)<<timeSinceStart<<setw(7) <<x<<setw(7)<<y<<setw(7)<<button<< endl;
}
