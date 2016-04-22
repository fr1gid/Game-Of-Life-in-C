//	life.c	11/07/2015
//******************************************************************************
//  The Game of Life
//
//  Lab Description:
//
//  The universe of the Game of Life is an infinite two-dimensional orthogonal
//  grid of square cells, each of which is in one of two states, alive or dead.
//  With each new generation, every cell interacts with its eight neighbors,
//  which are the cells horizontally, vertically, or diagonally adjacent
//  according to the following rules:
//
//  1. A live cell stays alive (survives) if it has 2 or 3 live neighbors,
//     otherwise it dies.
//  2. A dead cell comes to life (birth) if it has exactly 3 live neighbors,
//     otherwise it stays dead.
//
//  An initial set of patterns constitutes the seed of the simulation. Each
//  successive generation is created by applying the above rules simultaneously
//  to every cell in the current generation (ie. births and deaths occur
//  simultaneously.)  See http://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
//  Author:    Paul Roper, Brigham Young University
//  Revisions: June 2013   Original code
//             07/12/2013  life_pr, life_cr, life_nr added
//             07/23/2013  generations/seconds added
//             07/29/2013  100 second club check
//             12/12/2013  SWITCHES, display_results, init for port1 & WD
//	           03/24/2014  init_life moved to lifelib.c, 0x80 shift mask
//	                       blinker added, 2x loops
//             03/23/2015  start_generation() added, display_results(void)
//             11/07/2015  2nd parameter to ERROR2
//
//  Built with Code Composer Studio Version: 5.5.0.00090
//******************************************************************************
//  Lab hints:
//
//  The life grid (uint8 life[80][10]) is an 80 row x 80 column bit array.  A 0
//  bit is a dead cell while a 1 bit is a live cell.  The outer cells are always
//  dead.  A boolean cell value (0 or non-zero) is referenced by:
//
//         life[row][col >> 3] & (0x80 >> (col & 0x07))
//
//  Each life cell maps to a 2x2 lcd pixel.
//
//                     00       01             08       09
//  life[79][0-9]   00000000 00000000  ...  00000000 00000000 --> life_pr[0-9]
//  life[78][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_cr[0-9]
//  life[77][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_nr[0-9]
//  life[76][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0         |
//     ...                                                            |
//  life[75-4][0-9]   ...      ...            ...      ...            v
//     ...
//  life[03][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[02][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[01][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[00][0-9]   00000000 00000000  ...  00000000 00000000
//
//  The next generation can be made directly in the life array if the previous
//  cell values are held in the life_pr (previous row), life_cr (current row),
//  and life_nr (next row) arrays and used to count cell neighbors.
//
//  Begin each new row by moving life_cr values to life_pr, life_nr values to
//  life_cr, and loading life_nr with the row-1 life values.  Then for each
//  column, use these saved values in life_pr, life_cr, and life_nr to
//  calculate the number of cell neighbors of the current row and make changes
//  directly in the life array.
//
//  life_pr[0-9] = life_cr[0-9]
//  life_cr[0-9] = life_nr[0-9]
//  life_nr[0-9] = life[row-1][0-9]
//
//	I David Owen wrote this program.
//******************************************************************************
//******************************************************************************
// includes --------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "msp430.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "life.h"
#include "lifelib.h"
#include <cctype>

extern volatile uint16 switches;		// debounced switch values
extern const uint16 life_image[];

// global variables ------------------------------------------------------------
uint8 life[NUM_ROWS][NUM_COLS/8];		// 80 x 80 life grid
uint8 life_pr[NUM_COLS/8];				// previous row
uint8 life_cr[NUM_COLS/8];				// current row
uint8 life_nr[NUM_COLS/8];				// next row

uint8* temp;
uint8* life_p;
uint8* life_c;
uint8* life_n;

//CELL Birth
void cellBirth(int row, int col){
	life[(row)][(col) >> 3] |= (0x80 >> ((col) & 0x07));
}

//CELL Birth Point creation
void cellBirthPoint(int row, int col){
	lcd_point((col) << 1, (row) << 1, 7);
}

//CELL Death
void cellDeath(int row, int col){
	life[(row)][(col) >> 3] &= ~(0x80 >> ((col) & 0x07));
}

//CELL Death Point deletion
void cellDeathPoint(int row, int col){
	lcd_point((col) << 1, (row) << 1, 6);
}

//TEST Cell
int cellTest(uint8 life_row[], int col){
	return (life_row[(col) >> 3] & (0x80 >> ((col) & 0x07)) ? 1 : 0);
}

