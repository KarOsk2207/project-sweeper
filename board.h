#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#define MAX_ROWS 30
#define MAX_COLS 30

// Состояния клетки
typedef enum {
    CELL_HIDDEN,
    CELL_REVEALED,
    CELL_FLAGGED
} CellState;

// Структура одной клетки
typedef struct {
    bool hasMine;
    int adjacentMines;
    CellState state;
} Cell;

// Структура всего поля
typedef struct {
    int rows;
    int cols;
    int totalMines;
    Cell cells[MAX_ROWS][MAX_COLS];
    bool firstClickDone;
} Board;

// нициализация пустого поля
void BoardInit(Board* board, int rows, int cols, int mines);

// свобождение ресурсов (пока не нужно, но для будущего)
void BoardDestroy(Board* board);

// асстановка мин после первого безопасного клика (нажатая клетка и её соседи — без мин)
void BoardPlaceMines(Board* board, int safeRow, int safeCol);

// одсчёт чисел (adjacentMines) для всех клеток
void BoardCalculateNumbers(Board* board);

// ткрыть клетку, вернуть true если открыта мина (проигрыш)
bool BoardReveal(Board* board, int row, int col);

// Установить/снять флаг
void BoardToggleFlag(Board* board, int row, int col);

// роверка победы (все не-мины открыты)
bool BoardCheckVictory(Board* board);

#endif
