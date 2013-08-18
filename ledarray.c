/*
 * ledarray.c
 *
 * Created: 7/7/2013 12:20:27 AM
 *  Author: Justin
 */ 
#include "ledarray.h"
#include "tlc5940.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

extern uint16_t array[numrows][numcolumns];
extern uint8_t swap_array_flag;
extern uint8_t clear_array_flag;
extern uint8_t row;
extern uint8_t column;
volatile uint8_t flag = 0;
volatile uint8_t xlatNeedsPulse;
uint16_t nextarray[numrows][numcolumns];

void array_setup()
{
	// CTC with OCR0A as TOP
	TCCR1B |= (1 << WGM12);
	TCNT1 |= 0;
	
	// clk_io/1024 (From prescaler)
	TCCR1B |= ((1 << CS12) | (1 << CS10));
	
	// Generate an interrupt @ 1 Hz
	OCR1A = 20000;
	
	// Enable Timer/Counter1 Compare Match B interrupt
	TIMSK1 |= (1 << OCIE1A);
	
	// for loop to init nextarray to zeroes prior to first use
	row = 0; column = 0;
	for (column = 0; column < numcolumns; column++)
	{
		for (row = 0; row < numrows; row++)
		{
			nextarray[row][column] = 0;
		}
	}
	row = 0; column = 0;
}

/* Calculates the next array outside of ISR */
void update_array()
{
	uint8_t x = 0;			// Use local variables x and y to increment the row and column when updating
	uint8_t y = 0;
	if (flag ==0)
	{
		for (y = 0; y < numcolumns; y++)
		{
			for (x = 0; x < numrows; x++)
			{
				if(x == 10 && y==3)
			{nextarray[x][y] = 4000;}
				else
				{
					nextarray[x][y] = 0;
				}
			}
		}
		flag = 1;
	}
	
	else if (flag ==1)
	{x = 0; y = 0;
		for (y = 0; y < numcolumns; y++)
		{
			for (x = 0; x < numrows; x++)
			{
				if(x == 5  )
			{nextarray[x][y] = 4000;}
				else
				
				{
					nextarray[x][y] = 0;
				}
			}
		}
		flag = 0;
	}
	//refresh_array();				// Only after the entire array has been updated to we want to 
}

// Should clear entire buffer of all 16 channels to zero prior to writing the new array data to  the LED array
void refresh_array()
{
	for (int i=0; i < numChannels; i++)
	{
		TLC5940_SetAllGS(0);
		for (gsData_t i = 0; i < gsDataSize; i++) {
			SPDR = gsData[i];
			while (!(SPSR & (1 << SPIF)));
		}
		latchin(1);					// Latches it in		
	}
	clear_array_flag = 0;
	row = 0; column = 0;			// reset row and column to start writing to array from bottom left.
}

	
// The only thing this ISR does is swaps the current array with the next array.
// Need to check if the entire array has been written to the screen prior to refreshing.  This should be fast.
ISR(TIMER1_COMPA_vect) {
	if (swap_array_flag)
	{
		memcpy(&array, &nextarray, sizeof(array));		// Swaps nextarray with array
		swap_array_flag = 0;
		TLC5940_SetAllGS(0);
		refresh_array();
		
		
	
	}
		
	}
	
	

