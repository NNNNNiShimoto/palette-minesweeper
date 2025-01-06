#include <string>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <sstream>
using namespace std;

#define CELL_NUM 10
#define MINE_NUM 5

termios original;

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
    Color mineColor; //R,G,B or none only
    bool isOpened;
    bool isFlag;
    Color flagColor; //R,G,B or none only
    int mineNumber; 
    Color mineNumberColor;
};

struct Board{
    unique_ptr<Cursor> cursor;
    unique_ptr<Cell[]> cells;
    unique_ptr<vector<int>> mineIdxList;
    int redMineNum;
    int greenMineNum;
    int blueMineNum;
    int remainCellNum;
};

template <typename T>
string redText(const T& text) {
    return "\x1b[31m" + string(text) + "\x1b[39m";
}

template <>
string redText<int>(const int& text) {
    return "\x1b[31m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string greenText(const T& text) {
    return "\x1b[32m" + string(text) + "\x1b[39m";
}

template <>
string greenText<int>(const int& text) {
    return "\x1b[32m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string yellowText(const T& text) {
    return "\x1b[33m" + string(text) + "\x1b[39m";
}

template <>
string yellowText<int>(const int& text) {
    return "\x1b[33m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string blueText(const T& text) {
    return "\x1b[34m" + string(text) + "\x1b[39m";
}

template <>
string blueText<int>(const int& text) {
    return "\x1b[34m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string magentaText(const T& text) {
    return "\x1b[35m" + string(text) + "\x1b[39m";
}

template <>
string magentaText<int>(const int& text) {
    return "\x1b[35m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string cyanText(const T& text) {
    return "\x1b[96m" + string(text) + "\x1b[39m";
}

template <>
string cyanText<int>(const int& text) {
    return "\x1b[96m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string whiteText(const T& text) {
    return "\x1b[37m" + string(text) + "\x1b[39m";
}

template <>
string whiteText<int>(const int& text) {
    return "\x1b[37m" + to_string(text) + "\x1b[39m";
}

template <typename T>
string boldText(const T& text) {
    return "\x1b[1m" + string(text) + "\x1b[0m";
}

template <>
string boldText<int>(const int& text) {
    return "\x1b[1m" + to_string(text) + "\x1b[0m";
}

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

string getInfoString(shared_ptr<Board> board) {
    stringstream ss;
    ss << redText("RED")     << ": " << to_string(board->redMineNum)   << ", ";
    ss << greenText("GREEN") << ": " << to_string(board->greenMineNum) << ", ";
    ss << blueText("BLUE")   << ": " << to_string(board->blueMineNum)  << ", ";
    ss << "REMAINING MINES: " << to_string(board->remainCellNum) << "\n\r";
    return ss.str();
}

string getNumberString(int n, Color color) {
    if (n==0) return " ";
    switch(color) {
        case Color::RED:
            return redText(n);
        case Color::GREEN:
            return greenText(n);
        case Color::YELLOW:
            return yellowText(n);
        case Color::BLUE:
            return blueText(n);
        case Color::MAGENTA:
            return magentaText(n);
        case Color::CYAN:
            return cyanText(n);
        default:
            return whiteText(n);
    }
}

