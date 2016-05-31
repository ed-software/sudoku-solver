/*
 * sudoku.h
 *
 * Author: Ed Jones z5122494
 *
 * Declares functions to be used in sudoku.c
 *
 */

void readBoard (FILE *f, struct board *b);

int solveBoard (struct board *b, int r, int c);

void printBoard (struct board *b);

int checkValidInput (struct board *b, int r, int c));