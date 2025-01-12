#include <string>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <sstream>

#include "board.h"
#include "boardmanage.h"

using namespace std;

termios original;

//recover terminal 
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

// set terminal to raw mode
void enableRawMode() {
    termios terminal_config;
    tcgetattr(STDIN_FILENO, &terminal_config);
    atexit(disableRawMode);

    //save orinigal config for disable raw mode
    original = terminal_config;
    
    cfmakeraw(&terminal_config);
    terminal_config.c_cc[VMIN] = 1;
    terminal_config.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_config);
}

bool getIsGameclear(shared_ptr<Board> board) {
    bool isClear = true;
    for(int idx : *board->mineIdxList) {
        if (!isClear) break;
        isClear &= board->cells[idx].flagColor == board->cells[idx].mineColor;
    }
    return isClear && board->remainCellNum<=0;
}

void gameOver(shared_ptr<Board> board, Cursor cursor) {
    for(int idx: (*board->mineIdxList)) {
        if (board->cells[idx].isFlag) {
            board->cells[idx].flagColor = board->cells[idx].mineColor;
        } else {
            board->cells[idx].isOpened = true;
        }
    }
    system("clear");
    printGameView(board, cursor, false, false);
    cout << "GAMEOVER!\n\r";
}

void gameClear(shared_ptr<Board> board, Cursor cursor) {
    for (int i=0; i<CELL_NUM*CELL_NUM; i++) {
        board->cells[i].isOpened = true;
    }
    system("clear");
    printGameView(board, cursor, false, false);
    cout << "CONGRATULATIONS!\n\r";
}

int main(void) {
    char key;
    bool isLoop = true, isFirst = true, isCancel = false, isHelp = false;

    Cursor cursor = {0, 0};
    shared_ptr<Board> board = initBoard();

    enableRawMode();

    while(isLoop) {
        system("clear");
        printGameView(board, cursor, isHelp, isCancel);
        if (isCancel) cout << "Do you want to cancel the game? (y/n)\n\r";
        key = getchar();
        
        //for help
        if (key=='h' || isHelp) {
            isHelp = !isHelp;
            continue;
        }

        //for quit
        if (isCancel){
            if (key=='y') exit(0);
            if (key=='n') isCancel = false;
            continue;
        }

        switch(key){
            case 'w':
                cursor.x = (cursor.x-1+CELL_NUM)%CELL_NUM;
                break;
            case 's':
                cursor.x = (cursor.x+1)%CELL_NUM;
                break;
            case 'a':
                cursor.y = (cursor.y-1+CELL_NUM)%CELL_NUM;
                break;
            case 'd':
                cursor.y = (cursor.y+1)%CELL_NUM;
                break;
            case ' ':
                if (isFirst) {
                    setCells(board, cursor);
                    isFirst = false;
                }
                if (openCell(board, cursor)) {
                    //if open mine cell
                    gameOver(board, cursor);
                    isLoop = false;
                } else if (getIsGameclear(board)) {
                    gameClear(board, cursor);
                    isLoop = false;
                }
                break;
            case 'i':
                setFlag(board, cursor, Color::RED);
                if (getIsGameclear(board)) {
                    gameClear(board, cursor);
                    isLoop = false;
                    break;
                }
                break;
            case 'o':
                setFlag(board, cursor, Color::GREEN);
                if (getIsGameclear(board)) {
                    gameClear(board, cursor);
                    isLoop = false;
                    break;
                }
                break;
            case 'p':
                setFlag(board, cursor, Color::BLUE);
                if (getIsGameclear(board)) {
                    gameClear(board, cursor);
                    isLoop = false;
                    break;
                }
                break;
            case 'c':
                isCancel = true;
                break;
        }
    }

    return 0;
}
