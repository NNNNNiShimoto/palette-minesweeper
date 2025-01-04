#include <string>
#include <iostream>
#include <memory>
#include <termios.h>
#include <unistd.h>

using namespace std;

#define CELL_NUM 7
#define MINE_NUM 4

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

inline Color& operator|=(Color& a, Color b) {
    a = a | b;
    return a;
}

struct Cursor {
    int x;
    int y;
};

struct Cell {
    Color mineColor;//R,G,B or none only
    //for not-mine
    bool isOpened;
    int mineNum;
    Color numberColor;
    //for mine
    bool isFlagged;
};

struct Board{
    unique_ptr<Cursor> cursor;
    unique_ptr<Cell[]> cells;
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

string getPrintNumber(int n, Color color) {
    if (n==0) return " ";
    switch(color) {
        case Color::RED:
            return "\x1b[31m"+to_string(n)+"\x1b[39m";
        case Color::GREEN:
            return "\x1b[32m"+to_string(n)+"\x1b[39m";
        case Color::YELLOW:
            return "\x1b[33m"+to_string(n)+"\x1b[39m";
        case Color::BLUE:
            return "\x1b[34m"+to_string(n)+"\x1b[39m";
        case Color::MAGENTA:
            return "\x1b[35m"+to_string(n)+"\x1b[39m";
        case Color::CYAN:
            return "\x1b[36m"+to_string(n)+"\x1b[39m";
        default:
            return "\x1b[37m"+to_string(n)+"\x1b[39m";
    }
}

void printBoard(shared_ptr<Board> board) {
    string str = "info\n\r";
    for (int i = 0; i < CELL_NUM; i++) {
        str += "+";

        for (int j = 0; j < CELL_NUM; j++) {
            str += "---+";
        }

        str += "\n\r|";

        for (int j = 0; j < CELL_NUM; j++) {
            if (board->cells[i*CELL_NUM+j].mineColor==Color::NONE) {
                if (board->cursor->x == i && board->cursor->y == j) {
                    str += " \x1b[7m"+getPrintNumber(board->cells[i*CELL_NUM+j].mineNum, board->cells[i*CELL_NUM+j].numberColor)+"\x1b[0m |";
                } else {
                    str += " "+getPrintNumber(board->cells[i*CELL_NUM+j].mineNum, board->cells[i*CELL_NUM+j].numberColor)+" |";
                }
            } else {
                // if (board->cursor->x == i && board->cursor->y == j) {
                //     str += " \x1b[7m \x1b[0m |";
                // } else {
                //     str += "   |";
                // }
                // for after clear
                switch(board->cells[i*CELL_NUM+j].mineColor) {
                    case Color::RED:
                        str += " \x1b[31mX\x1b[39m |";
                        break;
                    case Color::GREEN:
                        str += " \x1b[32mX\x1b[39m |";
                        break;
                    case Color::BLUE:
                        str += " \x1b[34mX\x1b[39m |";
                        break;
                    default:
                        str += " X |";
                        break;
                }
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

unique_ptr<Cell[]> initCells() {
    unique_ptr<Cell[]> cells = make_unique<Cell[]>(CELL_NUM*CELL_NUM);
    
    //all cells initialize
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        cells[i].mineColor = Color::NONE;
        cells[i].isOpened = false;
        cells[i].isFlagged = false;
    }

    //mine set
    int l[] = {1, 3, 0, 6, 11, 33, 21, 23, 28, 40, 8 ,12};

    int color_cnt=0;
    for (Color color: {Color::RED, Color::GREEN, Color::BLUE}) {
        for (int i = 0+color_cnt*MINE_NUM; i < MINE_NUM*(color_cnt+1); i++) {
            cells[l[i]].mineColor = color;
        }
        color_cnt++;
    }

    //number set
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        if (cells[i].mineColor == Color::NONE) {
            int cnt = 0;
            Color color = Color::NONE;
            if (i%CELL_NUM!=0) {
                //is left cell exist?
                if (cells[i-1].mineColor!=Color::NONE) {
                    cnt++;
                    color |= cells[i-1].mineColor;
                }
                //is top-left cell exist?
                if (i/CELL_NUM!=0 && cells[i-CELL_NUM-1].mineColor!=Color::NONE) {
                    cnt++;
                    color |= cells[i-CELL_NUM-1].mineColor;
                }
                //is bottom-left cell exist?
                if (i/CELL_NUM!=CELL_NUM-1 && cells[i+CELL_NUM-1].mineColor!=Color::NONE) {
                    cnt++;
                    color |= cells[i+CELL_NUM-1].mineColor;
                }
            }
            if (i%CELL_NUM!=CELL_NUM-1) {
                //is right cell exist?
                if (cells[i+1].mineColor!=Color::NONE) {
                    cnt++;
                    color |= cells[i+1].mineColor;
                }
                //is top-right cell exist?
                if (i/CELL_NUM!=0 && cells[i-CELL_NUM+1].mineColor!=Color::NONE)  {
                    cnt++;
                    color |= cells[i-CELL_NUM+1].mineColor;
                }
                //is bottom-right cell exist?
                if (i/CELL_NUM!=CELL_NUM-1 && cells[i+CELL_NUM+1].mineColor!=Color::NONE)  {
                    cnt++;
                    color |= cells[i+CELL_NUM+1].mineColor;
                }
            }
            //is top cell exist?
            if (i/CELL_NUM!=0 && cells[i-CELL_NUM].mineColor!=Color::NONE) {
                cnt++;
                color |= cells[i-CELL_NUM].mineColor;
            }
            //is bottom cell exist?
            if (i/CELL_NUM!=CELL_NUM-1 && cells[i+CELL_NUM].mineColor!=Color::NONE) {
                cnt++;
                color |= cells[i+CELL_NUM].mineColor;
            }
            cells[i].mineNum = cnt;
            cells[i].numberColor = color;
            //TODO:color
        }
    }

    return cells;
}

shared_ptr<Board> initBoard() {
    unique_ptr<Cursor> cursor_ptr = make_unique<Cursor>();
    cursor_ptr->x = 0;
    cursor_ptr->y = 0;

    unique_ptr<Cell[]> cells = initCells();

    shared_ptr<Board> board_ptr = make_shared<Board>();
    board_ptr->cursor = std::move(cursor_ptr);
    board_ptr->cells = std::move(cells);

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
            case ' ':
                //open cell
                system("clear");
                break;
        }
    }

    return 0;
}
