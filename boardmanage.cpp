#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <random>

#include "board.h"
#include "colortext.h"

bool isOutOfBounds(int x, int y) {
    return x < 0 | x >= CELL_NUM | y < 0 | y >= CELL_NUM;
}

std::string getInfoString(std::shared_ptr<Board> board) {
    std::ostringstream oss;
    oss << redText("RED")     << ": " << std::to_string(board->redMineNum)   << ", ";
    oss << greenText("GREEN") << ": " << std::to_string(board->greenMineNum) << ", ";
    oss << blueText("BLUE")   << ": " << std::to_string(board->blueMineNum)  << ", ";
    oss << "REMAINING MINES: " << std::to_string(board->remainCellNum) << "\n\r";
    return oss.str();
}

std::string getNumberString(Cell cell) {
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
std::string getCellString(Cell cell) {
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

std::string getHelpString() {
    std::ostringstream oss;

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

void printGameView(std::shared_ptr<Board> board, Cursor cursor, bool isHelp, bool isGameover) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        std::cerr << "ERROR: invalid index in printGameView()" << std::endl;
        exit(1);
    }

    std::ostringstream oss;

    //print information
    oss << getInfoString(board);

    //print Help menu
    if (isHelp) {
        oss << getHelpString();
        std::cout << oss.str();
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

    std::cout << oss.str();
}

std::vector<int> generateMineIdxList(int x, int y) {
    std::random_device seed;
    std::mt19937 gen(seed());
    std::vector<int> allIdxList;
    for (int i = 0; i < CELL_NUM*CELL_NUM; i++) {
        if (i != x*CELL_NUM+y) allIdxList.push_back(i);
    }
    shuffle(allIdxList.begin(), allIdxList.end(), gen);

    int mineNum = std::min(MINE_NUM*3, CELL_NUM*CELL_NUM);

    return std::vector<int> (allIdxList.begin(), allIdxList.begin()+mineNum);
}

void setCells(std::shared_ptr<Board> board, Cursor cursor) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        std::cerr << "ERROR: invalid index in setCells()" << std::endl;
        exit(1);
    }
    
    std::unique_ptr<Cell[]> cells = std::make_unique<Cell[]>(CELL_NUM*CELL_NUM);
    
    //all cells initialize
    for(int i=0; i<CELL_NUM*CELL_NUM; i++) {
        cells[i].mineColor = Color::NONE;
        cells[i].isOpened = false;
        cells[i].isFlag = false;
    }

    std::vector<int> mineIdxList = generateMineIdxList(cursor.x, cursor.y);
    
    //for each of three colors, set MINE_NUM mines  
    int color_cnt=0;
    for (Color color: {Color::RED, Color::GREEN, Color::BLUE}) {
        for (int i = 0; i < MINE_NUM; i++) {
            cells[mineIdxList[i+color_cnt*MINE_NUM]].mineColor = color;
        }
        color_cnt++;
    }

    board->mineIdxList = std::make_unique<std::vector<int>>(std::move(mineIdxList));

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

std::shared_ptr<Board> initBoard() {
    Cursor dummy = {0, 0};
    std::shared_ptr<Board> board_ptr = std::make_shared<Board>();

    //set dummy cursor 
    //b/c need to print GameView before first open
    setCells(board_ptr, dummy);

    board_ptr->redMineNum = MINE_NUM;
    board_ptr->greenMineNum = MINE_NUM;
    board_ptr->blueMineNum = MINE_NUM;
    board_ptr->remainCellNum = CELL_NUM*CELL_NUM-MINE_NUM*3;

    return board_ptr;
}

void operateMineNum(std::shared_ptr<Board> board, Color color, bool isIncrease) {
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

void setFlag(std::shared_ptr<Board> board, Cursor cursor, Color color) {
    if (isOutOfBounds(cursor.x, cursor.y)) {
        std::cerr << "ERROR: invalid index in setFlag()" << std::endl;
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

void openCellRecursive(std::shared_ptr<Board> board, int x, int y){
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
int openCell(std::shared_ptr<Board> board, Cursor cursor){
    if (isOutOfBounds(cursor.x, cursor.y)) {
        std::cerr << "ERROR: invalid index in openCell()" << std::endl;
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
