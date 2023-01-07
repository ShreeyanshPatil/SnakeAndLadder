#include "../inc/SnakeNLadderDataTypes.h"
#include "../inc/FileManager.h"
#include "../inc/DoublyList.h"
// cl /I SnakeNLadderDataTypes.h /I FileManager.h /I DoublyList.h src\main.cpp src\FileManager.cpp src\DoublyList.cpp /link /out:snake.exe

FILE *gpFile = NULL;
HANDLE hThread = NULL;
struct Node *pHeadNode = NULL;
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
CONDITION_VARIABLE ConditionVar;
CRITICAL_SECTION BufferLock;
HWND ghwnd;
int winHeight;
int winWidth;

static struct Shapes shape;
DWORD WINAPI musicThreadFunc(LPVOID lpParam)
{
    InvalidateRect(ghwnd, NULL, TRUE);
    while (1)
    {
        EnterCriticalSection(&BufferLock);
        SleepConditionVariableCS(&ConditionVar, &BufferLock, INFINITE);
        LeaveCriticalSection(&BufferLock);
        PlaySound("sad.wav", NULL, SND_SYNC | SND_LOOP | SND_FILENAME);
        fprintf(gpFile, "playMusic thread wake up\n");
    }
}

void FullScreen(void)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    DWORD dwStyle; // To hold the style of window

    dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
    WINDOWPLACEMENT wPrev = {sizeof(WINDOWPLACEMENT)}; // to store the current window placement
    if (dwStyle & WS_OVERLAPPEDWINDOW)
    {
        if (GetWindowPlacement(ghwnd, &wPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
        {
            SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right - mi.rcMonitor.left), (mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
            ShowCursor(false);
        }
    }
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    static TCHAR szWindowName[] = TEXT("SnakeNLadder");
    static TCHAR szAppName[] = TEXT("SnakeNLadder App");
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    HWND hWnd = NULL;
    HBRUSH hBrush = NULL;
    HCURSOR hCursor = NULL;
    HICON hIcon = NULL;
    HICON hIconSm = NULL;
    WNDCLASSEX wndEx;
    MSG msg;
    ZeroMemory(&wndEx, sizeof(WNDCLASSEX));
    ZeroMemory(&msg, sizeof(MSG));
    // Open the log file to write debug messages
    if (fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to create a debug file"), TEXT("Error"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "File Opened successfully\n");
    }

    pHeadNode = (struct Node *)malloc(sizeof(struct Node));
    if (pHeadNode != NULL)
    {
        pHeadNode->data.left = -1;
        pHeadNode->data.right = -1;
        pHeadNode->data.top = -1;
        pHeadNode->data.bottom = -1;
        pHeadNode->no = -1;
        pHeadNode->pNext = pHeadNode;
        pHeadNode->pPrev = pHeadNode;
        pHeadNode->color.blue = -1;
        pHeadNode->color.green = -1;
        pHeadNode->color.red = -1;
    }
    else
    {
        fprintf(gpFile, "Failed to create HeadNode\n");
    }

    hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    assert(hBrush);

    hCursor = LoadCursor(NULL, IDC_ARROW);
    assert(hCursor);

    hIcon = LoadIcon(NULL, IDI_APPLICATION);
    assert(hIcon);

    hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    assert(hIconSm);

    wndEx.cbSize = sizeof(WNDCLASSEX);
    wndEx.cbClsExtra = 0;
    wndEx.cbWndExtra = 0;
    wndEx.hbrBackground = hBrush;
    wndEx.hCursor = hCursor;
    wndEx.hIcon = hIcon;
    wndEx.hIconSm = hIconSm;
    wndEx.hInstance = hInstance;
    wndEx.lpfnWndProc = WndProc;
    wndEx.lpszClassName = szAppName;
    wndEx.lpszMenuName = NULL;
    wndEx.style = CS_HREDRAW | CS_VREDRAW;

    ATOM bRet = RegisterClassEx(&wndEx);

    assert(bRet);

    hWnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, szWindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    assert(hWnd != NULL);
    ghwnd = hWnd;
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    FullScreen();
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (msg.wParam);
}

void DrawPlayers(HWND hwnd, struct Shapes *shapePtr)
{
    HDC hdc = GetDC(hwnd);
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, RGB(0, 0, 0));
    Ellipse(hdc, shapePtr->left, shapePtr->top, shapePtr->right, shapePtr->bottom);
    ReleaseDC(hwnd, hdc);
}
void DrawSnakes(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    Graphics graphics(hdc);

    Point snakeone7to17pt1[] = {
        Point(430, 630),
        Point(470, 625),
        Point(510, 620),
        Point(550, 625),
        Point(590, 650),
        Point(640, 670),
        Point(690, 690),
        Point(750, 730),
        Point(770, 720),
    };
    Point snakeone7to17pt2[] = {
        Point(430, 645),
        Point(470, 640),
        Point(510, 635),
        Point(550, 640),
        Point(590, 665),
        Point(640, 680),
        Point(690, 700),
        Point(750, 735),
        Point(770, 720)

    };

    Pen blackPen(Color(255, 0, 0, 0), 2);
    graphics.DrawCurve(&blackPen, snakeone7to17pt1, 9, 0.6f);
    graphics.DrawCurve(&blackPen, snakeone7to17pt2, 9, 0.6f);

    Point snakeone34to54pt1[] = {
        Point(790, 325),
        Point(800, 330),
        Point(850, 325),
        Point(890, 360),
        Point(870, 390),
        Point(830, 400),
        Point(790, 420),
        Point(760, 450),
        Point(740, 510)};

    Point snakeone34to54pt2[] = {
        Point(790, 340),
        Point(800, 345),
        Point(850, 340),
        Point(870, 375),
        Point(810, 390),
        Point(790, 400),
        Point(780, 410),
        Point(760, 440),
        Point(740, 510),

    };

    graphics.DrawCurve(&blackPen, snakeone34to54pt1, 9, 0.6f);
    graphics.DrawCurve(&blackPen, snakeone34to54pt2, 9, 0.6f);

    Point snakeone60to64pt1[] = {
        Point(380, 240),
        Point(400, 260),
        Point(370, 230),
        Point(340, 220),

        Point(300, 230),
        Point(220, 260),
        Point(100, 350),
        Point(40, 320),
    };
    Point snakeone60to64pt2[] = {
        Point(380, 260),
        Point(400, 280),
        Point(370, 250),
        Point(340, 240),

        Point(300, 250),
        Point(220, 280),
        Point(100, 360),
        Point(40, 320),
    };
    graphics.DrawCurve(&blackPen, snakeone60to64pt1, 8, 0.6f);
    graphics.DrawCurve(&blackPen, snakeone60to64pt2, 8, 0.6f);

    Point snakeone73to93pt1[] = {
        Point(900, 30),
        Point(950, 20),
        Point(970, 20),
        Point(980, 20),

        Point(1010, 30),
        Point(1020, 40),

        Point(1020, 50),
        Point(1020, 70),
        Point(930, 150),
        Point(860, 170),
        Point(870, 200)};
    Point snakeone73to93pt2[] = {
        Point(900, 45),
        Point(960, 35),
        Point(990, 40),
        Point(1000, 55),

        Point(1010, 70),
        Point(990, 90),
        Point(980, 100),

        Point(970, 110),
        Point(940, 130),
        Point(860, 160),
        Point(870, 200)};

    graphics.DrawCurve(&blackPen, snakeone73to93pt1, 8, 0.6f);
    graphics.DrawCurve(&blackPen, snakeone73to93pt2, 4, 0.6f);

    Point snakeone75to95pt1[] = {
        Point(650, 20),
        Point(600, 30),

        Point(580, 40),
        Point(560, 50),

        Point(540, 80),
        Point(580, 120),

        Point(600, 140),
        Point(630, 170),
        Point(660, 180),
    };

    Point snakeone75to95pt2[] = {
        Point(650, 35),
        Point(600, 45),

        Point(580, 55),
        Point(560, 65),

        Point(570, 95),
        Point(590, 110),

        Point(610, 120),
        Point(610, 130),
        Point(620, 180),
    };
    graphics.DrawCurve(&blackPen, snakeone75to95pt1, 9, 0.6f);
    graphics.DrawCurve(&blackPen, snakeone75to95pt2, 8, 0.6f);

    ReleaseDC(hwnd, hdc);
}
void DrawLadders(HWND hwnd)
{
    struct Node *pnode = pHeadNode->pNext;
    struct Node *pnodelastladderstart = NULL;
    int oldx, oldy;

    HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));

    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    int ladderEnd = pnode->ladderpos.end;
    HDC hdc = GetDC(hwnd);
    SelectObject(hdc, pen);
    // Ladder 4 to 14 : Start
    MoveToEx(hdc, pnode->data.left + 10, pnode->data.bottom - 70, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 30, pnode->data.bottom - 50);

    MoveToEx(hdc, oldx + 20, oldy - 30, NULL);
    LineTo(hdc, pnode->data.left + 40, pnode->data.bottom - 10);

    MoveToEx(hdc, oldx + 50, oldy - 75, NULL);
    LineTo(hdc, oldx + 40, pnode->data.bottom + 45);

    MoveToEx(hdc, oldx + 100, oldy - 85, NULL);
    LineTo(hdc, oldx + 90, pnode->data.bottom + 35);

    MoveToEx(hdc, oldx + 150, oldy - 95, NULL);
    LineTo(hdc, oldx + 140, pnode->data.bottom + 27);

    MoveToEx(hdc, oldx + 200, oldy - 101, NULL);
    LineTo(hdc, oldx + 190, pnode->data.bottom + 20);

    MoveToEx(hdc, oldx + 250, oldy - 102, NULL);
    LineTo(hdc, oldx + 240, pnode->data.bottom + 17);

    MoveToEx(hdc, oldx + 300, oldy - 111, NULL);
    LineTo(hdc, oldx + 290, pnode->data.bottom + 8);

    MoveToEx(hdc, oldx + 350, oldy - 118, NULL);
    LineTo(hdc, oldx + 340, pnode->data.bottom + 2);
    // fprintf(gpFile, "DrawLadders no after firsy** = %d, first=%d\n", pnode->no, pnodelastladderstart->no);
    //  Ladder 9 to 31 : Start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 90, pnode->data.bottom - 30, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 65, pnode->data.bottom - 10);

    MoveToEx(hdc, oldx + 40, oldy - 40, NULL);
    LineTo(hdc, pnode->data.left + 20, pnode->data.bottom - 20);

    MoveToEx(hdc, oldx + 60, oldy - 75, NULL);
    LineTo(hdc, oldx + 110, pnode->data.bottom + 160);

    MoveToEx(hdc, oldx + 80, oldy - 120, NULL);
    LineTo(hdc, oldx + 130, pnode->data.bottom + 120);

    MoveToEx(hdc, oldx + 100, oldy - 165, NULL);
    LineTo(hdc, oldx + 150, pnode->data.bottom + 75);

    MoveToEx(hdc, oldx + 120, oldy - 205, NULL);
    LineTo(hdc, oldx + 170, pnode->data.bottom + 35);
    // Ladder 9 to 31 : End
    // Ladder from 21 to 42 : start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 15, pnode->data.bottom - 50, NULL);

    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 30, pnode->data.bottom - 20);

    MoveToEx(hdc, oldx + 50, oldy - 25, NULL);
    LineTo(hdc, pnode->data.left + 75, pnode->data.bottom - 10);

    MoveToEx(hdc, oldx + 80, oldy - 50, NULL);
    LineTo(hdc, pnode->data.left - 70, pnode->data.bottom + 65);

    MoveToEx(hdc, oldx + 120, oldy - 80, NULL);
    LineTo(hdc, pnode->data.left - 30, pnode->data.bottom + 30);

    MoveToEx(hdc, oldx + 160, oldy - 130, NULL);
    LineTo(hdc, pnode->data.left + 10, pnode->data.bottom - 10);

    // Ladder from 21 to 42 : end
    // Ladder from 28 to 84 : Start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 10, pnode->data.bottom - 20, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 10, pnode->data.bottom - 20);

    MoveToEx(hdc, oldx + 40, oldy - 50, NULL);
    LineTo(hdc, pnode->data.left + 40, pnode->data.bottom - 50);

    MoveToEx(hdc, oldx - 10, oldy - 40, NULL);
    LineTo(hdc, pnode->data.left + 500, pnode->data.bottom + 390);

    MoveToEx(hdc, oldx - 45, oldy - 70, NULL);
    LineTo(hdc, pnode->data.left + 470, pnode->data.bottom + 360);

    MoveToEx(hdc, oldx - 80, oldy - 105, NULL);
    LineTo(hdc, pnode->data.left + 435, pnode->data.bottom + 325);

    MoveToEx(hdc, oldx - 115, oldy - 140, NULL);
    LineTo(hdc, pnode->data.left + 400, pnode->data.bottom + 290);

    MoveToEx(hdc, oldx - 150, oldy - 175, NULL);
    LineTo(hdc, pnode->data.left + 365, pnode->data.bottom + 255);

    MoveToEx(hdc, oldx - 185, oldy - 210, NULL);
    LineTo(hdc, pnode->data.left + 330, pnode->data.bottom + 220);

    MoveToEx(hdc, oldx - 220, oldy - 240, NULL);
    LineTo(hdc, pnode->data.left + 295, pnode->data.bottom + 190);

    MoveToEx(hdc, oldx - 255, oldy - 270, NULL);
    LineTo(hdc, pnode->data.left + 260, pnode->data.bottom + 160);

    MoveToEx(hdc, oldx - 290, oldy - 300, NULL);
    LineTo(hdc, pnode->data.left + 230, pnode->data.bottom + 130);

    MoveToEx(hdc, oldx - 320, oldy - 335, NULL);
    LineTo(hdc, pnode->data.left + 195, pnode->data.bottom + 100);

    MoveToEx(hdc, oldx - 355, oldy - 370, NULL);
    LineTo(hdc, pnode->data.left + 165, pnode->data.bottom + 65);

    MoveToEx(hdc, oldx - 390, oldy - 400, NULL);
    LineTo(hdc, pnode->data.left + 125, pnode->data.bottom + 30);

    MoveToEx(hdc, oldx - 425, oldy - 430, NULL);
    LineTo(hdc, pnode->data.left + 85, pnode->data.bottom - 5);

    // Ladder from 28 to 84 : End

    // Ladder from 51 to 67 : Start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 10, pnode->data.bottom - 20, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 10, pnode->data.bottom - 30);

    MoveToEx(hdc, oldx + 25, oldy - 55, NULL);
    LineTo(hdc, pnode->data.left + 35, pnode->data.bottom - 65);

    MoveToEx(hdc, oldx + 10, oldy - 60, NULL);
    LineTo(hdc, pnode->data.left + 350, pnode->data.bottom + 50);

    MoveToEx(hdc, oldx - 45, oldy - 70, NULL);
    LineTo(hdc, pnode->data.left + 290, pnode->data.bottom + 35);

    MoveToEx(hdc, oldx - 100, oldy - 85, NULL);
    LineTo(hdc, pnode->data.left + 230, pnode->data.bottom + 20);

    MoveToEx(hdc, oldx - 155, oldy - 100, NULL);
    LineTo(hdc, pnode->data.left + 170, pnode->data.bottom + 5);

    MoveToEx(hdc, oldx - 215, oldy - 115, NULL);
    LineTo(hdc, pnode->data.left + 115, pnode->data.bottom - 5);

    MoveToEx(hdc, oldx - 270, oldy - 130, NULL);
    LineTo(hdc, pnode->data.left + 60, pnode->data.bottom - 15);
    // Ladder from 51 to 67 : End

    // Ladder from 72 to 91 : Start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 30, pnode->data.bottom - 40, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 20, pnode->data.bottom - 50);

    MoveToEx(hdc, oldx + 70, oldy - 30, NULL);
    LineTo(hdc, pnode->data.left + 60, pnode->data.bottom - 40);

    MoveToEx(hdc, oldx + 85, oldy - 50, NULL);
    LineTo(hdc, pnode->data.left - 75, pnode->data.bottom + 90);

    MoveToEx(hdc, oldx + 110, oldy - 90, NULL);
    LineTo(hdc, pnode->data.left - 50, pnode->data.bottom + 50);

    MoveToEx(hdc, oldx + 135, oldy - 130, NULL);
    LineTo(hdc, pnode->data.left - 25, pnode->data.bottom + 10);

    MoveToEx(hdc, oldx + 165, oldy - 170, NULL);
    LineTo(hdc, pnode->data.left, pnode->data.bottom - 30);

    //  Ladder from 72 to 91 : END

    // Ladder from 80 to 99 : Start
    pnode = pnodelastladderstart->pNext;
    while (pnode->ladderpos.start != 1)
    {
        pnode = pnode->pNext;
    }
    pnodelastladderstart = pnode;
    ladderEnd = pnode->ladderpos.end;
    MoveToEx(hdc, pnode->data.left + 10, pnode->data.bottom - 40, NULL);
    oldx = pnode->data.left;
    oldy = pnode->data.bottom;
    while (pnode->no != ladderEnd)
    {
        pnode = pnode->pNext;
    }
    LineTo(hdc, pnode->data.left + 10, pnode->data.bottom - 50);

    MoveToEx(hdc, oldx + 45, oldy - 20, NULL);
    LineTo(hdc, pnode->data.left + 40, pnode->data.bottom - 30);

    MoveToEx(hdc, oldx + 65, oldy - 50, NULL);
    LineTo(hdc, pnode->data.left - 95, pnode->data.bottom + 90);

    MoveToEx(hdc, oldx + 100, oldy - 90, NULL);
    LineTo(hdc, pnode->data.left - 70, pnode->data.bottom + 50);

    MoveToEx(hdc, oldx + 120, oldy - 130, NULL);
    LineTo(hdc, pnode->data.left - 40, pnode->data.bottom + 10);

    MoveToEx(hdc, oldx + 140, oldy - 170, NULL);
    LineTo(hdc, pnode->data.left - 10, pnode->data.bottom - 30);

    // Ladder from 80 to 99 : End*/
    DeleteObject(pen);
    ReleaseDC(hwnd, hdc);
}
void DeleteNodes()
{
}
void DrawSnakeLadderBackground(HDC hdc, HFONT hFont, RECT rs)
{
    struct Node *temp = pHeadNode->pNext;
    char buf[5];
    memset(buf, 0, sizeof(buf));
    for (; temp->no != -1; temp = temp->pNext)
    {
        SelectObject(hdc, GetStockObject(DC_BRUSH));
        SetDCBrushColor(hdc, RGB(temp->color.red, temp->color.green, temp->color.blue));
        Rectangle(hdc, temp->data.left, temp->data.top, temp->data.right, temp->data.bottom);
        SelectObject(hdc, hFont);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetRect(&rs, temp->data.left, temp->data.top, temp->data.right, temp->data.bottom);
        snprintf(buf, 3, "%d", temp->no);
        DrawText(hdc, TEXT(buf), -1, &rs, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
    }
}
struct Node *playerpos = NULL;
int blockno = 0;

void DisplayDigit(HDC hdc, int iNumber)
{
    static BOOL fSevenSegment[7][7] = {
        1, 1, 1, 0, 1, 1, 1, // 0
        0, 0, 1, 0, 0, 1, 0, // 1
        1, 0, 1, 1, 1, 0, 1, // 2
        1, 0, 1, 1, 0, 1, 1, // 3
        0, 1, 1, 1, 0, 1, 0, // 4
        1, 1, 0, 1, 0, 1, 1, // 5
        1, 1, 0, 1, 1, 1, 1, // 6
    };
    static POINT ptSegment[7][6] = {
        7, 6, 11, 2, 31, 2, 35, 6, 31, 10, 11, 10,
        6, 7, 10, 11, 10, 31, 6, 35, 2, 31, 2, 11,
        36, 7, 40, 11, 40, 31, 36, 35, 32, 31, 32, 11,
        7, 36, 11, 32, 31, 32, 35, 36, 31, 40, 11, 40,
        6, 37, 10, 41, 10, 61, 6, 65, 2, 61, 2, 41,
        36, 37, 40, 41, 40, 61, 36, 65, 32, 61, 32, 41,
        7, 66, 11, 62, 31, 62, 35, 66, 31, 70, 11, 70};
    int iSeg;
    for (iSeg = 0; iSeg < 7; iSeg++)
        if (fSevenSegment[iNumber][iSeg])
            Polygon(hdc, ptSegment[iSeg], 6);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    static HFONT hFont;
    static RECT rs;
    static int x = 0;
    static int y = 0;
    static int x2;
    static int y2;
    int lower = 1, upper = 6;
    static int iNumber = 0;
    DWORD dwThreadId;
    static struct Node *temp = NULL;
    static HBRUSH hBrushRed;
    static int one = 1;
    switch (uMsg)
    {
    case WM_CREATE:
    {
        hFont = CreateFont(35, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));

        x = 0;
        int xadjust = 120;
        int yadjust = 76;
        y = (winWidth + 76) * 9;
        x2 = (winWidth + xadjust);
        y2 = (winWidth + 76) * 10;
        int i;
        int k = 0;
        int j = 1;
        int m = 0;
        int n = 9;

        int line = 1;
        int colorIndex = 0;

        playerpos = pHeadNode->pNext;
        DWORD dwThreadId;

        InitializeConditionVariable(&ConditionVar);
        InitializeCriticalSection(&BufferLock);
        hThread = CreateThread(NULL, 0, musicThreadFunc, NULL, 0, &dwThreadId);
        int **colorMap = ReadColorMap();
        for (i = 1, j = 1; i <= 100; i++, j++)
        {
            struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
            if (newNode != NULL)
            {
                newNode->data.left = x;
                newNode->data.top = y;
                newNode->data.right = x2;
                newNode->data.bottom = y2;
                newNode->color.red = colorMap[line - 1][colorIndex + 0];
                newNode->color.green = colorMap[line - 1][colorIndex + 1];
                newNode->color.blue = colorMap[line - 1][colorIndex + 2];
                fprintf(gpFile, "%d, %d, %d\n", newNode->color.red, newNode->color.green, newNode->color.blue);
                colorIndex += 3;
                newNode->no = i;
                newNode->ladderpos.end = 0;
                if (i == 4 || i == 9 || i == 21 || i == 28 || i == 51 || i == 72 || i == 80)
                {
                    newNode->ladderpos.start = 1;
                    if (i == 4)
                    {
                        newNode->ladderpos.end = 14;
                    }
                    else if (i == 9)
                    {
                        newNode->ladderpos.end = 31;
                    }
                    else if (i == 21)
                    {
                        newNode->ladderpos.end = 42;
                    }
                    else if (i == 28)
                    {
                        newNode->ladderpos.end = 84;
                    }
                    else if (i == 51)
                    {
                        newNode->ladderpos.end = 67;
                    }
                    else if (i == 72)
                    {
                        newNode->ladderpos.end = 91;
                    }
                    else if (i == 80)
                    {
                        newNode->ladderpos.end = 99;
                    }
                    else
                    {
                        newNode->ladderpos.end = 0;
                    }
                }
                else
                {
                    newNode->ladderpos.start = 0;
                }

                if (colorIndex > 29)
                {
                    colorIndex = 0;
                }
                GenericInsert(pHeadNode->pPrev, newNode, pHeadNode);
                if (j >= 10)
                {
                    y -= (winWidth + yadjust);
                    y2 -= ((winWidth + yadjust));
                    j = 0;
                    line++;
                    if (line % 2 == 0)
                    {
                        x += winWidth + xadjust;
                        x2 += winWidth + xadjust;
                    }
                    else
                    {
                        x -= winWidth + xadjust;
                        x2 -= winWidth + xadjust;
                    }
                }
                if (line % 2 != 0)
                {
                    x += winWidth + xadjust;
                    x2 += winWidth + xadjust;
                }
                else
                {
                    x -= winWidth + xadjust;
                    x2 -= winWidth + xadjust;
                }
            }
            else
            {
                fprintf(gpFile, "Failed to create newNode\n");
            }
            k++;
        }
        hBrushRed = CreateSolidBrush(RGB(255, 0, 0));
        // Done with use of colormap so delete the memory
        for (i = 0; i < MAX_LINE; i++)
        {
            free(colorMap[i]);
        }
        free(colorMap);
        // srand(time(0));
    }
    break;

    case WM_SIZE:
        winHeight = LOWORD(lParam);
        winWidth = HIWORD(lParam);

        break;
    case WM_PAINT:

        hdc = BeginPaint(hwnd, &ps);
        DrawSnakeLadderBackground(hdc, hFont, rs);
        temp = pHeadNode->pNext;
        if (one)
        {
            shape.left = temp->data.left + 20;
            shape.top = temp->data.top + 50;
            shape.right = temp->data.right - 70;
            shape.bottom = temp->data.bottom;
            one = 0;
            playerpos = temp;
        }
        DrawPlayers(hwnd, &shape);
        DrawSnakes(hwnd);
        DrawLadders(hwnd);
        SetMapMode(hdc, MM_ISOTROPIC);
        SetWindowExtEx(hdc, 276, 72, NULL);
        SetViewportExtEx(hdc, winWidth, winHeight, NULL);
        SetWindowOrgEx(hdc, 138, 86, NULL);
        SetViewportOrgEx(hdc, winWidth / 2, winWidth / 2, NULL);
        OffsetWindowOrgEx(hdc, -440, -10, NULL);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        SelectObject(hdc, hBrushRed);
        DisplayDigit(hdc, iNumber % 7);

        EndPaint(hwnd, &ps);
        break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            break;
        case 0x52:

            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case VK_UP:
        {
            iNumber = (rand() % (upper - lower + 1)) + lower;
            fprintf(gpFile, "VK_UP presssed1 = %d\n", playerpos->no);

            fprintf(gpFile, "left = %d top=%d right=%d bottom=%d", shape.left, shape.top, shape.right, shape.bottom);

            int counter = iNumber;
            fprintf(gpFile, "VK_UP presssed iNumber= %d\n", iNumber);
            while (counter != 0)
            {
                playerpos = playerpos->pNext;
                shape.left = playerpos->data.left + 20;
                shape.right = playerpos->data.right - 70;
                shape.top = playerpos->data.top + 50;
                shape.bottom = playerpos->data.bottom;
                counter = counter - 1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            if (playerpos->ladderpos.start == 1)
            {
                fprintf(gpFile, "VK_UP presssed2 = %d\n", playerpos->no);
                WakeAllConditionVariable(&ConditionVar);
                int ladderEnd = playerpos->ladderpos.end;
                while (playerpos->no != ladderEnd)
                {
                    playerpos = playerpos->pNext;
                    fprintf(gpFile, "VK_UP presssed = %d\n", playerpos->no);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                shape.left = playerpos->data.left + 20;
                shape.right = playerpos->data.right - 70;
                shape.top = playerpos->data.top + 50;
                shape.bottom = playerpos->data.bottom;
            }
        }

        break;
        case VK_DOWN:
            WakeAllConditionVariable(&ConditionVar);
            break;
        }
        break;
    }

    case WM_DESTROY:
        DeleteNodes();
        PostQuitMessage(0);
        break;
    }
    return (DefWindowProc(hwnd, uMsg, wParam, lParam));
}