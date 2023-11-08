#include "Config.h"

#include "scores.h"
#include <algorithm>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#endif

// TODO free allocated memory (after using scores from loadScores and
// filterScores is finished)

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

void Config::setDifficulty(int difficulty) {
    switch (difficulty) {
    case 0:
        player.field.height = 0;
        player.field.width = 0;
        player.field.mineCount = 0;
        break;
    case 1:
        player.field.height = 9;
        player.field.width = 9;
        player.field.mineCount = 10;
        break;
    case 2:
        player.field.height = 16;
        player.field.width = 16;
        player.field.mineCount = 40;
        break;
    case 3:
        player.field.height = 16;
        player.field.width = 30;
        player.field.mineCount = 99;
        break;
    case 4:
        player.field.height = 8;
        player.field.width = 8;
        player.field.mineCount = 10;
        break;
    }
}
int Config::getDifficulty() {
    if (player.field.width == 0 and player.field.height == 0 and
        player.field.mineCount == 0)
        return 0;
    else if (player.field.width == 9 and player.field.height == 9 and
             player.field.mineCount == 10)
        return 1;
    else if (player.field.width == 16 and player.field.height == 16 and
             player.field.mineCount == 40)
        return 2;
    else if (player.field.width == 30 and player.field.height == 16 and
             player.field.mineCount == 99)
        return 3;
    else if (player.field.width == 8 and player.field.height == 8 and
             player.field.mineCount == 10)
        return 4;
    else
        return -1;
}

void Config::handleOption(char option, char *arg, bool from_cli) {
    handleOption(option, arg == nullptr ? 0 : atoi(arg), from_cli);
    switch (option) {
    case 'n':
        if (strlen(optarg) < 20)
            strcpy(player.field.playerName, optarg);
        else
            strncpy(player.field.playerName, optarg, 20);
        break;
    case 'p':
        strcpy(replayFileName, cacheDirectory);
        strcat(replayFileName, arg);
        strcat(replayFileName, ".replay");
        playReplay = true;
        boolDrawCursor = true;
        break;
    case 'C': {
        int length = strlen(optarg);

        if (optarg[strlen(optarg) - 1] != '/')
            length++;

        if (length > 101) {
            cout << "Config directory path must be shorter than 100 characters. "
                            "Exiting."
                     << endl;
            exit(1);
        } else {
            defaultCacheDirectory = false;
            strcpy(cacheDirectory, optarg);
            if (optarg[strlen(optarg) - 1] != '/')
                strcat(cacheDirectory, "/");
        }
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
        setDifficulty(arg);
        if (!from_cli)
            cout << "Set difficulty to " << getDifficulty() << "." << endl;
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

    char fullpath[110];
    strcpy(fullpath, cacheDirectory);
    strcat(fullpath, "scores.dat");

    Score *scores;
    int count = loadScores(fullpath, &scores);

    if (count == 0) { // no scores in score file
        if (scoreListType != List::EXPORT_CSV)
            cout << "No high scores yet." << endl;
    } else {
        if (scoreListType != List::EXPORT_CSV) {
            // info about filter and sort
            cout << endl << "Displaying scores." << endl;
            cout << setw(16) << left << "Sorted by: ";

            int (*compareFunc)(const void *, const void *) = NULL;
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
            case Flagging::FLAGGING_ONLY:
                cout << "flagging only" << endl;
                break;
            case Flagging::NO_FLAGGING_ONLY:
                cout << "non-flagging only" << endl;
                break;
            }

            cout << setw(16) << left << "Won: ";
            switch (listFinished) {
            case Finished::BOTH:
                cout << "all" << endl;
                break;
            case Finished::FINISHED_ONLY:
                cout << "won only" << endl;
                break;
            case Finished::UNFINISHED_ONLY:
                cout << "lost only" << endl;
                break;
            }

            cout << setw(16) << left << "Difficulty: ";

            switch (getDifficulty()) {
            case -1:
                cout << player.field.width << "x" << player.field.height << ", "
                         << player.field.mineCount << " mines" << endl;
                break;
            case 0:
                cout << "beginner, intermediate, expert, beginner classic" << endl;
                break;
            case 1:
                cout << "beginner only" << endl;
                break;
            case 2:
                cout << "intermediate only" << endl;
                break;
            case 3:
                cout << "expert only" << endl;
                break;
            case 4:
                cout << "beginner classic only" << endl;
                break;
            }

            cout << setw(16) << left << "Square size: ";
            if (squareSize != 0)
                cout << squareSize << endl;
            else
                cout << "all" << endl;

            cout << setw(16) << left << "Player name: ";
            if (!strcmp(player.field.playerName, ""))
                cout << "all" << endl;
            else
                cout << player.field.playerName << endl;

            cout << setw(16) << left << "Count: ";
            if (scoreListLength != 0)
                cout << scoreListLength << endl;
            else
                cout << "all" << endl;

            cout << endl;

            qsort(scores, count, sizeof(Score), compareFunc);
        }

        Score *filteredScores;

        count = filterScores(scores, count, &filteredScores, (int)listFlagging,
                             (int)listFinished, player.field.width,
                             player.field.height, player.field.mineCount,
                             squareSize, player.field.playerName);

        //    cout<<"count="<<count<<endl;

        displayScores(filteredScores, count, scoreListLength,
                      scoreListType == List::EXPORT_CSV);

        cout << endl;
        free(scores);
    }
}
