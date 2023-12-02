#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iostream>
#include <sys/ioctl.h>

#include <unistd.h>

using namespace std;

#define MAX_WIDTH 100
#define MAX_HEIGHT 100


#define BORDER_WIDTH 10
#define DISPLAY_BORDER_WIDTH 4
#define FIELD_X 10
#define FIELD_Y 48


#define MAX_HS 20
#define SCORE_FILE_VERSION 5

#define MAX_NAME_LENGTH 20

extern char readme[];

const char* ordinalNumberSuffix(int);

unsigned int terminalWidth();

bool outputLine(string,int);

bool isValidName(const string&);

// Bit flags
enum class Flagging : int {
    FLAGGING = 1,
    NO_FLAGGING = 2,
    BOTH = 3,
};
static inline Flagging getFlaggingOrDefault(int value) {
    switch (value) {
        case 1: return Flagging::FLAGGING;
        case 2: return Flagging::NO_FLAGGING;
        default: return Flagging::BOTH;
    }
}
enum class Finished : int {
    FINISHED = 1,
    UNFINISHED = 2,
    BOTH = 3,
};
static inline Finished getFinishedOrDefault(int value) {
    switch (value) {
        case 1: return Finished::FINISHED;
        case 2: return Finished::UNFINISHED;
        default: return Finished::BOTH;
    }
}

#endif

