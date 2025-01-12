#include <memory>
#include <iostream>
#include "board.h"

bool getIsGameclear(std::shared_ptr<Board> board) {
    bool isClear = true;
    for(int idx : *board->mineIdxList) {
        if (!isClear) break;
        isClear &= board->cells[idx].flagColor == board->cells[idx].mineColor;
    }
    return isClear && board->remainCellNum<=0;
}

void gameOver(std::shared_ptr<Board> board, Cursor cursor) {
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

void gameClear(std::shared_ptr<Board> board, Cursor cursor) {
    for (int i=0; i<CELL_NUM*CELL_NUM; i++) {
        board->cells[i].isOpened = true;
    }
    system("clear");
    printGameView(board, cursor, false, false);
    std::cout << "CONGRATULATIONS!\n\r";
}
