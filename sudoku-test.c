/*
 * sudoku-test.c
 *
 * Author: Ed Jones z5122494
 *
 * Tests sudoku.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sudoku.h"

static void testSudoku (void);

int main (int argc, char *argv[])  {
    printf ("Testing sudoku...\n");
    testSudoku();
    printf ("All tests passed!  You are awesome\n");

	return EXIT_SUCCESS;
}

static void testSudoku (void) {


}