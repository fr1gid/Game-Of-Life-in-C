//	life.h	03/23/2015
//*******************************************************************************

#ifndef LIFE_H_
#define LIFE_H_

#define myCLOCK			1600000			// 1.6 Mhz clock
#define CLOCK			_1MHZ

#define RED_TOGGLE P4OUT ^= 0x40


#define TEST_CELL(array,col) (array[(col) >> 3] & (0x80 >> ((col) & 0x07)) ? 1 : 0)

#define CELL_BIRTH(row, col) life[(row)][(col) >> 3] |= (0x80 >> ((col) & 0x07))
#define CELL_DRAW(row, col) lcd_point((col) << 1, (row) << 1, 7)

#define CELL_DEATH(row, col) life[(row)][(col) >> 3] &= ~(0x80 >> ((col) & 0x07))
#define CELL_DELETE(row, col) lcd_point((col) << 1, (row) << 1, 6)


//*******************************************************************************
#endif /* LIFE_H_ */
