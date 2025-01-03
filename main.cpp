#include <string>
#include <iostream>
#include <memory>
#include <termios.h>
#include <unistd.h>

using namespace std;

#define CELL_NUM 7

typedef enum {
    MINE_NONE, MINE_RED, MINE_GREEN, MINE_BLUE
} MineType;

struct Cursor {
    int x;
    int y;
};

struct Cell {
    MineType mineType;
    //for not-mine
    bool isOpened;
    int mineNum;
    int mineColor;
    //for mine
    bool isFlagged;
};

struct Board{
    unique_ptr<Cursor> cursor;
    unique_ptr<vector<vector<Cell>>> cells;
};

// set terminal to raw mode
void enableRawMode() {
    termios terminal_config;
    tcgetattr(STDIN_FILENO, &terminal_config);
    cfmakeraw(&terminal_config);
    terminal_config.c_cc[VMIN] = 1;
    terminal_config.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_config);
}

string getNumber(int x, int y) {
    if (x==y) return "\x1b[7m-\x1b[0m";
    return "0";
}

void printBoard(shared_ptr<Board> board) {
    string str = "";
    for (int i = 0; i < CELL_NUM; i++) {
        str += "+";

        for (int j = 0; j < CELL_NUM; j++) {
            str += "---+";
        }

        str += "\n\r|";

        for (int j = 0; j < CELL_NUM; j++) {
            if (board->cursor->x == i && board->cursor->y == j) {
                str += " \x1b[7m0\x1b[0m |";
            } else {
                str += " - |";
            }

        }

        str += "\n\r";
    }

    str += "+";

    for (int j = 0; j < CELL_NUM; j++) {
        str += "---+";
    }

    cout << str << "\n\r";

}

shared_ptr<Board> initBoard() {
    unique_ptr<Cursor> cursor_ptr = make_unique<Cursor>();
    cursor_ptr->x = 0;
    cursor_ptr->y = 0;

    shared_ptr<Board> board_ptr = make_shared<Board>();
    board_ptr->cursor = std::move(cursor_ptr);
    return board_ptr;
}

int main(void) {
    char key;
    shared_ptr<Board> board = initBoard();

    enableRawMode();
    system("clear");

    while(1) {
        printBoard(board);
        key = getchar();
        if (key == 'c') break;
        switch(key){
            case 'w':
                board->cursor->x = (board->cursor->x-1+CELL_NUM)%CELL_NUM;
                system("clear");
                break;
            case 's':
                board->cursor->x = (board->cursor->x+1)%CELL_NUM;
                system("clear");
                break;
            case 'a':
                board->cursor->y = (board->cursor->y-1+CELL_NUM)%CELL_NUM;
                system("clear");
                break;
            case 'd':
                board->cursor->y = (board->cursor->y+1)%CELL_NUM;
                system("clear");
                break;
        }
    }

    return 0;
}
