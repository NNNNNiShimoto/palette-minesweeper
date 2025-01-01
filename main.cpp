#include <string>
#include <iostream>

using namespace std;

#define CELL_NUM 7

string getNumber(int x, int y) {
    if (x==y) return "-";
    return "0";
}

void printBoard() {
    string board = "";
    for (int i = 0; i < CELL_NUM; i++) {
        board += "+";

        for (int j = 0; j < CELL_NUM; j++) {
            board += "---+";
        }

        board += "\n|";

        for (int j = 0; j < CELL_NUM; j++) {
            board += " "+getNumber(i,j)+" |";
        }

        board += "\n";
    }

    board += "+";

    for (int j = 0; j < CELL_NUM; j++) {
        board += "---+";
    }

    cout << board << endl;

}

int main(void) {
    printBoard();
    return 0;
}
