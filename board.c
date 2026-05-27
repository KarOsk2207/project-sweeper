#include "board.h"
#include <stdlib.h>
#include <time.h>

// ---------------------------------------------------------------------------
// нутренние вспомогательные функции
static bool InBounds(Board* board, int r, int c) {
    return r >= 0 && r < board->rows && c >= 0 && c < board->cols;
}

static int CountNeighborMines(Board* board, int r, int c) {
    int count = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr;
            int nc = c + dc;
            if (InBounds(board, nr, nc) && board->cells[nr][nc].hasMine)
                count++;
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
void BoardInit(Board* board, int rows, int cols, int mines) {
    board->rows = rows;
    board->cols = cols;
    board->totalMines = mines;
    board->firstClickDone = false;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            board->cells[r][c].hasMine = false;
            board->cells[r][c].adjacentMines = 0;
            board->cells[r][c].state = CELL_HIDDEN;
        }
    }
}

void BoardDestroy(Board* board) {
    // ока нечего удалять, но оставим для единообразия
    (void)board;
}

// ---------------------------------------------------------------------------
void BoardPlaceMines(Board* board, int safeRow, int safeCol) {
    // Собираем все клетки, в которые можно поставить мину (кроме safe-зоны)
    int totalCells = board->rows * board->cols;
    int* candidates = (int*)malloc(totalCells * sizeof(int));
    int candCount = 0;

    for (int r = 0; r < board->rows; r++) {
        for (int c = 0; c < board->cols; c++) {
            // safe-зона: клетка, по которой кликнули, и 8 соседей
            if (abs(r - safeRow) <= 1 && abs(c - safeCol) <= 1) continue;
            candidates[candCount++] = r * board->cols + c;
        }
    }

    // сли мин нужно больше, чем доступных клеток — ограничим
    int minesToPlace = board->totalMines;
    if (minesToPlace > candCount) minesToPlace = candCount;

    // Тасование Фишера-етса для выбора случайных позиций
    srand((unsigned int)time(NULL));
    for (int i = 0; i < minesToPlace; i++) {
        int j = i + rand() % (candCount - i);
        int temp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = temp;

        int idx = candidates[i];
        int r = idx / board->cols;
        int c = idx % board->cols;
        board->cells[r][c].hasMine = true;
    }

    free(candidates);
    board->firstClickDone = true;
}

void BoardCalculateNumbers(Board* board) {
    for (int r = 0; r < board->rows; r++) {
        for (int c = 0; c < board->cols; c++) {
            if (!board->cells[r][c].hasMine) {
                board->cells[r][c].adjacentMines = CountNeighborMines(board, r, c);
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool BoardReveal(Board* board, int row, int col) {
    if (!InBounds(board, row, col)) return false;
    Cell* cell = &board->cells[row][col];
    if (cell->state == CELL_REVEALED || cell->state == CELL_FLAGGED)
        return false;

    cell->state = CELL_REVEALED;

    // сли открыли мину — проигрыш
    if (cell->hasMine) {
        return true;
    }

    // сли пустая клетка (0 мин вокруг) — открываем соседей рекурсивно
    if (cell->adjacentMines == 0) {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                BoardReveal(board, row + dr, col + dc);
            }
        }
    }
    return false; // не мина
}

void BoardToggleFlag(Board* board, int row, int col) {
    if (!InBounds(board, row, col)) return;
    Cell* cell = &board->cells[row][col];
    if (cell->state == CELL_HIDDEN) {
        cell->state = CELL_FLAGGED;
    } else if (cell->state == CELL_FLAGGED) {
        cell->state = CELL_HIDDEN;
    }
}

bool BoardCheckVictory(Board* board) {
    for (int r = 0; r < board->rows; r++) {
        for (int c = 0; c < board->cols; c++) {
            Cell* cell = &board->cells[r][c];
            if (!cell->hasMine && cell->state != CELL_REVEALED) {
                return false; // есть неоткрытая безопасная клетка
            }
        }
    }
    return true;
}
