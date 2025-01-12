#ifndef BOARDMANAGE_H
#define BOARDMANAGE_H

#include <memory>
#include <string>
#include <vector>

#include "board.h"

bool isOutOfBounds(int x, int y);

std::string getInfoString(std::shared_ptr<Board> board);
std::string getNumberString(Cell cell);
std::string getCellString(Cell cell);
std::string getHelpString();

void printGameView(std::shared_ptr<Board> board, Cursor cursor, bool isHelp, bool isGameover);

std::vector<int> generateMineIdxList(int x, int y);
void setCells(std::shared_ptr<Board> board, Cursor cursor);

std::shared_ptr<Board> initBoard();

void operateMineNum(std::shared_ptr<Board> board, Color color, bool isIncrease);
void setFlag(std::shared_ptr<Board> board, Cursor cursor, Color color);

void openCellRecursive(std::shared_ptr<Board> board, int x, int y);
int openCell(std::shared_ptr<Board> board, Cursor cursor);

#endif