//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int row, int col, const uint8* pattern){

	uint8 i; // iterator
	int number; //
	int c; // column value
	c = col;
	number = 0;
	while(*pattern && (*pattern != 'y')){ // are we at the star to of the pattern?
		*pattern++;
	}
	while(*pattern && !isdigit(*pattern)){ // have we gotten to the digits yet?
		*pattern++;
	}
	while(isdigit(*pattern)){ // While we are looking at numbers
		number = number * 10 + (*pattern - '0');
		*pattern++;
	}
	row += number;
	while(*pattern && *pattern != '\n'){
		*pattern++;
	}
	*pattern++;
	number = 1;

	// Start with actual pattern construction
	while(*pattern && (*pattern != '!')){ // are we at the end of the pattern?
		if(isdigit(*pattern)){ // is it a number currently?
			number = 0;
			while(isdigit(*pattern)){
				number = number * 10 + (*pattern - '0');
				*pattern++;
			}
		}else if(*pattern == 'b'){ // is it a dead cell?
			for(i = number; i > 0; i--){
				CELL_DEATH(row, col);
				CELL_DELETE(row, col);
				col++;
			}
			number = 1;
			*pattern++;
		}else if(*pattern == 'o'){ // is it a live cell?
			for(i = number; i > 0; i--){
				CELL_BIRTH(row, col);
				CELL_DRAW(row, col);
				col++;
			}
			number = 1;
			*pattern++;
		}else if(*pattern == '$'){ // are we at the end of the current line?
			for(i = number; i > 0; i--){
				row--;
			}
			col = c;
			number = 1;
			*pattern++;
		}
	}
} // end draw_rle_pattern


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void){
	// System initalization
	RBX430_init(CLOCK);					// init board
	ERROR2(_SYS, lcd_init());			// init LCD
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts

	// Game initalization
	lcd_clear();
	memset(life, 0, sizeof(life));		// clear life array
	lcd_backlight(ON);
	lcd_rectangle(0, 0, NUM_COLS*2, NUM_ROWS*2, 1);
	lcd_wordImage(life_image, 17, 50, 1);
	lcd_cursor(10, 20);
	printf("\b\tPress Any Key");

	// Variables for LIFE
	switches = 0;
	while (!switches);	// wait for any switch to pick a level
	// MAIN PROGRAM LOOP -------------------------------------------------------------
	while (1){

		// Initialize main variables
		uint16 row, col;
		int totalNeighbors;
		int leftNeighbors;
		int middleNeighbors;
		int currentCell;
		int rightNeighbors;

		// Clear Life Array
		memset(life, 0, sizeof(life));

		// Clear Sliding Arrays
		memset(life_pr, 0, 10 * sizeof(uint8));
		memset(life_cr, 0, 10 * sizeof(uint8));
		memset(life_nr, 0, 10 * sizeof(uint8));

		// Detect Pattern Style
		init_life(switches);  // load a new life seed into LCD
		switches = 0;

		//---------------------------------------------------------------------------
		// PERFORMANCE MUST BE OPTIMIZED BELLOW!
		//___________________________________________________________________________
		// Start a new generation
		while (1){
			RED_TOGGLE;
			// Begin at top left
			memcpy(&life_pr, &life[79], 10 * sizeof(uint8));
			memcpy(&life_cr, &life[78], 10 * sizeof(uint8));
			memcpy(&life_nr, &life[77], 10 * sizeof(uint8));

			row = (NUM_ROWS-2);
			do {
				// Start a new row
				totalNeighbors = 0;
				leftNeighbors = 0; // 0 becasue it is the border

				// Check surrounding cells
				currentCell = TEST_CELL(life_cr, 1); // are we alive?
				rightNeighbors = TEST_CELL(life_pr, 2) + TEST_CELL(life_cr, 2) + TEST_CELL(life_nr, 2); // is ther anything alive to the right? up down or current?
				middleNeighbors = TEST_CELL(life_pr, 1) + currentCell + TEST_CELL(life_nr, 1); // is there anything above or bellow us?

				col = 1;
				do{
					totalNeighbors = leftNeighbors + (middleNeighbors - currentCell) + rightNeighbors; // add up the nearby cells to get the total neighboors
					if(currentCell == 1){ // if the cell is currently alive
						if (!(totalNeighbors == 2 || totalNeighbors == 3)){
							// the cell has too few or too many neighbors
							CELL_DEATH(row, col);
							CELL_DELETE(row, col);
						}
					}else{	// the cell is currently dead
						if(totalNeighbors == 3){			// the cell has 3 live neighbors
							CELL_BIRTH(row, col);			// set cell bit in life array
							CELL_DRAW(row, col);			// set LCD 2x2 pixel point
						}
					}
					leftNeighbors = middleNeighbors;				// what was middle is now left
					currentCell = TEST_CELL(life_cr, col + 1);		// current now has to check for a new self
					middleNeighbors = rightNeighbors;				// what was right is now middle
					rightNeighbors = TEST_CELL(life_pr, col + 2) + TEST_CELL(life_cr, col + 2) + TEST_CELL(life_nr, col + 2);
					col++;
				} while (col < 79);

				memcpy(&life_pr, &life_cr, sizeof(life_cr));
				memcpy(&life_cr, &life_nr, sizeof(life_nr));
				memcpy(&life_nr, &life[row - 2], sizeof(life[0]));
				row--;
			} while (row > 0);
			if (display_results()) break; // display life generation and generations/second on LCD
		}
	}
}
