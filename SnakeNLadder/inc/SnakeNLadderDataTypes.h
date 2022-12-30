#ifndef _SNAKENLADDERDATATYPES_H
#define _SNAKENLADDERDATATYPES_H
#include <windows.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")

#define true 1
#define false 0
#define MAX_LINE 10
#define COLOR_COMP_PER_LINE 30 // R, G, B color component per line

CONDITION_VARIABLE ConditionVar;
CRITICAL_SECTION BufferLock;
struct Shapes
{
    int left;
    int top;
    int right;
    int bottom;
};
struct colorData
{
    int red;
    int green;
    int blue;
};

struct Ladder
{
    int start;
    int end;
};
struct Snake
{
    int start;
    int end;
};
struct Node
{
    struct Shapes data;
    int no;
    struct colorData color;
    struct Ladder ladderpos;
    struct Snake snakepos;
    struct Node *pNext;
    struct Node *pPrev;
};

int winHeight;
int winWidth;
char tmp[5];
extern FILE *gpFile;
extern HANDLE hThread;
static struct Shapes shape;
extern struct Node *pHeadNode;
HWND ghwnd;

#endif