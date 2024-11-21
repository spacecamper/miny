#include "Config.h"

#include "scores.h"
#include <algorithm>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

Config conf;

void Config::gameStarted() {
    gameState = GAME_PLAYING;
    prefixArg = 0;
}

int Config::targetWidth() {
    return FIELD_X + player.field.width * squareSize + BORDER_WIDTH;
}
int Config::targetHeight() {
    return FIELD_Y + player.field.height * squareSize + BORDER_WIDTH;
}

void Config::handleInput(char key) {
    if (gameState == GAME_PLAYING) {
        prefixArg = 0;
        return;
    }
    if ('0' <= key && key <= '9') {
        prefixArg *= 10;
        prefixArg += key - '0';
        return;
    }
    if (key == 127) { // backspace
        prefixArg = 0;
        return;
    }
    if (gameState == GAME_WON || gameState == GAME_LOST) {
        player.field.newGame();
    }
    handleOption(key, prefixArg, false);
    prefixArg = 0;
    switch (key) {
    case 'd': case 's': case 'm': case 'w': case 'h':
        player.field.checkValues();
        glutReshapeWindow(targetWidth(), targetHeight());
        player.field.newGame();
        break;
    case 'l': case '3': case 'B': case 't': case 'i': case 'c': case 'f': case 'g':
        listScores();
        break;
    }
}

#define set_if_0(VAR, VAL) if (VAR == 0 || force) VAR = VAL;
#define set3(H, W, M) \
set_if_0(player.field.height, H);\
set_if_0(player.field.width, W);\
set_if_0(player.field.mineCount, M);
void Config::setDifficulty(bool force) {
    switch (baseDifficulty) {
    case 0: set3(0, 0, 0); break;
    case 1: set3(9, 9, 10); break;
    case 2: set3(16, 16, 40); break;
    case 3: set3(16, 30, 99); break;
    case 4: set3(8, 8, 10); break;
    }
}
int Config::getDifficulty(int width, int height, int mines) {
    if (width == 9 and height == 9 and mines == 10)
        return 1;
    else if (width == 16 and height == 16 and mines == 40)
        return 2;
    else if (width == 30 and height == 16 and mines == 99)
        return 3;
    else if (width == 8 and height == 8 and mines == 10)
        return 4;
    else
        return -1;
}
int Config::getDifficulty() {
    return getDifficulty(player.field.width, player.field.height, player.field.mineCount);
}

void Config::handleOption(char option, char *arg, bool from_cli) {
    handleOption(option, arg == nullptr ? 0 : atoi(arg), from_cli);
    switch (option) {
    case 'n':
        player.field.playerName = optarg;
        break;
    case 'p':
        replayFileName = cacheDirectory + arg + ".replay";
        playReplay = true;
        drawCursor = true;
        break;
    case 'C': {
        defaultCacheDirectory = false;
        cacheDirectory = optarg;
        if (cacheDirectory.back() != '/')
            cacheDirectory += "/";
        break;
    }
    }
}
void Config::handleOption(char option, int arg, bool from_cli) {
    switch (option) {
    case 'o':
        if (from_cli)
            player.field.oldFinalResultDisplay = true;
        else
            player.field.oldFinalResultDisplay =
                    !player.field.oldFinalResultDisplay;
        if (!from_cli)
            cout << "Displaying results in "
                     << (player.field.oldFinalResultDisplay ? "old" : "new")
                     << " format." << endl;
        break;
    case 'd':
        baseDifficulty = arg;
        if (!from_cli) {
            setDifficulty(true);
            cout << "Set difficulty to " << arg << "." << endl;
        }
      break;
    case 's':
        squareSize = clamp(arg, 3, 100);
        if (!from_cli)
            cout << "Set square size to " << squareSize << "." << endl;
        break;
    case 'm':
        player.field.mineCount = arg;
        if (!from_cli)
            cout << "Set mine count to " << arg << "." << endl;
        break;
    case 'w':
        player.field.width = arg;
        if (!from_cli)
            cout << "Set field width to " << arg << "." << endl;
        break;
    case 'h':
        player.field.height = arg;
        if (!from_cli)
            cout << "Set field height to " << arg << "." << endl;
        break;
    case 'l':
        scoreListLength = arg;
        break;
    case '3': case 'B':
        printScores = true;
        scoreListType = List::BBBV;
        break;
    case 't':
        printScores = true;
        scoreListType = List::TIME;
        break;
    case 'i':
        printScores = true;
        scoreListType = List::IOE;
        break;
    case 'c':
        printScores = true;
        scoreListType = List::EXPORT_CSV;
        break;
    case 'f':
        listFlagging = getFlaggingOrDefault(arg);
        break;
    case 'g':
        listFinished = getFinishedOrDefault(arg);
        break;
    case 'H':
        cout << readme;
        break;
    }
}

