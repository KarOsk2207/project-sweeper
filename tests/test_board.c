#include "unity.h"
#include "../board.h"
#include <stdlib.h>  // rand
#include <time.h>

// етерминированный генератор для воспроизводимости тестов
static unsigned int test_seed = 12345;
void test_srand(unsigned int seed) { test_seed = seed; }
int test_rand(void) {
    test_seed = test_seed * 1103515245 + 12345;
    return (int)(test_seed / 65536) % 32768;
}

#define srand test_srand
#define rand test_rand

// ------------------------------------------------------------
// Тесты, которые уже были (базовые)
// ------------------------------------------------------------
void test_board_init(void) {
    Board board;
    BoardInit(&board, 9, 9, 10);
    TEST_ASSERT_EQUAL_INT(9, board.rows);
    TEST_ASSERT_EQUAL_INT(9, board.cols);
    TEST_ASSERT_EQUAL_INT(10, board.totalMines);
    TEST_ASSERT_FALSE(board.firstClickDone);
}

void test_place_mines_count(void) {
    Board board;
    BoardInit(&board, 9, 9, 10);
    BoardPlaceMines(&board, 4, 4);
    int count = 0;
    for (int r = 0; r < board.rows; r++) {
        for (int c = 0; c < board.cols; c++) {
            if (board.cells[r][c].hasMine) count++;
        }
    }
    TEST_ASSERT_EQUAL_INT(10, count);
}

void test_adjacent_calculation(void) {
    Board board;
    BoardInit(&board, 3, 3, 1);
    board.cells[1][1].hasMine = true;
    board.totalMines = 1;
    BoardCalculateNumbers(&board);
    TEST_ASSERT_EQUAL_INT(1, board.cells[0][0].adjacentMines);
    TEST_ASSERT_EQUAL_INT(1, board.cells[0][1].adjacentMines);
    TEST_ASSERT_EQUAL_INT(1, board.cells[2][2].adjacentMines);
    TEST_ASSERT_EQUAL_INT(1, board.cells[1][0].adjacentMines);
}

void test_reveal_mine(void) {
    Board board;
    BoardInit(&board, 3, 3, 1);
    board.cells[1][1].hasMine = true;
    bool hit = BoardReveal(&board, 1, 1);
    TEST_ASSERT_TRUE(hit);
    TEST_ASSERT_EQUAL_INT(CELL_REVEALED, board.cells[1][1].state);
}

void test_toggle_flag(void) {
    Board board;
    BoardInit(&board, 3, 3, 0);
    BoardToggleFlag(&board, 0, 0);
    TEST_ASSERT_EQUAL_INT(CELL_FLAGGED, board.cells[0][0].state);
    BoardToggleFlag(&board, 0, 0);
    TEST_ASSERT_EQUAL_INT(CELL_HIDDEN, board.cells[0][0].state);
}

void test_victory_check(void) {
    Board board;
    BoardInit(&board, 2, 2, 1);
    board.cells[0][0].hasMine = true;
    board.cells[0][1].state = CELL_REVEALED;
    board.cells[1][0].state = CELL_REVEALED;
    board.cells[1][1].state = CELL_REVEALED;
    TEST_ASSERT_TRUE(BoardCheckVictory(&board));
    board.cells[1][0].state = CELL_HIDDEN;
    TEST_ASSERT_FALSE(BoardCheckVictory(&board));
}

void test_first_click_safe(void) {
    Board board;
    BoardInit(&board, 9, 9, 10);
    BoardPlaceMines(&board, 4, 4);
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++)
            TEST_ASSERT_FALSE(board.cells[4+dr][4+dc].hasMine);
}

// ------------------------------------------------------------
// ополнительные тесты (граничные случаи)
// ------------------------------------------------------------

// Flood fill: при открытии пустой клетки с 0 соседей должны открыться все соседние пустые клетки
void test_flood_fill_empty(void) {
    Board board;
    BoardInit(&board, 3, 3, 0);          // без мин
    BoardPlaceMines(&board, 0, 0);       // формально, но мин 0
    BoardCalculateNumbers(&board);
    BoardReveal(&board, 0, 0);
    // се клетки должны стать открытыми, т.к. мин нет
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            TEST_ASSERT_EQUAL_INT(CELL_REVEALED, board.cells[r][c].state);
}

// овторное открытие уже открытой клетки не должно менять состояние
void test_reveal_already_revealed(void) {
    Board board;
    BoardInit(&board, 3, 3, 0);
    board.cells[1][1].state = CELL_REVEALED;
    BoardReveal(&board, 1, 1);
    TEST_ASSERT_EQUAL_INT(CELL_REVEALED, board.cells[1][1].state);
}

// ельзя открыть клетку, помеченную флагом
void test_reveal_flagged(void) {
    Board board;
    BoardInit(&board, 3, 3, 0);
    board.cells[0][0].state = CELL_FLAGGED;
    BoardReveal(&board, 0, 0);
    TEST_ASSERT_EQUAL_INT(CELL_FLAGGED, board.cells[0][0].state);
}

// одсчёт соседей с несколькими минами
void test_adjacent_multiple_mines(void) {
    Board board;
    BoardInit(&board, 3, 3, 0);
    board.cells[0][0].hasMine = true;
    board.cells[0][1].hasMine = true;
    board.cells[0][2].hasMine = true;
    BoardCalculateNumbers(&board);
    // Центральная клетка (1,1) должна иметь 3 мины
    TEST_ASSERT_EQUAL_INT(3, board.cells[1][1].adjacentMines);
    // летка (1,0) - 2 мины (соседи слева-вверху и слева-центр)
    TEST_ASSERT_EQUAL_INT(2, board.cells[1][0].adjacentMines);
}

// обеда не должна засчитываться, если остались флаги на безопасных клетках
void test_victory_with_flags(void) {
    Board board;
    BoardInit(&board, 2, 2, 1);
    board.cells[0][0].hasMine = true;
    board.cells[0][1].state = CELL_REVEALED;
    board.cells[1][0].state = CELL_REVEALED;
    board.cells[1][1].state = CELL_FLAGGED;   // флаг на безопасной клетке
    TEST_ASSERT_FALSE(BoardCheckVictory(&board));
    board.cells[1][1].state = CELL_REVEALED;
    TEST_ASSERT_TRUE(BoardCheckVictory(&board));
}

int main(void) {
    // азовые тесты
    test_board_init();
    test_place_mines_count();
    test_adjacent_calculation();
    test_reveal_mine();
    test_toggle_flag();
    test_victory_check();
    test_first_click_safe();

    // ополнительные тесты
    test_flood_fill_empty();
    test_reveal_already_revealed();
    test_reveal_flagged();
    test_adjacent_multiple_mines();
    test_victory_with_flags();

    unity_print_summary();
    return 0;
}
