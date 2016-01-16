class Field {
public:

    int width, height, mineCount;
    bool mine[MAX_WIDTH][MAX_HEIGHT];
    int state[MAX_WIDTH][MAX_HEIGHT];  // 0-8 - adjacent mine count for revealed field, 9 - not revealed, 10 - flag

    bool isMine(int x, int y);

    void setMine(int x, int y);

    void placeMines(int firstClickX, int firstClickY);

    void init();

    void revealAround(int squareX, int squareY);

    void revealSquare(int squareX, int squareY);

    void squareClicked(int squareX, int squareY);

    void checkAndRevealAround(int squareX,int squareY);

    int calculateRemainingMines();
    
    void checkValues();

    void click(int, int, int);
};
