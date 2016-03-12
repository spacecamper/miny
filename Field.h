class Field {

    

public:


    short width, height;
    short mineCount;

    bool mine[MAX_WIDTH][MAX_HEIGHT];
    int state[MAX_WIDTH][MAX_HEIGHT];  // 0-8 - adjacent mine count for revealed field, 9 - not revealed, 10 - flag
    int val3BV;
    unsigned int effectiveClicks;
    unsigned int ineffectiveClicks;


    int calculate3BV();
    void floodFillMark(int [MAX_WIDTH][MAX_HEIGHT],int,int);
    void ffmProc(int [MAX_WIDTH][MAX_HEIGHT],int,int);
    bool isMine(int x, int y);

    void setMine(int x, int y);

    void placeMines(int firstClickX, int firstClickY);

    void init();

    void revealAround(int squareX, int squareY);

    void revealSquare(int squareX, int squareY);

    void squareClicked(int squareX, int squareY);

    bool adjacentMinesFlagged(int squareX,int squareY);

    int calculateRemainingMines();
    
    void checkValues();

    void click(int, int, int);

    int get3BV();

    void viewClicks();
    
    float getGameProgress();
};
