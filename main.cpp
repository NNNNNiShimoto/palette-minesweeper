#include <string>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <sstream>

#include "colortext.h"
#include "board.h"

using namespace std;

#define CELL_NUM 10
#define MINE_NUM 5

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

bool isOutOfBounds(int x, int y) {
    return x < 0 | x >= CELL_NUM | y < 0 | y >= CELL_NUM;
}

string getInfoString(shared_ptr<Board> board) {
    ostringstream oss;
    oss << redText("RED")     << ": " << to_string(board->redMineNum)   << ", ";
    oss << greenText("GREEN") << ": " << to_string(board->greenMineNum) << ", ";
    oss << blueText("BLUE")   << ": " << to_string(board->blueMineNum)  << ", ";
    oss << "REMAINING MINES: " << to_string(board->remainCellNum) << "\n\r";
    return oss.str();
}

string getNumberString(Cell cell) {
    if (cell.mineNumber==0) return " ";
    int n = cell.mineNumber;
    switch(cell.mineNumberColor) {
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

//return "P"(flag) or "."(not open) or "2"(mine num) or "X"(opened mine)
string getCellString(Cell cell) {
    if (cell.isFlag) {        
        switch(cell.flagColor) {
            case Color::RED:
                return boldText(redText("P"));
            case Color::GREEN:
                return boldText(greenText("P"));
            case Color::BLUE:
                return boldText(blueText("P"));
            default:
                return "E";
        }
    } else if (!cell.isOpened) {
        return ".";
    } else if (cell.mineColor==Color::NONE) {
        return getNumberString(cell);
    } else {
        switch(cell.mineColor) {
            case Color::RED:
                return underlineText(redText("X"));
            case Color::GREEN:
                return underlineText(greenText("X"));
            case Color::BLUE:
                return underlineText(blueText("X"));
            default:
                return "E";
        }
    }
}

string getHelpString() {
    ostringstream oss;

    oss << "---KEY CONTROLS---\n\r\n\r";

    oss << "[W/S/A/D] UP/DOWN/LEFT/RIGHT\n\r";
    oss << "[I] Place/Remove a " << redText("RED")     << " flag.\n\r";
    oss << "[O] Place/Remove a " << greenText("GREEN") << " flag.\n\r";
    oss << "[P] Place/Remove a " << blueText("BLUE")   << " flag.\n\r";
    oss << "[Space] Open a tile.\n\r";
    oss << "[C] Quit the game.\n\r\n\r";

    oss << "---COLOR HELP---\n\r\n\r";

    oss << redText("RED") << "   + " << blueText("BLUE") 
        << "  -> " << magentaText("MAGENTA") << "\n\r";
    oss << blueText("BLUE") << "  + " << greenText("GREEN") 
        << " -> " << cyanText("CYAN") << "\n\r";
    oss << greenText("GREEN") << " + " << redText("RED") 
        << "   -> " << yellowText("YELLOW") << "\n\r";
    oss << redText("RED") << " + " << blueText("BLUE") 
        << " + " << greenText("GREEN") << " -> " << whiteText("WHITE") << "\n\r\n\r";

    oss << "Press any key to return to the game.";

    return oss.str();
}

void printGameView(shared_ptr<Board> board, Cursor cursor, bool isHelp, bool isGameover) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        cerr << "ERROR: invalid index in printGameView()" << endl;
        exit(1);
    }

    ostringstream oss;

    //print information
    oss << getInfoString(board);

    //print Help menu
    if (isHelp) {
        oss << getHelpString();
        cout << oss.str();
        return;
    }

    //print board
    for (int i = 0; i < CELL_NUM; i++) {
        oss << "+";
        for (int j = 0; j < CELL_NUM; j++) {
            oss << "---+";
        }

        oss << "\n\r|";

        for (int j = 0; j < CELL_NUM; j++) {
            if (i == cursor.x && j == cursor.y) {
                oss << " " << underlineText(getCellString(board->cells[i*CELL_NUM+j])) << " |";
            } else {
                oss << " " << getCellString(board->cells[i*CELL_NUM+j]) << " |";
            }
        }
        oss << "\n\r";
    }

    for (int j = 0; j < CELL_NUM; j++) {
        oss << "+---";
    }
    oss << "+\n\r";

    if (!isGameover) oss << "[H] Open Help menu.\n\r";

    // for (int idx: *board->mineIdxList) {
    //     oss << idx << ", ";
    // }

    cout << oss.str();
}

vector<int> generateMineIdxList(int x, int y) {
    random_device seed;
    mt19937 gen(seed());
    vector<int> allIdxList;
    for (int i = 0; i < CELL_NUM*CELL_NUM; i++) {
        if (i != x*CELL_NUM+y) allIdxList.push_back(i);
    }
    shuffle(allIdxList.begin(), allIdxList.end(), gen);

    int mineNum = min(MINE_NUM*3, CELL_NUM*CELL_NUM);

    return vector<int> (allIdxList.begin(), allIdxList.begin()+mineNum);
}

