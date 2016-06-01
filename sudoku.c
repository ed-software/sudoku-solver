#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku.h"

/*
 * All candidates start as unused.
 */
void init_candidates(candidates *c)
{
	int i;

	for (i = MIN_NUM; i <= MAX_NUM; ++i)
		(*c)[i] = 0;
}

/*
 * Using a candidate number means marking it as used in the array.
 */
void use_candidate(candidates *cp, int num)
{
	assert_(cp != NULL && num >= MIN_NUM && num <= MAX_NUM);

	(*cp)[num] = 1;
}

/*
 * Restoring a candidate means marking it as unused in the array.
 */
void restore_candidate(candidates *cp, int num)
{
	assert_(cp != NULL && num >= MIN_NUM && num <= MAX_NUM);

	(*cp)[num] = 0;
}

/*
 *
 * CELLS AND BOARDS.
 *
 */

/*
 * Auxiliar. Calculates the square number for the given cell. Squares are
 * numberd from top to bottom, left to right.
 */
int square(int row, int col)
{
	assert_(row >= MIN_NUM && row <= MAX_NUM &&
	       col >= MIN_NUM && col <= MAX_NUM);

	return (((row - 1) / SUBDIMENSION) * SUBDIMENSION) +
		((col - 1) / SUBDIMENSION) + 1;
}

/*
 * Every board starts empty. Cell candidate pointers are established.
 */
void init_board(struct board *b)
{
	int i;
	int j;

	assert_(b != NULL);

	b->unset_cells = TOTAL_NUMS * TOTAL_NUMS;

	for (i = MIN_NUM; i <= MAX_NUM; ++i) {
		init_candidates(b->rows + i);
		init_candidates(b->columns + i);
		init_candidates(b->squares + i);

		for (j = MIN_NUM; j <= MAX_NUM; ++j) {
			b->cells[i][j].has_value = 0;
			b->cells[i][j].value = 0;
			b->cells[i][j].row_candidates = b->rows + i;
			b->cells[i][j].col_candidates = b->columns + j;
			b->cells[i][j].square_candidates = b->squares + square(i, j);
		}
	}
}

/*
 * Finds the lowest candidate number which is free in all arrays, having a
 * value greater or equal to the "atleast" argument.
 */
int find_common_free(candidates *r, candidates *c, candidates *s, int atleast)
{
	assert_(r != NULL && c != NULL && s != NULL);

	int i;
	for (i = atleast; i <= MAX_NUM; ++i)
		if ((! (*r)[i]) && (! (*c)[i]) && (! (*s)[i]))
			return i;
	return (-1);
}

/*
 * Sets a cell value in the given board.
 */
void set_cell(struct board *b, int r, int c, int val)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM &&
	       val >= MIN_NUM && val <= MAX_NUM);

	assert_((! (*(b->cells[r][c].row_candidates))[val]) &&
	       (! (*(b->cells[r][c].col_candidates))[val]) &&
	       (! (*(b->cells[r][c].square_candidates))[val]));

	b->unset_cells -= 1;
	b->cells[r][c].has_value = 1;
	b->cells[r][c].value = val;
	use_candidate(b->cells[r][c].row_candidates, val);
	use_candidate(b->cells[r][c].col_candidates, val);
	use_candidate(b->cells[r][c].square_candidates, val);
}

/*
 * Unsets a cell value in the given board.
 */
void unset_cell(struct board *b, int r, int c, int val)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM &&
	       val >= MIN_NUM && val <= MAX_NUM);

	assert_((*(b->cells[r][c].row_candidates))[val] &&
	       (*(b->cells[r][c].col_candidates))[val] &&
	       (*(b->cells[r][c].square_candidates))[val]);

	b->unset_cells += 1;
	b->cells[r][c].has_value = 0;
	b->cells[r][c].value = 0;
	restore_candidate(b->cells[r][c].row_candidates, val);
	restore_candidate(b->cells[r][c].col_candidates, val);
	restore_candidate(b->cells[r][c].square_candidates, val);
}

/*
 * Checks if a cell value is set. Returns 1 if set, 0 otherwise.
 */
