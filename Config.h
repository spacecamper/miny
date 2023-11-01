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
    int scoreListLength;

    enum {
      GAME_INITIALIZED=-1,
      GAME_PLAYING=0,
      GAME_LOST=1,
      GAME_WON=2,
    } gameState;
    char cacheDirectory[100];
    bool isFlagging;
    bool gamePaused;
    bool playReplay;
    bool boolDrawCursor;
};

#endif