void printGameView(shared_ptr<Board> board) {
    string str = "";
    //print information
    str += getInfoString(board);

    //print board
    for (int i = 0; i < CELL_NUM; i++) {
        str += boldText("+");
        for (int j = 0; j < CELL_NUM; j++) {
            str += "---+";
        }

        str += "\n\r|";

        for (int j = 0; j < CELL_NUM; j++) {
            if (board->cells[i*CELL_NUM+j].isFlag) {
                str += " ";
                if (board->cursor->x == i && board->cursor->y == j) str += "\x1b[4m";
                switch(board->cells[i*CELL_NUM+j].flagColor) {
                    case Color::RED:
                        str += "\x1b[1m\x1b[31mP\x1b[39m\x1b[0m |";
                        break;
                    case Color::GREEN:
                        str += "\x1b[1m\x1b[32mP\x1b[39m\x1b[0m |";
                        break;
                    case Color::BLUE:
                        str += "\x1b[1m\x1b[34mP\x1b[39m\x1b[0m |";
                        break;
                    default:
                        str += "\x1b[1mP\x1b[0m |";  
                        break;                
                }
            } else if (!board->cells[i*CELL_NUM+j].isOpened) {
                if (board->cursor->x == i && board->cursor->y == j) {
                    str += " \x1b[4m.\x1b[0m |";
                } else {
                    str += " . |";
                }
            } else if (board->cells[i*CELL_NUM+j].mineColor==Color::NONE) {
                if (board->cursor->x == i && board->cursor->y == j) {
                    str += " \x1b[4m"+getNumberString(board->cells[i*CELL_NUM+j].mineNumber, board->cells[i*CELL_NUM+j].mineNumberColor)+"\x1b[0m |";
                } else {
                    str += " "+getNumberString(board->cells[i*CELL_NUM+j].mineNumber, board->cells[i*CELL_NUM+j].mineNumberColor)+" |";
                }
            } else {
                //use for gameover
                switch(board->cells[i*CELL_NUM+j].mineColor) {
                    case Color::RED:
                        str += " \x1b[4m\x1b[31mX\x1b[39m\x1b[0m |";
                        break;
                    case Color::GREEN:
                        str += " \x1b[4m\x1b[32mX\x1b[39m\x1b[0m |";
                        break;
                    case Color::BLUE:
                        str += " \x1b[4m\x1b[34mX\x1b[39m\x1b[0m |";
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

void setCells(shared_ptr<Board> board) {
    unique_ptr<Cell[]> cells = make_unique<Cell[]>(CELL_NUM*CELL_NUM);
    
    //all cells initialize
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        cells[i].mineColor = Color::NONE;
        cells[i].isOpened = false;
        cells[i].isFlag = false;
    }

    //mine set
    random_device seed;
    mt19937 gen(seed());
    vector<int> allIdxList;
    for (int i = 0; i < CELL_NUM*CELL_NUM; i++) {
        allIdxList.push_back(i);
    }
    shuffle(allIdxList.begin(), allIdxList.end(), gen);
    vector<int> mineIdxList(allIdxList.begin(), allIdxList.begin()+MINE_NUM*3);
    
    int color_cnt=0;
    for (Color color: {Color::RED, Color::GREEN, Color::BLUE}) {
        for (int i = 0+color_cnt*MINE_NUM; i < MINE_NUM*(color_cnt+1); i++) {
            cells[mineIdxList[i]].mineColor = color;
        }
        color_cnt++;
    }

    board->mineIdxList = make_unique<vector<int>>(std::move(mineIdxList));

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
            cells[i].mineNumber = cnt;
            cells[i].mineNumberColor = color;
        }
    }
    board->cells = std::move(cells);
    board->remainCellNum = CELL_NUM*CELL_NUM-MINE_NUM*3;
    //return cells;
}

shared_ptr<Board> initBoard() {
    unique_ptr<Cursor> cursor_ptr = make_unique<Cursor>();
    cursor_ptr->x = 0;
    cursor_ptr->y = 0;

    shared_ptr<Board> board_ptr = make_shared<Board>();
    setCells(board_ptr);
    board_ptr->cursor = std::move(cursor_ptr);
    board_ptr->redMineNum = MINE_NUM;
    board_ptr->greenMineNum = MINE_NUM;
    board_ptr->blueMineNum = MINE_NUM;

    return board_ptr;
}

void operateMineNum(shared_ptr<Board> board, Color color, bool isIncrease) {
    if (isIncrease) {
        switch(color) {
            case Color::RED:
                board->redMineNum++;
                break;
            case Color::GREEN:
                board->greenMineNum++;
                break;
            case Color::BLUE:
                board->blueMineNum++;
                break;
            default:
                break;
        }
    } else {
        switch(color) {
            case Color::RED:
                board->redMineNum--;
                break;
            case Color::GREEN:
                board->greenMineNum--;
                break;
            case Color::BLUE:
                board->blueMineNum--;
                break;
            default:
                break;
        }
    }
}

void setFlag(shared_ptr<Board> board, Color color) {
    if(board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isFlag) {
        if (color==board->cells[board->cursor->x*CELL_NUM+board->cursor->y].flagColor) {
            board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isFlag=false;
            board->cells[board->cursor->x*CELL_NUM+board->cursor->y].flagColor=Color::NONE;
            operateMineNum(board, color, true);
        } else {
            operateMineNum(board, board->cells[board->cursor->x*CELL_NUM+board->cursor->y].flagColor, true);
            board->cells[board->cursor->x*CELL_NUM+board->cursor->y].flagColor=color;
            operateMineNum(board, color, false);
        }
    } else {
        if (!board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isOpened) {
            board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isFlag=true;
            board->cells[board->cursor->x*CELL_NUM+board->cursor->y].flagColor=color;
            operateMineNum(board, color, false);
        }
    }
}

void openCellRec(shared_ptr<Board> board, int x, int y){
    if (y!=0) {
        if(!board->cells[x*CELL_NUM+y-1].isFlag //left
        && !board->cells[x*CELL_NUM+y-1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[x*CELL_NUM+y-1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[x*CELL_NUM+y-1].mineNumber) openCellRec(board, x, y-1); 
        }
        if (x!=0  //upper-left
        && !board->cells[(x-1)*CELL_NUM+y-1].isFlag 
        && !board->cells[(x-1)*CELL_NUM+y-1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x-1)*CELL_NUM+y-1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x-1)*CELL_NUM+y-1].mineNumber) openCellRec(board, x-1, y-1);
        }
        if (x!=CELL_NUM-1  //bottom-left
        && !board->cells[(x+1)*CELL_NUM+y-1].isFlag 
        && !board->cells[(x+1)*CELL_NUM+y-1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x+1)*CELL_NUM+y-1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x+1)*CELL_NUM+y-1].mineNumber) openCellRec(board, x+1, y-1);
        }
    }
    if (y!=CELL_NUM-1) {
        if(!board->cells[x*CELL_NUM+y+1].isFlag //right
        && !board->cells[x*CELL_NUM+y+1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[x*CELL_NUM+y+1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[x*CELL_NUM+y+1].mineNumber) openCellRec(board, x, y+1); 
        }
        if (x!=0  //upper-right
        && !board->cells[(x-1)*CELL_NUM+y+1].isFlag 
        && !board->cells[(x-1)*CELL_NUM+y+1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x-1)*CELL_NUM+y+1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x-1)*CELL_NUM+y+1].mineNumber) openCellRec(board, x-1, y+1);
        }
        if (x!=CELL_NUM-1  //bottom-right
        && !board->cells[(x+1)*CELL_NUM+y+1].isFlag 
        && !board->cells[(x+1)*CELL_NUM+y+1].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x+1)*CELL_NUM+y+1].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x+1)*CELL_NUM+y+1].mineNumber) openCellRec(board, x+1, y+1);
        }
    }
    if (x!=0
        && !board->cells[(x-1)*CELL_NUM+y].isFlag //top
        && !board->cells[(x-1)*CELL_NUM+y].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x-1)*CELL_NUM+y].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x-1)*CELL_NUM+y].mineNumber) openCellRec(board, x-1, y); 
    }
    if (x!=CELL_NUM-1
        && !board->cells[(x+1)*CELL_NUM+y].isFlag //bottom
        && !board->cells[(x+1)*CELL_NUM+y].isOpened 
        &&  board->cells[x*CELL_NUM+y-1].mineColor == Color::NONE) {
            board->cells[(x+1)*CELL_NUM+y].isOpened = true;
            board->remainCellNum--;
            if (!board->cells[(x+1)*CELL_NUM+y].mineNumber) openCellRec(board, x+1, y); 
    }
}

