//	lifelib.c	11/07/2015
//******************************************************************************
//******************************************************************************
//
//	LL     IIII   FFFFFF  EEEEEE  LL     IIII  BBBBB        CCCCC
//	LL      II    FF      EE      LL      II   BB  BB      CC   CC
//	LL      II    FF      EE      LL      II   BB  BB      CC
//	LL      II    FFFF    EEEE    LL      II   BBBBB       CC
//	LL      II    FF      EE      LL      II   BB  BB      CC
//	LL      II    FF      EE      LL      II   BB  BB  ..  CC   CC
//	LLLLL  IIII   FF      EEEEEE  LLLLL  IIII  BBBBB   ..   CCCCC
//
//	!!!!!  DO NOT ALTER display_results function !!!!!
//
//	12/11/2014	Checking for 1000 iterations instead of 60 seconds
//	04/08/2014	Fixed generations/second fraction
//	11/07/2015	__interrupt void ISR_trap(void) moved to RBX430-1.c
//
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

//*******************************************************************************
//	life rle patterns

// still lifes
const uint8 block[];
const uint8 beehive[];
const uint8 loaf[];
const uint8 boat[];

// oscillators
const uint8 blinker[];
const uint8 toad[];
const uint8 beacon[];
const uint8 by_flop[];
const uint8 hexapole[];
const uint8 pulsar[];

// guns
const uint8 gosper_glider_gun[];
const uint8 gosper_glider_gun2[];

// spaceships
const uint8 glider[];
const uint8 glider1[];
const uint8 lwss[];
const uint8 mwss[];
const uint8 hwss[];
const uint8 bird[];
const uint8 loafer[];
const uint8 canada_goose[];

// factories
const uint8 beacon_maker[];
const uint8 blinker_puffer[];
const uint8 carnivore[];
const uint8 converter[];

const uint8 jon_arbuckle[];
const uint8 replicator[];

//*******************************************************************************

// global variables
volatile uint16 seconds;				// # of seconds
volatile uint16 switches;				// debounced switch values

static volatile uint16 WDT_sec_cnt;		// WDT second counter
static volatile uint16 WDT_debounce_cnt;// WDT debounce counter
static volatile uint16 old_seconds;
static volatile uint16 generation;		// generation counter

extern uint8 life[NUM_ROWS][NUM_COLS/8];

//	vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DO NOT ALTER vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//******************************************************************************
//	Call display_results after every generation
//
//	returns TRUE when MAX_TIME is reached or switch is pressed
//
int display_results(void)
{
	//LED_RED_ON;						// turn red LED on
	generation++;					// next generation
	lcd_mode(0);
	lcd_cursor(4, 1);
	lcd_printf("%d", generation);

	// only update when seconds changes
	if (seconds != old_seconds)
	{
		old_seconds = seconds;
		printf("/%d", seconds);
		lcd_cursor(145, 1);
		printf("%2d", generation / seconds);
	}

	// check for MAX_GENERATION or MAX_TIME exceeded
	if ((generation >= MAX_GENERATION) || (seconds >= MAX_TIME))
	{
		long hSecs, hGensPerSec;
		_BIC_SR(GIE);
		lcd_cursor(4, 1);			// output results
		lcd_printf("%d/%d", generation, seconds);
		lcd_cursor(145, 1);
		printf("%2d", generation / seconds);
		lcd_cursor(50, 80);
		hSecs = ((long)seconds * 100) + (((WDT_1SEC_CNT - WDT_sec_cnt) * 100) / WDT_1SEC_CNT);
		hGensPerSec = ((long)generation * 100 * 100) / hSecs;
		lcd_printf("\a\b\t%ld.%02ld", hGensPerSec / 100, hGensPerSec % 100);
		if (generation < MAX_TIME * 5)
		{
			lcd_cursor(30, 30);
			lcd_printf("\a\b\tTOO SLOW!");
		}
		_BIS_SR(GIE);
		switches = 0;				// clear any pending switches
		while (!switches);			// wait for any switch
	}
	//LED_RED_OFF;					// turn red LED off
	return switches;				// return switch value
} // end display_results


//******************************************************************************
//	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DO NOT ALTER ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

void start_generation(void)
{
	WDT_sec_cnt = WDT_1SEC_CNT;		// reset WD 1 second counter
	seconds = 0;					// clear second counter
	old_seconds = 0;
	switches = 0;					// clear switch variable
	generation = 0;					// start generation counter
	return;
}