void Config::listScores() {
    // TODO 'other' setups may produce too high 3BV/s etc and break layout

    string fullpath = cacheDirectory + "scores.dat";

    vector<Score> scores = loadScores(fullpath);

    if (scores.size() == 0) { // no scores in score file
        if (scoreListType != List::EXPORT_CSV)
            cout << "No high scores yet." << endl;
    } else {
        if (scoreListType != List::EXPORT_CSV) {
            // info about filter and sort
            cout << endl << "Displaying scores." << endl;
            cout << setw(16) << left << "Sorted by: ";

            ScoreCmpFunc *compareFunc;
            switch (scoreListType) {
            case List::TIME:
                cout << "time" << endl;
                compareFunc = compareByTime;
                break;
            case List::BBBV:
                cout << "3BV/s" << endl;
                compareFunc = compareBy3BVs;
                break;
            case List::IOE:
                cout << "IOE" << endl;
                compareFunc = compareByIOE;
                break;
            }

            cout << setw(16) << left << "Flagging: ";
            switch (listFlagging) {
            case Flagging::BOTH:
                cout << "all" << endl;
                break;
            case Flagging::FLAGGING:
                cout << "flagging only" << endl;
                break;
            case Flagging::NO_FLAGGING:
                cout << "non-flagging only" << endl;
                break;
            }

            cout << setw(16) << left << "Won: ";
            switch (listFinished) {
            case Finished::BOTH:
                cout << "all" << endl;
                break;
            case Finished::FINISHED:
                cout << "won only" << endl;
                break;
            case Finished::UNFINISHED:
                cout << "lost only" << endl;
                break;
            }

            cout << setw(16) << left << "Difficulty: ";

            switch (getDifficulty()) {
            case -1:
                if (baseDifficulty != 0) {
                    cout << player.field.width << "x" << player.field.height << ", "
                    << player.field.mineCount << " mines" << endl;
                } else {
                    cout << "beginner, intermediate, expert, beginner classic" << endl;
                }
                break;
            case 1: cout << "beginner only" << endl; break;
            case 2: cout << "intermediate only" << endl; break;
            case 3: cout << "expert only" << endl; break;
            case 4: cout << "beginner classic only" << endl; break;
            }

            cout << setw(16) << left << "Square size: ";
            if (squareSize != 0)
                cout << squareSize << endl;
            else
                cout << "all" << endl;

            cout << setw(16) << left << "Player name: ";
            if (player.field.playerName == "")
                cout << "all" << endl;
            else
                cout << player.field.playerName << endl;

            cout << setw(16) << left << "Count: ";
            if (scoreListLength != 0)
                cout << scoreListLength << endl;
            else
                cout << "all" << endl;

            cout << endl;

            sortScores(scores, compareFunc);
        }

        vector<Score> filteredScores =
          filterScores(scores, listFlagging, listFinished, player.field.playerName);

        //    cout<<"count="<<count<<endl;

        displayScores(filteredScores, scoreListLength,
                      scoreListType == List::EXPORT_CSV);

        cout << endl;
    }
}
