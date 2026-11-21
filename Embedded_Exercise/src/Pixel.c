/*
 * Pixel.c
 *
 *  Created on: -----
 *      Author: -----
 */

#include "Pixel.h"
#define RSTn (1 << 0)
#define LAT  (1 << 1)
#define SB   (1 << 2)
#define SCK  (1 << 3)
#define SDA  (1 << 4)


//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


// Here the setup operations for the LED matrix will be performed
void setup(){
	controlSignal = 0;
	channel = 0;

	// Toggle RSTn bit (wait 500 us in between sets)
	controlSignal &= ~RSTn;     // bit0 = 0
	usleep(500);
	controlSignal |= RSTn;		// bit0 = 1
	usleep(500);

	// Set SDA bit (every bit we send is 1)
	controlSignal |= SDA;		// bit4 = 1

	// Set 6-bit values in a loop: 24 values times 6 bits = 144 clock pulses
	for(uint8_t i = 0; i < 24; i++) {
		for(uint8_t j = 0; j < 6; j++) {
			controlSignal &= ~SCK;	// bit3 = 0
			controlSignal |= SCK; 	// bit3 = 1 (rising edge of SCK)
		}
	}

	// Set SB bit to enable transmission to the 8-bit register
	controlSignal |= SB;	// bit2 = 1


}

//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	//Hint: you can invert Y-axis quite easily with 7-y
	dots[x][y][0]=b;
	//Write rest of two lines of code required to make this function work properly (green and red colors to array).
	dots[x][y][1]=g;	// Set green to first element
	dots[x][y][2]=r;	// Set red to second element

}


//Put new data to led matrix. Hint: This function is supposed to send 24-bytes and parameter x is for channel x-coordinate.
void run(uint8_t x){
	controlSignal &= ~LAT;

	// 8 pixels
	for(uint8_t y = 0; y < 8; y++) {

		// 3 colors: b, g, r (same order as in the dots array)
		for(uint8_t c = 0; c < 3; c++) {
			uint8_t curr = dots[x][y][c];

			// 8 bits, MSB first
			for(uint8_t bit = 0; bit < 8; bit++) {
				if(curr & 0x80)	controlSignal |= SDA;	// top bit is 1
				else			controlSignal &= ~SDA;	// top bit is 0

				controlSignal &= ~SCK;
				controlSignal |= SCK;	// rising edge

				curr <<= 1;		// move the next bit to the top
			}
		}
	}

	controlSignal &= ~SCK;	// clock LOW at the end, as the guide requires
	latch();
}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){
	controlSignal |= LAT;	// bit1 = 1
	controlSignal &= ~LAT;	// bit1 = 0
}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){
	switch(x) {
	case 0: channel = 1 << 0; break;	// 00000001
	case 1: channel = 1 << 1; break;	// 00000010
	case 2: channel = 1 << 2; break;	// 00000100
	case 3: channel = 1 << 3; break;	// 00001000
	case 4: channel = 1 << 4; break;	// 00010000
	case 5: channel = 1 << 5; break;	// 00100000
	case 6: channel = 1 << 6; break;	// 01000000
	case 7: channel = 1 << 7; break;	// 10000000

	default: channel = 0; break;		// all lines off
	}

}