//------------------------------------------------------------------------------
//	Initialize Life pattern seed -----------------------------------------------
void init_life(enum SEED seed)
{
	int i;
	lcd_clear();						// clear LCD
	memset(life, 0, sizeof(life));		// clear life array
	lcd_backlight(ON);					// turn on LCD backlight
	lcd_rectangle(0, 0, NUM_COLS*2, NUM_ROWS*2, 1);
	switch(seed)
	{
		case LIFE:
		{
			draw_rle_pattern(65, 30, gosper_glider_gun);
			draw_rle_pattern(60, 5, pulsar);
			draw_rle_pattern(4, 7, beacon_maker);
			draw_rle_pattern(4, 30, hexapole);
			draw_rle_pattern(4, 40, hexapole);
			draw_rle_pattern(4, 50, hexapole);
			draw_rle_pattern(4, 60, hexapole);
			draw_rle_pattern(15, 65, loafer);
			break;
		}

		case BIRD:
		{
			draw_rle_pattern(60, 5, pulsar);		// sun
			draw_rle_pattern(31, 2, bird);			// bird
			draw_rle_pattern(40, 20, hwss);			// bigger bird
			draw_rle_pattern(11, 29, glider);		// blows bird apart
			draw_rle_pattern(65, 65, loafer);

			for (i = 2; i <= 78; i += 3)			// draw ground
			{
				draw_rle_pattern(7, i, block);		// block
			}
			break;
		}

		case BOMB:
		{
			draw_rle_pattern(65, 10, gosper_glider_gun);
			draw_rle_pattern(40, 10, lwss);			// LWSS
			draw_rle_pattern(20, 10, pulsar);		// pulsar
			draw_rle_pattern(20, 35, pulsar);		// pulsar
			draw_rle_pattern(20, 60, pulsar);		// pulsar
			draw_rle_pattern(10, 10, block);		// block
			draw_rle_pattern(10, 18, beehive);		// beehive
			draw_rle_pattern(10, 26, loaf);			// loaf
			draw_rle_pattern(10, 34, boat);			// boat
			draw_rle_pattern(10, 42, toad);			// toad
			draw_rle_pattern(10, 50, beacon);		// beacon
			draw_rle_pattern(10, 58, blinker);		// blinker
			draw_rle_pattern(10, 66, by_flop);		// by_flop
			break;
		}

		default:
		{
			draw_rle_pattern(4, 2, jon_arbuckle);	// pattern of choice
			break;
		}
	}
	// initialize game
	start_generation();					// start time
	return;
} // end init_life



//------------------------------------------------------------------------------
//	Initialize watchdog --------------------------------------------------------
void watchdog_init()
{
	// configure Watchdog and Port 1
	WDTCTL = WDT_CTL;					// Set Watchdog interval
	WDT_sec_cnt = WDT_1SEC_CNT;			// set WD 1 second counter
	WDT_debounce_cnt = 0;				// clear debounce count
	IE1 |= WDTIE;						// enable WDT interrupt
	return;
} // end watchdog_init


void port1_init(void)
{
	// configure P1 switches and ADXL345 INT1 for interrupt
	P1SEL &= ~0x0f;						// select GPIO
	P1DIR &= ~0x0f;						// Configure P1.0-3 as Inputs
	P1OUT |= 0x0f;						// use pull-ups
	P1IES |= 0x0f;						// high to low transition
	P1REN |= 0x0f;						// Enable pull-ups
	P1IE |= 0x0f;						// P1.0-3 interrupt enabled
	P1IFG &= ~0x0f;						// P1.0-3 IFG cleared
	WDT_debounce_cnt = 0;				// clear debounce count
	switches = 0;						// clear switches flag
	return;
} // end port1_init


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Port 1 interrupt service routine
//
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
	P1IFG &= ~0x0f;						// P1.0-3 IFG cleared
	WDT_debounce_cnt = DEBOUNCE_CNT;	// enable debounce
	return;
} // end Port_1_ISR


//------------------------------------------------------------------------------
// Watchdog Timer ISR ----------------------------------------------------------
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
	if (--WDT_sec_cnt == 0)
	{
		WDT_sec_cnt = WDT_1SEC_CNT;		// reset counter
		++seconds;						// count seconds
		LED_GREEN_TOGGLE;				// toggle green LED
	}

	// check for switch debounce
	if (WDT_debounce_cnt && (--WDT_debounce_cnt == 0))
	{
		switches = (P1IN ^ 0x0f) & 0x0f;
	}
} // end WDT_ISR(void)


