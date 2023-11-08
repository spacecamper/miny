#ifndef CONFIG_H
#define CONFIG_H

#include "Player.h"
#include "common.h"
#include <string>

struct Config {
    enum GameState {
        GAME_INITIALIZED = -1,
        GAME_PLAYING = 0,
        GAME_LOST = 1,
        GAME_WON = 2,
    };
    enum class List : int {
        TIME = 1,
        BBBV = 2,
        IOE = 3,
        EXPORT_CSV = 4,
    };
    enum class Flagging : int {
        BOTH = 0,
        FLAGGING_ONLY = 1,
        NO_FLAGGING_ONLY = 2,
        MAX = NO_FLAGGING_ONLY,
    };
    static inline Flagging getFlaggingOrDefault(int value) {
        return (0 <= value && value <= (int)Flagging::MAX)
                ? static_cast<Flagging>(value)
                : Flagging::BOTH;
    }
    enum class Finished : int {
        BOTH = 0,
        FINISHED_ONLY = 1,
        UNFINISHED_ONLY = 2,
        MAX = UNFINISHED_ONLY,
    };
    static inline Finished getFinishedOrDefault(int value) {
        return (0 <= value && value <= (int)Finished::MAX)
                ? static_cast<Finished>(value)
                : Finished::BOTH;
    }

    inline Config() {
	  player.field.replay.recording=false;
	}


    Player player;
    int windowWidth;
    int windowHeight;
    // difficulty: 0-all of 1 to 4; 1-beg; 2-int; 3-exp; 4-beg classic; -1 custom
    // The difficulty value used from the CLI if other values were not
    // explicitly given.
    int baseDifficulty = 2;

    int squareSize = 35;
    int scoreListLength = MAX_HS;
    GameState gameState = GAME_INITIALIZED;
    List scoreListType = List::TIME;
    Flagging listFlagging = Flagging::BOTH;
    Finished listFinished = Finished::FINISHED_ONLY;
    string cacheDirectory;
    bool isFlagging;
    bool gamePaused = false;
    bool playReplay = false;
    bool boolDrawCursor = false;

    string replayFileName;
    bool printScores = false;

    bool defaultCacheDirectory = true;
    // Prefix argument given to setting adjustment during game play.
    int prefixArg = 0;

    void gameStarted();
    // Handle input relating to game configuration, rather than actions that
    // affect the game.
    void handleInput(char key);
    // Update Field to reflect the stated difficulty option, setting those
    // options that have not already been set unless force.
    void setDifficulty(bool force = false);
    // Get the value of difficulty being used.
    int getDifficulty();
    int targetWidth();
    int targetHeight();

    void handleOption(char option, char *arg, bool from_cli = true);
    // Handle only int and bool options.
    // Interactive is if the option is given in argv.
    void handleOption(char option, int arg, bool from_cli = true);

    void listScores();
};

#endif