int is_set(struct board *b, int r, int c)
{
	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM);

	return (b->cells[r][c].has_value);
}

/*
 * Calculates the number following a given one circularly.
 */
int following(int num)
{
	assert_(num >= MIN_NUM && num <= MAX_NUM);

	return ((num - MIN_NUM + 1) % TOTAL_NUMS + MIN_NUM);
}

/*
 * Calculates the cell following a given one. Advances from top to bottom and
 * left to right. Returns 0 if there is no next cell, 1 otherwise and modifies
 * the arguments to point to the next cell in that case.
 */
int next_cell(int *r, int *c)
{
	assert_(r != NULL && c != NULL);

	if ((*r) == MAX_NUM && (*c) == MAX_NUM)
		return 0;

	*c = following(*c);
	if ((*c) == MIN_NUM)
		(*r) = following(*r);
	return 1;
}

/*
 * Prints the given board on screen.
 */
void print_board(struct board *b)
{
	int i;
	int j;

	assert_(b != NULL);

	for (i = MIN_NUM; i <= MAX_NUM; ++i) {
		for (j = MIN_NUM; j <= MAX_NUM; ++j)
			printf(" %d", b->cells[i][j].value);
		printf("\n");
	}
}

/*
 * Solves a board starting with the given cell. Returns 1 if the board could be
 * solved, 0 if not.
 */
int solve_board(struct board *b, int r, int c)
{
	int prev;
	int val;

	assert_(b != NULL &&
	       r >= MIN_NUM && r <= MAX_NUM &&
	       c >= MIN_NUM && c <= MAX_NUM);

	/* Base case: board solved, print it. */
	if (b->unset_cells == 0) {
		print_board(b);
		return 1;
	}

	/* Find the next unset cell. */
	while (is_set(b, r, c) && next_cell(&r, &c));

	/* This should never happen. */
	if (is_set(b, r, c))
		return 1;

	/* Try every possible cell value until the board can be solved. */
	prev = MIN_NUM;
	while (1) {
		val = find_common_free(b->cells[r][c].row_candidates,
				       b->cells[r][c].col_candidates,
				       b->cells[r][c].square_candidates,
				       prev);
		if (val == -1)
			break;

		set_cell(b, r, c, val);
		if (solve_board(b, r, c))
			return 1;
		unset_cell(b, r, c, val);

		prev = val+1;
	}

	return 0;
}

/*
 * Reads a board from the given file. Format: a digit represents a cell value,
 * a dot represents an empty cell. Cells should be given from top to bottom and
 * left to right. All other characters are ignored.
 *
 * Example:
 *
 *     5 3 . . 7 . . . .
 *     6 . . 1 9 5 . . .
 *     . 9 8 . . . . 6 .
 *     8 . . . 6 . . . 3
 *     4 . . 8 . 3 . . 1
 *     7 . . . 2 . . . 6
 *     . 6 . . . . 2 8 .
 *     . . . 4 1 9 . . 5
 *     . . . . 8 . . 7 9
 *
 */
void read_board(char *f, struct board *b)
{
	int row;
	int col;
	int c;
	int i = 0;

	assert_(f != NULL && b != NULL);

	row = MIN_NUM;
	col = MIN_NUM;


	// while (! feof(f)) {
	while (f != 0) {
		c = f[i];
		if ((isdigit(c) && c != '0') || c == '.') {
			if (c != '.')
				set_cell(b, row, col, (c - '0'));
			if (! next_cell(&row, &col))
				break;
		}
		i++;
	}
	//print_board(b);
}


/*
 * Returns the given board on screen.
 */
char* return_board(struct board *b)
{
	int i;
	int j;
	char* sudoku = malloc(81*sizeof(char));
	char str[1]

	assert_(b != NULL);

	for (i = MIN_NUM; i <= MAX_NUM; ++i) {
		for (j = MIN_NUM; j <= MAX_NUM; ++j){
			sprintf(str, "%d", b->cells[i][j].value);
			strcat(sudoku, str);
		}

	}

	return sudoku;
}