// still lifes
const uint8 block[] = { "x = 2, y = 2, rule = B3/S23\n2o$2o!" };
const uint8 beehive[] = { "x = 4, y = 3, rule = B3/S23\nb2o$o2bo$b2o!" };
const uint8 loaf[] = { "x = 4, y = 4, rule = B3/S23\nb2o$o2bo$bobo$2bo!" };
const uint8 boat[] = { "x = 3, y = 3, rule = B3/S23\n2o$obo$bo!" };

// oscillators
const uint8 blinker[] = { "x = 3, y = 1, rule = B3/S23\n3o!" };
const uint8 toad[] = { "x = 4, y = 2, rule = B3/S23\nb3o$3o!" };
const uint8 beacon[] = { "x = 4, y = 4, rule = B3/S23\n2o$o$3bo$2b2o!" };

const uint8 by_flop[] = { "x = 6, y = 7, rule = B3/S23\n"
		"3bo$bobo$5bo$5o$5bo$bobo$3bo!" };

const uint8 hexapole[] = {	"x = 9, y = 9, rule = B3/S23\n"
		"2o$obo2$2bobo2$4bobo2$6bobo$7b2o!"  };

const uint8 pulsar[] = { "x = 15, y = 15, rule = B3/S23\n"
		"4bo5bo$4bo5bo$4b2o3b2o2$3o2b2ob2o2b3o$2bobobobobobo$4b2o3b2o2$"
		"4b2o3b2o$2bobobobobobo$3o2b2ob2o2b3o2$4b2o3b2o$4bo5bo$4bo5bo!" };

// guns
const uint8 gosper_glider_gun[] = {	"x = 36, y = 9, rule = B3/S23\n"
		"24bo$22bobo$12b2o6b2o12b2o$11bo3bo4b2o12b2o$2o8bo5bo3b2o$"
		"2o8bo3bob2o4bobo$10bo5bo7bo$11bo3bo$12b2o!" };

const uint8 gosper_glider_gun2[] = { "x = 36, y = 9, rule = B3/S23\n"
		"12b2o$11bo3bo$10bo5bo7bo$2o8bo3bob2o4bobo$2o8bo5bo3b2o$"
		"11bo3bo4b2o12b2o$12b2o6b2o12b2o$22bobo$24bo!" };

// spaceships
const uint8 glider[] = { "x = 3, y = 3, rule = B3/S23\n3o$2bo$bo!" };
const uint8 glider1[] = { "x = 3, y = 3, rule = B3/S23\nbo$2bo$3o!" };

const uint8 lwss[] = { "x = 5, y = 4, rule = B3/S23\no2bo$4bo$o3bo$b4o!" };
const uint8 mwss[] = { "x = 6, y = 5, rule = B3/S23\n2bo$o3bo$5bo$o4bo$b5o!" };
const uint8 hwss[] = { "x = 7, y = 5, rule = B3/S23\n2b2o$o4bo$6bo$o5bo$b6o!" };

const uint8 bird[] = { "x = 10, y = 20, rule = B3/S23\n"
		"4bobo$3bo$3bo$3bo2bo$3b3o5$4b2o$2b2ob2o$2b4o$3b2o3$2b3o$bo2bo$4bo$4bo$bobo!"  };

const uint8 loafer[] = { "x = 9, y = 9, rule = B3/S23\n"
		"b2o2bob2o$o2bo2b2o$bobo$2bo$8bo$6b3o$5bo$6bo$7b2o!"
};

const uint8 canada_goose[] = { "x = 13, y = 12, rule = B3/S23\n"
		"10b3o$b2o9bo$ob3o6bo$4b2o2b2o$8bo$4bo$3bo3b2o$4b2obobo$b2obo2bobo$4b2o"
		"4bo$9b2o$9b2o!" };

// factories
const uint8 beacon_maker[] = { "x = 55, y = 54, rule = B3/S23\n"
		"53b2o$52bobo$51bo$50bo$49bo$"
		"48bo$47bo$46bo$45bo$44bo$43bo$42bo$41bo$40bo$39bo$38bo$37bo$36bo$35bo$"
		"34bo$33bo$32bo$31bo$30bo$29bo$28bo$27bo$26bo$25bo$24bo$23bo$22bo$21bo$"
		"20bo$19bo$18bo$17bo$16bo$15bo$14bo$13bo$12bo$11bo$10bo$9bo$8bo$7bo$6bo"
		"$5bo$4bo$3bo$3o$2bo$2bo!" };

