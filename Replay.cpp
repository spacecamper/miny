#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "common.h"
#include "Config.h"
#include "Timer.h"
#include "Replay.h"
#include "Action.h"
#include "Field.h"

Replay::Replay() {
    recording=false;
}

void Replay::deleteData() {
    data.clear();
}

void Replay::recordEvent(int x, int y, int button, long elapsedTime) {
    if (recording) {
        data.push_back(*(new Action(x,y,button,(conf.gameState==Config::GAME_INITIALIZED ? 0 : elapsedTime))));
    //    cout << "Elapsed time: " << elapsedTime << endl;
    }
}

void Replay::writeToFile(ofstream *file, void* fieldPtr, Score *score) {
    Field* field = (Field*)fieldPtr;

    *file << "miny-replay-file-version: 2" << endl;

   /* *file << field->playerName << endl << squareSize << endl;

    *file << field->width << " " << field->height << endl;
*/

    score->writeToFile(file);

    for (int j=0;j<field->height;j++) {
        for (int i=0;i<field->width;i++) 
            *file << field->isMine(i,j) << " ";
        *file << endl;
    }

    *file << endl;
    *file << data.size() << endl;

    std::list<Action>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        *file << (*iter).timeSinceStart << " " << (*iter).x << " " << (*iter).y << " " << (*iter).button << endl;
    }
}

void Replay::dump() {
    cout << "Dumping replay data."<<endl;
    
    std::list<Action>::iterator iter;

    for (iter=data.begin(); iter!=data.end(); iter++) {
        (*iter).dump();
    }
}