//open cells using openCellRec, 
//ret 0:notmine, 1:mine
int openCell(shared_ptr<Board> board){
    if (!board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isFlag
      &&!board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isOpened) {
        if (board->cells[board->cursor->x*CELL_NUM+board->cursor->y].mineColor!=Color::NONE) {
            return 1;
        }

        board->cells[board->cursor->x*CELL_NUM+board->cursor->y].isOpened = true;
        board->remainCellNum--;
        //if opened cell was blanc, open recursively
        if (!board->cells[board->cursor->x*CELL_NUM+board->cursor->y].mineNumber) {
            openCellRec(board, board->cursor->x, board->cursor->y);
        }
    }
    return 0;
}

bool getIsGameclear(shared_ptr<Board> board) {
    bool isClear = true;
    for(int idx : *board->mineIdxList) {
        if (!isClear) break;
        isClear &= board->cells[idx].flagColor == board->cells[idx].mineColor;
    }
    return isClear && board->remainCellNum<=0;
}

void gameOver(shared_ptr<Board> board) {
    for(int idx: (*board->mineIdxList)) {
        if (board->cells[idx].isFlag) {
            board->cells[idx].flagColor = board->cells[idx].mineColor;
        } else {
            board->cells[idx].isOpened = true;
        }
    }
    system("clear");
    printGameView(board);
    cout << "GAMEOVER!\n\r";
}

