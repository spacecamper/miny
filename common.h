#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <cstring>
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

void ordinalNumberSuffix(char *, int);

unsigned int terminalWidth();

bool outputLine(string,int);

bool isValidName(char *);

#endif

