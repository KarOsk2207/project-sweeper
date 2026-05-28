#include "unity.h"
#include "../board.h"
#include <stdlib.h>  // rand
#include <time.h>

// спомогательная функция для тестов: переопределяем srand, чтобы использовать фиксированный seed
static unsigned int test_seed = 12345;
void test_srand(unsigned int seed) { test_seed = seed; }
int test_rand(void) {
    test_seed = test_seed * 1103515245 + 12345;
    return (int)(test_seed / 65536) % 32768;
}

// аменяем макросы srand/rand на наши, чтобы сделать тесты детерминированными
#define srand test_srand
#define rand test_rand

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
    BoardPlaceMines(&board, 4, 4);   // безопасный клик в центре
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
    // ручную поставим мину в (1,1)
    board.cells[1][1].hasMine = true;
    board.totalMines = 1; // чтобы не мешало
    BoardCalculateNumbers(&board);
    // летка (0,0) должна иметь 1 соседа (мину в (1,1))
    TEST_ASSERT_EQUAL_INT(1, board.cells[0][0].adjacentMines);
    // (0,1) - тоже 1
    TEST_ASSERT_EQUAL_INT(1, board.cells[0][1].adjacentMines);
    // (2,2) - 1
    TEST_ASSERT_EQUAL_INT(1, board.cells[2][2].adjacentMines);
    // (1,0) - 1
    TEST_ASSERT_EQUAL_INT(1, board.cells[1][0].adjacentMines);
}

void test_reveal_mine(void) {
    Board board;
    BoardInit(&board, 3, 3, 1);
    board.cells[1][1].hasMine = true;
    bool hit = BoardReveal(&board, 1, 1);
    TEST_ASSERT_TRUE(hit);                       // должно вернуть true (мина)
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
    // оставим мину в (0,0)
    board.cells[0][0].hasMine = true;
    // ткроем все не-мины
    board.cells[0][1].state = CELL_REVEALED;
    board.cells[1][0].state = CELL_REVEALED;
    board.cells[1][1].state = CELL_REVEALED;
    TEST_ASSERT_TRUE(BoardCheckVictory(&board));
    // Спрячем одну обратно
    board.cells[1][0].state = CELL_HIDDEN;
    TEST_ASSERT_FALSE(BoardCheckVictory(&board));
}

void test_first_click_safe(void) {
    Board board;
    BoardInit(&board, 9, 9, 10);
    BoardPlaceMines(&board, 4, 4);
    //  клетке (4,4) и соседних (3,3)… не должно быть мин
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++)
            TEST_ASSERT_FALSE(board.cells[4+dr][4+dc].hasMine);
}

int main(void) {
    // аглушка для srand/rand (уже заменены)
    test_board_init();
    test_place_mines_count();
    test_adjacent_calculation();
    test_reveal_mine();
    test_toggle_flag();
    test_victory_check();
    test_first_click_safe();

    unity_print_summary();
    return 0;
}
