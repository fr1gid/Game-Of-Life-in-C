//	life.h	04/09/2015
//*******************************************************************************
//
//  Author:		Paul Roper, Brigham Young University
//	Revisions:	June 2013	Original code
//				07/12/2013	cell_value, cell_birth, cell_death macros added
//				07/23/2013	Fixed macros, added ternary operator (?:), {}
//				12/12/2013	MAX_TIME, Prototypes for display_results, port1_init
//				03/23/2015	extern rle declarations dropped
//
//*******************************************************************************
//
//                            MSP430F2274
//                  .-----------------------------.
//            SW1-->|P1.0^                    P2.0|<->LCD_DB0
//            SW2-->|P1.1^                    P2.1|<->LCD_DB1
//            SW3-->|P1.2^                    P2.2|<->LCD_DB2
//            SW4-->|P1.3^                    P2.3|<->LCD_DB3
//       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
//        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
//        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
//        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
//                  |                             |
//         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
//        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
//        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
//         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
//   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
//             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
//           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
//           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
//                  '-----------------------------'
//
//*******************************************************************************

#ifndef LIFELIB_H_
#define LIFELIB_H_

// defined constants -----------------------------------------------------------
#ifndef myCLOCK
#define myCLOCK			1200000			// 1.2 Mhz clock
#define CLOCK			_1MHZ
#endif

#define	WDT_CTL			WDT_MDLY_32		// WD configuration (SMCLK, ~32 ms)
#define WDT_CPI			32000			// WDT Clocks Per Interrupt (@1 Mhz)
#define	WDT_1SEC_CNT	(myCLOCK/WDT_CPI)	// WDT counts/second
#define DEBOUNCE_CNT	2

#define NUM_ROWS		80
#define NUM_COLS		80

#ifndef MAX_TIME
#define MAX_TIME		60	//100
#endif

#ifndef MAX_GENERATION
#define MAX_GENERATION 1000
#endif

enum SEED { LIFE=0x01, BIRD=0x02, BOMB=0x04, YOURS=0x08 };

void draw_rle_pattern(int row, int col, const uint8* object);
void init_life(enum SEED seed);
void start_generation(void);
void watchdog_init(void);
void port1_init(void);
int display_results(void);

#endif /* LIFELIB_H_ */