void gameClear(shared_ptr<Board> board) {
    for (int i=0; i<CELL_NUM*CELL_NUM; i++) {
        board->cells[i].isOpened = true;
    }
    system("clear");
    printGameView(board);
    cout << "CONGRATULATIONS!\n\r";
}

int main(void) {
    char key;
    bool isLoop = true, isFirst = true, isCancel = false;
    shared_ptr<Board> board = initBoard();

    enableRawMode();

    while(isLoop) {
        system("clear");
        printGameView(board);
        if (isCancel) cout << "Do you want to cancel this game? (y/n)\n\r";
        key = getchar();
        if (key=='c') {
            isCancel = true;
            continue;
        }
        if (isCancel){
            if (key=='y') break;
            else if (key=='n') isCancel = false;
            else continue;
        } 
        switch(key){
            case 'w':
                board->cursor->x = (board->cursor->x-1+CELL_NUM)%CELL_NUM;
                break;
            case 's':
                board->cursor->x = (board->cursor->x+1)%CELL_NUM;
                break;
            case 'a':
                board->cursor->y = (board->cursor->y-1+CELL_NUM)%CELL_NUM;
                break;
            case 'd':
                board->cursor->y = (board->cursor->y+1)%CELL_NUM;
                break;
            case ' ':
                if (openCell(board)) {
                    if (isFirst) {
                        while(board->cells[board->cursor->x*CELL_NUM+board->cursor->y].mineColor!=Color::NONE) {
                            setCells(board);
                        }
                        openCell(board);
                        isFirst = false;
                        break;
                    } else {
                        gameOver(board);
                        isLoop = false;
                        break;
                    }
                }
                isFirst = false;
                if (getIsGameclear(board)) {
                    gameClear(board);
                    isLoop = false;
                    break;
                }
                break;
            case 'i':
                setFlag(board, Color::RED);
                if (getIsGameclear(board)) {
                    gameClear(board);
                    isLoop = false;
                    break;
                }
                break;
            case 'o':
                setFlag(board, Color::GREEN);
                if (getIsGameclear(board)) {
                    gameClear(board);
                    isLoop = false;
                    break;
                }
                break;
            case 'p':
                setFlag(board, Color::BLUE);
                if (getIsGameclear(board)) {
                    gameClear(board);
                    isLoop = false;
                    break;
                }
                break;  
        }
    }

    return 0;
}
