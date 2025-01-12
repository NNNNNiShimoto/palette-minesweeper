#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <vector>

#define CELL_NUM 10
#define MINE_NUM 5

enum class Color {
    NONE    = 0b000,
    RED     = 0b100,
    GREEN   = 0b010,
    BLUE    = 0b001,
    YELLOW  = 0b110,
    MAGENTA = 0b101,
    CYAN    = 0b011,
    WHITE   = 0b111
};

inline Color operator|(Color a, Color b) {
    using T = std::underlying_type_t<Color>;
    return static_cast<Color>(static_cast<T>(a) | static_cast<T>(b));
}

struct Cursor {
    int x;
    int y;
};

struct Cell {
    Color mineColor; //R,G,B or none only
    bool isOpened;
    bool isFlag;
    Color flagColor; //R,G,B or none only
    int mineNumber; 
    Color mineNumberColor;
};

struct Board{
    std::unique_ptr<Cell[]> cells;
    std::unique_ptr<std::vector<int>> mineIdxList;
    int redMineNum;
    int greenMineNum;
    int blueMineNum;
    int remainCellNum;
};

#endif