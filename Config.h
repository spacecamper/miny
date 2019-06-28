#ifndef CONFIG_H
#define CONFIG_H

#include "Player.h"

struct Config {
    Player* player;
    int windowWidth;
    int windowHeight;
    int originalWidth;
    int originalHeight;
    int squareSize;
};

#endif
