#include <iostream>
using namespace std;

#define RIGHT 0
#define DOWN 1
#define NONE -1

struct Position {
    int x;
    int y;
};

void PrintBoard(char board[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            cout << board[i][j];
        }
        cout << endl;
    }
}

int CheckDirection(Position point, char board[10][10]) {
    if (point.y < 9) {
        if (board[point.x][point.y + 1] == '*') {
            return RIGHT;
        }
    }
    if (point.x < 9) {
        if (board[point.x + 1][point.y] == '*') {
            return DOWN;
        }
    }
    return NONE;
}

Position FindShipElement(char board[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (board[i][j] == '*') {
                return{ i, j };
            }
        }
    }
    return { -1, -1 }; 
}

int DetermineSize(char board[10][10], Position point, int direction) {
    int size = 1;
    if (direction == RIGHT) {
        while (++point.y < 10 && board[point.x][point.y] == '*') {
            size++;
        }
    }
    if (direction == DOWN) {
        while (++point.x < 10 && board[point.x][point.y] == '*') {
            size++;
        }
    }
    return size;
}

void ChangeShip(char board[10][10], Position point, int direction, int size) {
    int ship_char_val = size % 10;
    char ship_char = char('0' + ship_char_val);
    board[point.x][point.y] = ship_char;

    if (direction == RIGHT) {
        for (int i = 1; i < size; i++) {
            board[point.x][point.y + i] = ship_char;
        }
    }
    if (direction == DOWN) {
        for (int i = 1; i < size; i++) {
            board[point.x + i][point.y] = ship_char;
        }
    }
}

int main() {
    char board[10][10];

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            cin >> board[i][j];
        }
    }
    cout << endl;

    while (true) {
        Position point = FindShipElement(board);

        // Warunek zakonczenia: brak statkow
        if (point.x == -1) {
            PrintBoard(board);
            return 0;
        }

        int direction = CheckDirection(point, board);
        int size = DetermineSize(board, point, direction);

        ChangeShip(board, point, direction, size);
    }

    return 0;
}