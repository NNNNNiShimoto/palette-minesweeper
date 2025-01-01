#include <stdio.h>
#include <string.h>

#define CELL_NUM 10

int getNumber(int x, int y) {
    return 0;
}

void printBoard() {
    char board[2048];

    char gridTopLeft     = '┏';
    char gridTopRight    = '┓';
    char gridBottomLeft  = '┗';
    char gridBottomRight = '┛';
    char topJoint    = '┳';
    char bottomJoint = '┻';
    char leftJoint   = '┣';
    char rightJoint  = '┫';
    char centerJoint = '╋';
    char verticalBar = '┃';
    char horizontalBar[] = "━━━";
    
    board[0] = gridTopLeft;
    board[1] = horizontalBar;
    printf("%s", board);
    // for (int i = 0; i < CELL_NUM; i++) {
    // }
}

int main(void) {
    // printf("┏━━━┳━━━┳━━━┳━━━┓\n");
    // printf("┃ 0 ┃ 0 ┃ 0 ┃ 0 ┃\n");
    // printf("┗━━━┻━━━┻━━━┻━━━┛\n");
    printBoard();
    return 0;
}