const uint8 blinker_puffer[] = { "x = 9, y = 18, rule = B3/S23\n"
		"5bo$3bo3bo$8bo$3bo4bo$4b5o4$6b2o$3b3ob2o$4b4o$5b2o2$2b2o$"
		"o4bo$6bo$o5bo$b6o!" };

const uint8 carnivore[] = { "x = 31, y = 21, rule = B3/S23\n"
		"28b2o$28b2o2$26b4o$26bo3bo$29bo$29bo$2bo17bo7bo$o3bo13bo3bo4bobo$5bo4b"
		"5o8bo4bo$o4bo3bo4bo3bo4bo$b5o8bo4b5o4bo$9bo3bo13bobo$11bo16bo$29bo$29b"
		"o$26bo3bo$26b4o2$28b2o$28b2o!" };

const uint8 converter[] = { "x = 29, y = 32, rule = B3/S23\n"
		"6bo14b3o$7b2o11bo2bo$6b2o15bo$23bo$20bobo8$17bobo$18b2o$18bo5$15bo$14b"
		"2o8b2o$2o11b3obo5bo2bo$2o10b2o8b2ob3o$13b2o8b2obo$14bo10bo2$14bo$13b2o"
		"$2o10b2o13b2o$2o11b3obo9b2o$14b2o$15bo!" };

// HighLife patterns
const uint8 replicator[] = { "x = 4, y = 4, rule = B36/S23\nb3o$o3b$o3b$o!" };

//	#C Square Root of Minus Garfield -- "Conway's Game of Death"
//	#C Submission by Henning Makholm <henning@makholm.net>
//	#C
//	#C Pseudo-still life in the likeness of Jon Arbuckle found by
//	#C a randomized search, partially inspired by Paul Callahan's
//	#C online still life generator. Several thousand candidates
//	#C were produced and inspected to find one that would burn
//	#C quickly when hit by a glider.
const uint8 jon_arbuckle[] = { "x = 126, y = 75, rule = B3/S23\n"
		"51bo$"
		"37b2ob2o8bobo$"
		"36bo2b2o2bo3b2o2bo$"
		"33b2o2bo4b2o3bob2o$"
		"32bo2b2o7b2obo5bo$"
		"25bo2b2obob2o9bobob8o$"
		"23b3o3bobo2bo11bobo7bo$"
		"22bo3b3o2b3o12b2o3b4obo2bo$"
		"22bo2bo2bobo19bo3bob4o$"
		"20b2o4bobob2o18b2o$"
		"20bo6bo30b2o$"
		"17b2obo37bobo$"
		"16bobob2o30b2o6bo$"
		"13bo3bo2bo14bo15bobo5bo$"
		"13b4o3bo13bobo8bobo3bobob2obo$"
		"17b3o15bobo3b2obob4obob2obob2o$"
		"15bobo18b2o2bobobo5bo$"
		"14bobo23b2o3b5o$"
		"14b2o20b2o14b2ob2obo$"
		"12b2o21bobo7b5o2bob2ob2o$"
		"13bo21bo7b3o2bobobo6b2o$"
		"12bo23bo5bo8b2o6bobo$"
		"12b2o19b4o4bobo9b2o5b2o$"
		"13bo15b2o2bo6bobo10bobo6b2o$"
		"12bo16bo4bo5b2o13bo6bo$"
		"13b3o14b4o20bo8bo$"
		"16bo23b2o12b4o6bo$"
		"13b3obob2o9b2o8b2o15bo5b2o$"
		"13bo2b2obobo8b2o24bo6bo$"
		"14bo4bobo18b2o14b2o6bo$"
		"13b2ob4obob2o5b2o9bo21b2o$"
		"12bo3bo4bo2bo4bobo8bo15b2o5bo$"
		"12b3o2b3ob2o6bo10b2o14bo7bo$"
		"15b2o2bobo5bob2o10bo15bo5b2o$"
		"12b2o3bobobobo2bobo11bo15b2o$"
		"12bob4obobob4obo11b2o14bo6b2o$"
		"13bo4bobobo5b2o11bo11b2obo2b2obobo$"
		"14b3obobo3b4o12bo5b2ob2obobobobobobo$"
		"16bobob4o3bo13b3obob2obobobobobobob2o$"
		"16bobo5bo18bobo5b2obobobo2bo$"
		"15b2obob4o20b2o8b2obobobo$"
		"16bobobo25b2o3b3o3bobob2o$"
		"16bobo2bo24bobobo2bo3b2o$"
		"15b2ob4o3b2obo2bo17b2o13b2o$"
		"17bo6bobob4o23b4o5bo$"
		"17bob3o2bobo5b2o21bo2bo6bo$"
		"16b2obo2bobo2bob2obobo21b2o8bo$"
		"17bobobobo2b2obobo2bo30b2o$"
		"17bobobo6bo5b2o20b2o6bo$"
		"16b2obob2o3b2o28bo8b2o$"
		"17bobobo4bo7b2o21bo9bo$"
		"17bobobo2bobo7bo23bo6b2o$"
		"16b2ob2o3b2o9bo21b2o6bo$"
		"15bo2bo13b4o21bo8bo$"
		"15b2obo5b2o6bo21b2obo6bobo$"
		"16bob2o3bobo7bo14b2ob2o2bob2o5b2o$"
		"15bo3bo3bo8b2o13bobobob2o$"
		"15bob2o5bo6bo15bo2bo13b2o$"
		"14b2obo5b2o6bo13b2ob2o14bo$"
		"15bobo11b2ob2o12bobo13bobo$"
		"15bob2o11bobobob2o8bobo4bo8b2o$"
		"14b2o14bobobobo2b2o4b2ob2o2bobo5b2o$"
		"15bo15bo3bo2b2o2bo9b2o5bobo$"
		"15bobo23b2o16b2o$16b2o25b2o12b2o$"
		"18b2o23bobo5b2obobobo$"
		"18bo2bo2bo19bobob2o2bob2o$"
		"19b6o20b2obob2o$"
		"25b2o21bo$"
		"21b2obo2bob2o14b4o$"
		"20bobob4obobo13bo3b2o$"
		"20b2o7bobo2bo8b2o2b2o2bo$"
		"18b2o6b2obob4o7bo2b2obob2o$"
		"19bo7bob2o9bo2b2o3bobo$"
		"17bo9bo4b3o3b5o2b4o2bo"
		"!"
		} ;


