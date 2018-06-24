#ifndef ACTION_H
#define ACTION_H

class Action {
public:
    
    int x, y;
    int button;
    long timeSinceStart;

    Action();

    Action(int, int, int, long);

    void dump();
};

#endif
