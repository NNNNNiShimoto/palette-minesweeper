#include <memory>
#include "board.h"

bool getIsGameclear(std::shared_ptr<Board> board);
void gameOver(std::shared_ptr<Board> board, Cursor cursor);
void gameClear(std::shared_ptr<Board> board, Cursor cursor);