// lcd images
const uint16 life_image[] = { 126, 76,
	0x0dff,0x8000,0x87df,0x000e,0x08ff,0x8380,0x87df,0x05ff,0x7000,0xfe8e,0x01fe,
	0x7410,0x000e,0x06ff,0x0dff,0xfe80,0x01fe,0x769f,0x08ff,0xffce,0x01fe,0x039f,
	0x04ff,0xfb80,0x03fe,0x77df,0x06ff,0x0dff,0xffce,0x02fe,0x000e,0x07ff,0xffd0,
	0x01fe,0x069f,0x04ff,0xfc00,0x04fe,0x039f,0x05ff,0x0dff,0xffd0,0x02fe,0x039f,
	0x07ff,0x02fe,0x77df,0x04ff,0xffc0,0x04fe,0x77df,0x05ff,0x0dff,0x03fe,0x77df,
	0x07ff,0x03fe,0x04ff,0xfc00,0x04fe,0xd7df,0x05ff,0x0dff,0x04fe,0x000e,0x06ff,
	0x03fe,0x000e,0x03ff,0xfb80,0x05fe,0x000e,0x04ff,0x0dff,0x04fe,0x001a,0x06ff,
	0x03fe,0x0010,0x03ff,0xfb80,0x05fe,0x001f,0x04ff,0x0dff,0x04fe,0x041f,0x06ff,
	0xffd0,0x02fe,0x039f,0x03ff,0xf800,0x05fe,0x039f,0x04ff,0x0dff,0xffd0,0x03fe,
	0x77df,0x06ff,0xffce,0x02fe,0x039f,0x03ff,0x8000,0x05fe,0x069f,0x04ff,0x0dff,
	0xffd0,0x03fe,0xd7df,0x06ff,0xffce,0x02fe,0x039f,0x03ff,0x7000,0x05fe,0x77df,
	0x04ff,0x0dff,0xffce,0x04fe,0x000e,0x05ff,0xffc0,0x02fe,0x069f,0x04ff,0x05fe,
	0x77df,0x04ff,0x0dff,0xffce,0x01fe,0xffce,0x02fe,0x001f,0x05ff,0xfb80,0x02fe,
	0x07df,0x04ff,0xffd0,0x05fe,0x04ff,0x0dff,0xffce,0x01fe,0xffc0,0x02fe,0x039f,
	0x05ff,0xfb80,0x02fe,0x07df,0x04ff,0xffce,0x05fe,0x000e,0x03ff,0x0dff,0xffc0,
	0x01fe,0xffc0,0x02fe,0x07df,0x05ff,0x7000,0x02fe,0x041f,0x04ff,0xffc0,0x05fe,
	0x0010,0x03ff,0x0dff,0xfc00,0x01fe,0xfc00,0x02fe,0x77df,0x06ff,0xffda,0x01fe,
	0x039f,0x04ff,0xfc00,0x05fe,0x001f,0x03ff,0x0dff,0xfb80,0x01fe,0xfb80,0x03fe,
	0x06ff,0xfc00,0x01fe,0x001f,0x04ff,0xfb80,0x05fe,0x039f,0x03ff,0x0dff,0xfb80,
	0x01fe,0xfb80,0x03fe,0x000e,0x05ff,0x7000,0x87da,0x000e,0x04ff,0xd000,0x05fe,
	0x041f,0x03ff,0x02ff,0xfc0e,0x87df,0x7390,0x08ff,0xf800,0x01fe,0xfb80,0x03fe,
	0x001a,0x0cff,0x7000,0x05fe,0x07df,0x03ff,0x01ff,0xd380,0x03fe,0x0390,0x07ff,
	0x7000,0x01fe,0xf800,0x03fe,0x039f,0x05ff,0x8380,0x769f,0x06ff,0x05fe,0x77df,
	0x03ff,0x01ff,0xffce,0x04fe,0x000e,0x07ff,0x01fe,0xf800,0x03fe,0x041f,0x05ff,
	0xffd0,0x01fe,0x039f,0x05ff,0xffd0,0x04fe,0x87df,0x03ff,0x7000,0x05fe,0x769f,
	0x07ff,0x87d0,0xf800,0x03fe,0x07df,0x04ff,0x7000,0x02fe,0x041f,0x05ff,0xffce,
	0x05fe,0x03ff,0x8000,0x06fe,0x0010,0x06ff,0x7400,0x8000,0x03fe,0x77df,0x04ff,
	0xf800,0x02fe,0x77df,0x05ff,0xfc00,0x05fe,0x000e,0x02ff,0xfb80,0x06fe,0x069f,
	0x07ff,0x8000,0x03fe,0x87df,0x04ff,0xfb80,0x03fe,0x05ff,0xfb80,0x05fe,0x000e,
	0x02ff,0xfb80,0x07fe,0x000e,0x06ff,0x7000,0x04fe,0x04ff,0xfc00,0x03fe,0x0010,
	0x04ff,0xf800,0x05fe,0x001f,0x02ff,0xfb80,0x03fe,0x800e,0x03fe,0x039f,0x06ff,
	0x7000,0x04fe,0x000e,0x03ff,0xffc0,0x03fe,0x001f,0x04ff,0x8000,0x05fe,0x001f,
	0x02ff,0xffc0,0x03fe,0x01ff,0xffd0,0x02fe,0x77df,0x07ff,0x04fe,0x0010,0x03ff,
	0xffc0,0x03fe,0x039f,0x04ff,0x7000,0x05fe,0x039f,0x02ff,0xffc0,0x03fe,0x01ff,
	0xffce,0x03fe,0x000e,0x05ff,0x7000,0x04fe,0x001f,0x03ff,0xffc0,0x03fe,0x07df,
	0x05ff,0xffda,0x04fe,0x041f,0x02ff,0xffc0,0x03fe,0x01ff,0xfc00,0x03fe,0x001a,
	0x06ff,0x04fe,0x039f,0x03ff,0xffc0,0x03fe,0x77df,0x05ff,0xffce,0x04fe,0x07df,
	0x02ff,0xffc0,0x03fe,0x01ff,0xf800,0x03fe,0x041f,0x06ff,0x04fe,0x041f,0x03ff,
	0xffc0,0x03fe,0x87df,0x05ff,0xffce,0x04fe,0x77df,0x02ff,0xfc00,0x03fe,0x000e,
	0x7000,0x03fe,0x77df,0x06ff,0xffd0,0x03fe,0x07df,0x03ff,0xffc0,0x04fe,0x000e,
	0x04ff,0xfc00,0x04fe,0x87df,0x02ff,0xfb80,0x03fe,0x02ff,0x04fe,0x06ff,0xffd0,
	0x03fe,0x77df,0x03ff,0xfc00,0x04fe,0x000e,0x04ff,0xfb80,0x05fe,0x02ff,0xfb80,
	0x03fe,0x000e,0x01ff,0xffce,0x03fe,0x0010,0x05ff,0xffce,0x03fe,0x87df,0x03ff,
	0xfc00,0x04fe,0x001a,0x04ff,0xf800,0x05fe,0x000e,0x01ff,0xf800,0x03fe,0x000e,
	0x01ff,0xffc0,0x03fe,0x039f,0x05ff,0xffce,0x03fe,0xd7df,0x03ff,0xfb80,0x04fe,
	0x001f,0x04ff,0x7000,0x05fe,0x000e,0x01ff,0x7000,0x03fe,0x001a,0x01ff,0xfc00,
	0x03fe,0x041f,0x05ff,0xffce,0x04fe,0x03ff,0xfb80,0x04fe,0x039f,0x04ff,0x7000,
	0x05fe,0x0010,0x01ff,0x01ff,0x03fe,0x001f,0x01ff,0xfb80,0x03fe,0x77df,0x05ff,
	0xffce,0x04fe,0x000e,0x02ff,0xfb80,0x04fe,0x041f,0x05ff,0xffda,0x04fe,0x001f,
	0x01ff,0x01ff,0xffd0,0x02fe,0x039f,0x01ff,0xd000,0x03fe,0x87df,0x05ff,0xffc0,
	0x04fe,0x0010,0x02ff,0xfb80,0x04fe,0x07df,0x05ff,0xffce,0x04fe,0x039f,0x01ff,
	0x01ff,0xffce,0x02fe,0x041f,0x01ff,0x7000,0x04fe,0x05ff,0xffc0,0x04fe,0x001a,
	0x02ff,0xf800,0x04fe,0x77df,0x05ff,0xffce,0x04fe,0x039f,0x01ff,0x01ff,0xfb80,
	0x02fe,0x77df,0x02ff,0x04fe,0x000e,0x04ff,0xffc0,0x04fe,0x001f,0x02ff,0x8000,
	0x04fe,0x77df,0x05ff,0xfc00,0x04fe,0x039f,0x01ff,0x01ff,0x8000,0x02fe,0x77df,
	0x02ff,0x04fe,0x001f,0x04ff,0xfc00,0x04fe,0x039f,0x02ff,0x7000,0x04fe,0x87df,
	0x05ff,0xfb80,0x04fe,0x07df,0x01ff,0x02ff,0xffda,0x02fe,0x02ff,0xffd0,0x03fe,
	0x039f,0x04ff,0xfb80,0x04fe,0x039f,0x02ff,0x7000,0x05fe,0x05ff,0xf800,0x04fe,
	0x77df,0x01ff,0x02ff,0xffce,0x02fe,0x000e,0x01ff,0xffce,0x03fe,0x039f,0x01ff,
	0x8380,0x07fe,0x041f,0x02ff,0x7000,0x05fe,0x05ff,0x8000,0x04fe,0x77df,0x01ff,
	0x02ff,0xfb80,0x02fe,0x039f,0x01ff,0xffce,0x03fe,0x07df,0x01ff,0xffd0,0x07fe,
	0x07df,0x03ff,0x05fe,0x000e,0x04ff,0x7000,0x04fe,0x77df,0x01ff,0x02ff,0x7000,
	0x02fe,0x041f,0x01ff,0xffc0,0x03fe,0x07df,0x01ff,0x08fe,0x07df,0x03ff,0xffd0,
	0x04fe,0x000e,0x04ff,0x7000,0x04fe,0xd7df,0x01ff,0x03ff,0xffce,0x01fe,0x77df,
	0x01ff,0xffc0,0x03fe,0x77df,0x01ff,0xffd0,0x07fe,0x77df,0x03ff,0xffce,0x04fe,
	0x000e,0x05ff,0xffda,0x04fe,0x01ff,0x03ff,0xfb80,0x02fe,0x000e,0xffc0,0x03fe,
	0x77df,0x01ff,0xffce,0x07fe,0x77df,0x03ff,0xffce,0x04fe,0x0010,0x05ff,0xffce,
	0x04fe,0x000e,0x01ff,0xfc0e,0x0390,0x01ff,0xffda,0x01fe,0x039f,0xffc0,0x03fe,
	0x87df,0x01ff,0xffc0,0x07fe,0x87df,0x03ff,0xffc0,0x04fe,0x0010,0x05ff,0xffce,
	0x04fe,0x000e,0x8000,0x01fe,0x77df,0x01ff,0xfb80,0x01fe,0x77df,0xffce,0x03fe,
	0xd7df,0x01ff,0xfb80,0x08fe,0x03ff,0xfb80,0x04fe,0x001f,0x05ff,0xffc0,0x04fe,
	0x0010,0x8000,0x02fe,0x0010,0x01ff,0xffd0,0x06fe,0x01ff,0xd000,0x08fe,0x03ff,
	0xfb80,0x04fe,0x001f,0x05ff,0xfc00,0x04fe,0x001f,0x7000,0x02fe,0x77df,0x01ff,
	0x7380,0x87da,0xffd0,0x04fe,0x02ff,0x08fe,0x000e,0x02ff,0xf800,0x04fe,0x001f,
	0xfc00,0x7390,0x000e,0x02ff,0xfb80,0x04fe,0x001f,0x01ff,0xffce,0x02fe,0x0390,
	0x02ff,0xffd0,0x04fe,0x02ff,0xfc00,0x07fe,0x000e,0x02ff,0x8000,0x04fe,0x001f,
	0xffc0,0x01fe,0x77df,0x038e,0x01ff,0xf800,0x04fe,0x039f,0x01ff,0xd000,0x02fe,
	0x87df,0x000e,0x8000,0x04fe,0x87df,0x02ff,0x7000,0xfc0e,0x06fe,0x000e,0x02ff,
	0x7000,0x04fe,0x001f,0xfb80,0x03fe,0x7390,0xd00e,0x04fe,0x039f,0x02ff,0xffd0,
	0x08fe,0x77df,0x04ff,0x7380,0x05fe,0x0010,0x03ff,0x04fe,0x001a,0xfb80,0x09fe,
	0x039f,0x02ff,0xfc00,0x08fe,0x77df,0x05ff,0x05fe,0x000e,0x03ff,0xffd0,0x03fe,
	0x0010,0x8000,0x09fe,0x041f,0x02ff,0x7000,0x08fe,0x07df,0x04ff,0x7000,0x05fe,
	0x001f,0x03ff,0xffce,0x03fe,0x000e,0x01ff,0xffda,0x08fe,0x07df,0x03ff,0xfc0e,
	0x07fe,0x039f,0x05ff,0x05fe,0x001f,0x03ff,0xffce,0x03fe,0x000e,0x01ff,0xffce,
	0x08fe,0x07df,0x03ff,0x7000,0xffda,0x06fe,0x0010,0x05ff,0xffda,0x04fe,0x039f,
	0x03ff,0xfc00,0x02fe,0x87df,0x02ff,0xfb80,0x08fe,0x07df,0x04ff,0xfb80,0x05fe,
	0xd7df,0x06ff,0xffd0,0x04fe,0x001f,0x03ff,0xfb80,0x02fe,0x77df,0x02ff,0x7000,
	0x08fe,0x041f,0x05ff,0xffce,0x04fe,0x041f,0x06ff,0xffce,0x04fe,0x039f,0x03ff,
	0x8000,0x02fe,0x039f,0x03ff,0xffce,0x07fe,0x039f,0x05ff,0x7000,0xffd0,0x03fe,
	0x000e,0x06ff,0xffce,0x04fe,0x039f,0x04ff,0xffda,0x01fe,0x000e,0x03ff,0x8380,
	0x07fe,0x000e,0x06ff,0x7000,0xfc0e,0x87df,0x038e,0x07ff,0xffce,0x04fe,0x039f,
	0x04ff,0x8380,0x741f,0x05ff,0x7380,0xd410,0x03fe,0x741f,0x0390,0x01ff,0x11ff,
	0xffc0,0x04fe,0x039f,0x13ff,0x11ff,0xffc0,0x04fe,0x041f,0x13ff,0x11ff,0xfc00,
	0x04fe,0x041f,0x13ff,0x11ff,0xfb80,0x04fe,0x041f,0x13ff,0x11ff,0xfb80,0x04fe,
	0x041f,0x13ff,0x11ff,0xfb80,0x04fe,0x07df,0x13ff,0x11ff,0xf800,0x04fe,0x07df,
	0x13ff,0x11ff,0xf800,0x04fe,0x07df,0x13ff,0x11ff,0x8000,0x04fe,0x07df,0x13ff,
	0x11ff,0x7000,0x04fe,0x07df,0x13ff,0x11ff,0x7000,0x04fe,0x07df,0x13ff,0x12ff,
	0x04fe,0x07df,0x13ff,0x12ff,0xffce,0x03fe,0x069f,0x13ff,0x12ff,0xffc0,0x03fe,
	0x039f,0x13ff,0x12ff,0xd380,0x03fe,0x0010,0x13ff,0x13ff,0xfc0e,0x01fe,0x7390,
	0x14ff
};
