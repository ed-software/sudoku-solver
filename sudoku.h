#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


#define SUBDIMENSION	(3)
#define MIN_NUM		(1)
#define MAX_NUM		(9)
#define TOTAL_NUMS	(9)
#define ARRAY_SIZE	(MIN_NUM + TOTAL_NUMS)

#ifdef ASSERT
#define assert_(A) assert(A)
#else
#define assert_(A)
#endif

/*
 *
 * CANDIDATES.
 *
 */

/*
 * A candidates array represents which values have already been used for a row,
 * column or square.
 */
typedef int candidates[ARRAY_SIZE];

/*
 * A cell has a flag to indicate if its value has been set or not, the cell
 * value and three pointers to candidate arrays. One for the row it belongs to,
 * one for the column it belongs to and one for the square it belongs to.
 */
struct cell {
	int has_value;
	int value;

	candidates *row_candidates;
	candidates *col_candidates;
	candidates *square_candidates;
};

/*
 * A board has a number of unset cells, a matrix of cells and the candidate
 * arrays for each row, column and square in the board.
 */
struct board {
	int unset_cells;
	struct cell cells[ARRAY_SIZE][ARRAY_SIZE];

	candidates rows[ARRAY_SIZE];
	candidates columns[ARRAY_SIZE];
	candidates squares[ARRAY_SIZE];
};

void init_board(struct board *b);
void read_board(char *f, struct board *b);
int solve_board(struct board *b, int r, int c);
char* return_board(struct board *b);