void setCells(shared_ptr<Board> board, Cursor cursor) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        cerr << "ERROR: invalid index in setCells()" << endl;
        exit(1);
    }
    
    unique_ptr<Cell[]> cells = make_unique<Cell[]>(CELL_NUM*CELL_NUM);
    
    //all cells initialize
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        cells[i].mineColor = Color::NONE;
        cells[i].isOpened = false;
        cells[i].isFlag = false;
    }

    vector<int> mineIdxList = generateMineIdxList(cursor.x, cursor.y);
    
    //for each of three colors, set MINE_NUM mines  
    int color_cnt=0;
    for (Color color: {Color::RED, Color::GREEN, Color::BLUE}) {
        for (int i = 0; i < MINE_NUM; i++) {
            cells[mineIdxList[i+color_cnt*MINE_NUM]].mineColor = color;
        }
        color_cnt++;
    }

    board->mineIdxList = make_unique<vector<int>>(std::move(mineIdxList));

    //if cell is not mine, set number and number's color
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        if (cells[i].mineColor == Color::NONE) {
            int cnt = 0, x = i/CELL_NUM, y = i%CELL_NUM;
            Color color = Color::NONE;

            //loop for around 8 cells
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    if (!isOutOfBounds(x+dx, y+dy) 
                    && cells[(x+dx)*CELL_NUM+(y+dy)].mineColor != Color::NONE) {
                        cnt++;
                        color = color | cells[(x+dx)*CELL_NUM+(y+dy)].mineColor;
                    }
                }
            }

            cells[i].mineNumber = cnt;
            cells[i].mineNumberColor = color;
        }
    }

    board->cells = std::move(cells);

}

shared_ptr<Board> initBoard() {
    Cursor dummy = {0, 0};
    shared_ptr<Board> board_ptr = make_shared<Board>();

    //set dummy cursor 
    //b/c need to print GameView before first open
    setCells(board_ptr, dummy);

    board_ptr->redMineNum = MINE_NUM;
    board_ptr->greenMineNum = MINE_NUM;
    board_ptr->blueMineNum = MINE_NUM;
    board_ptr->remainCellNum = CELL_NUM*CELL_NUM-MINE_NUM*3;

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

void setFlag(shared_ptr<Board> board, Cursor cursor, Color color) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        cerr << "ERROR: invalid index in setFlag()" << endl;
        exit(1);  
    }

    if(board->cells[cursor.x*CELL_NUM+cursor.y].isFlag) {
        //if already flag exists, delete
        if (color==board->cells[cursor.x*CELL_NUM+cursor.y].flagColor) {
            board->cells[cursor.x*CELL_NUM+cursor.y].isFlag=false;
            board->cells[cursor.x*CELL_NUM+cursor.y].flagColor=Color::NONE;
            operateMineNum(board, color, true);
        } else {
        //if another color flag exists, replace
            operateMineNum(board, board->cells[cursor.x*CELL_NUM+cursor.y].flagColor, true);
            board->cells[cursor.x*CELL_NUM+cursor.y].flagColor=color;
            operateMineNum(board, color, false);
        }
    } else {
        //if flag not exists, place a flag
        if (!board->cells[cursor.x*CELL_NUM+cursor.y].isOpened) {
            board->cells[cursor.x*CELL_NUM+cursor.y].isFlag=true;
            board->cells[cursor.x*CELL_NUM+cursor.y].flagColor=color;
            operateMineNum(board, color, false);
        }
    }
}

void openCellRecursive(shared_ptr<Board> board, int x, int y){
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if ((dx == 0 && dy == 0) || isOutOfBounds(x+dx, y+dy)) continue;
            
            if(!board->cells[(x+dx)*CELL_NUM+y+dy].isFlag
            && !board->cells[(x+dx)*CELL_NUM+y+dy].isOpened 
            &&  board->cells[(x+dx)*CELL_NUM+y+dy].mineColor == Color::NONE) {
                board->cells[(x+dx)*CELL_NUM+y+dy].isOpened = true;
                board->remainCellNum--;
                if (board->cells[(x+dx)*CELL_NUM+y+dy].mineNumber == 0) openCellRecursive(board, x+dx, y+dy); 
            }
        }
    }
}

//open cells using openCellRecursive(), 
//ret 0:notmine, 1:mine
int openCell(shared_ptr<Board> board, Cursor cursor){
    if (isOutOfBounds(cursor.x, cursor.y)) {
        cerr << "ERROR: invalid index in openCell()" << endl;
        exit(1);  
    }

    //cannot open the flag cell or already opened cell
    if (!board->cells[cursor.x*CELL_NUM+cursor.y].isFlag
      &&!board->cells[cursor.x*CELL_NUM+cursor.y].isOpened) {

        //if mine cell opened
        if (board->cells[cursor.x*CELL_NUM+cursor.y].mineColor!=Color::NONE) {
            return 1;
        }

        board->cells[cursor.x*CELL_NUM+cursor.y].isOpened = true;
        board->remainCellNum--;

        //if opened cell was blanc, open recursively
        if (!board->cells[cursor.x*CELL_NUM+cursor.y].mineNumber) {
            openCellRecursive(board, cursor.x, cursor.y);
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
