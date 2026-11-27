/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************

//***Hint: Use sleep(x)  or usleep(x) if you want some delays.****
//***To call assembler code found in blinker.S, call it using: blinker();***


//Comment this if you want to disable all interrupts
#define enable_interrupts




/***************************************************************************************
Name:
Student number:

Name:
Student number:

Name:
Student number:

Tick boxes that you have coded

Led-matrix driver		Game		    Assembler
	[]					[]					[]

Brief description:

*****************************************************************************************/
// Globals
volatile uint8_t currentChannel = 0;     // column shown next by TickHandler

volatile uint8_t alienX = 0;
volatile int8_t  alienDir = 1;           // +1 = right, -1 = left
volatile uint8_t alienCounter = 0;

#define ALIEN_SPEED 3        // alien moves every 3rd tick

volatile uint8_t shipX = 3;

volatile uint8_t bulletActive = 0;       // 1 = a bullet is flying
volatile uint8_t bulletX = 0;
volatile uint8_t bulletY = 0;

volatile uint8_t points = 0;
volatile uint8_t misses = 0;
volatile uint8_t gameOver = 0;           // 1 = end screen is showing

#define WIN_POINTS 5

// Drawing
void drawShip(uint8_t x, uint8_t r, uint8_t g, uint8_t b){
	SetPixel(x-1, 7, r, g, b);
	SetPixel(x,   7, r, g, b);
	SetPixel(x+1, 7, r, g, b);
	SetPixel(x,   6, r, g, b);
}

void clearScreen(){
	for (uint8_t x = 0; x < 8; x++)
		for (uint8_t y = 0; y < 8; y++)
			SetPixel(x, y, 0, 0, 0);
}

// Winning screen: thumbs up
// 'Y' = yellow (hand), 'B' = blue (sleeve), '.' = off
void Win(){
	const char *pic[8] = {
		"........",
		"...YY...",
		"..YYY...",
		"BBYYYYYY",
		"BBYYYYYY",
		"BBYYYYYY",
		"BBYYYYY.",
		"........"
	};

	for (uint8_t y = 0; y < 8; y++) {
		for (uint8_t x = 0; x < 8; x++) {
			if (pic[y][x] == 'Y')      SetPixel(x, y, 255, 255, 0);   // yellow
			else if (pic[y][x] == 'B') SetPixel(x, y, 0, 0, 255);     // blue
			else                       SetPixel(x, y, 0, 0, 0);       // off
		}
	}
}

// Game state
void endGame(uint8_t win){
	gameOver = 1;

	if (win) {
		Win();          // thumbs up
		return;
	}
}

void resetGame(){
	gameOver = 0;
	points = 0;
	misses = 0;
	alienX = 0;
	alienDir = 1;
	shipX = 3;
	bulletActive = 0;

	clearScreen();
	drawShip(shipX, 0, 0, 255);
	SetPixel(alienX, 0, 255, 0, 0);
}

void alienHit(){
	points++;
	if (points >= WIN_POINTS) endGame(1);
}


int main()
{
	//**DO NOT REMOVE THIS****
	    init_platform();
	//************************


#ifdef	enable_interrupts
	    init_interrupts();
#endif

	    //setup screen
	    setup();

	    drawShip(shipX, 0, 0, 255);          // blue ship on the bottom rows
	    SetPixel(alienX, 0, 255, 0, 0);      // red alien on the top row


	    Xil_ExceptionEnable();



	    //Try to avoid writing any code in the main loop.
		while(1){


		}


		cleanup_platform();
		return 0;
}


//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){
	//Don't remove this
	uint32_t StatusEvent;

	// Exceptions must be disabled when updating screen
	Xil_ExceptionDisable();



	//****Write code here ****
	open_line(8);                  // all lines off first (prevents ghost pixels)
	run(currentChannel);           // send this column's data to the DM163
	open_line(currentChannel);     // open this column

	currentChannel++;              // next column on the next call
	if (currentChannel > 7) currentChannel = 0;


	//****END OF OWN CODE*****************

	//*********clear timer interrupt status. DO NOT REMOVE********
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
	//*************************************************************
	//enable exceptions
	Xil_ExceptionEnable();
}


//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	//Don't remove this
	uint32_t StatusEvent;

	//****Write code here ****
	// Game only runs while the end screen is not showing.
	// (No "return" here, so the interrupt status below is always cleared.)

	if (!gameOver) {

		// Alien
		alienCounter++;
		if (alienCounter >= ALIEN_SPEED) {
			alienCounter = 0;

			SetPixel(alienX, 0, 0, 0, 0);          // clear the old alien

			if (alienX == 7) alienDir = -1;        // bounce at the right edge
			if (alienX == 0) alienDir = 1;         // bounce at the left edge
			alienX += alienDir;

			SetPixel(alienX, 0, 255, 0, 0);        // draw the alien at its new position
		}

		// Bullet
		if (bulletActive) {
			SetPixel(bulletX, bulletY, 0, 0, 0);   // clear the old bullet

			if (bulletY == 1) {
				// check whether it hit or miss the alien
				bulletActive = 0;
				if (bulletX == alienX) alienHit();
			} else {
				bulletY--;                                 // move up one row
				SetPixel(bulletX, bulletY, 0, 255, 0);     // draw the green bullet
			}
		}
	}



	//****END OF OWN CODE*****************
	//clear timer interrupt status. DO NOT REMOVE
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status){
	//****Write code here ****

	//Hint: Status==0x01 ->btn0, Status==0x02->btn1, Status==0x04->btn2, Status==0x08-> btn3, Status==0x10->SW0, Status==0x20 -> SW1

	// SW0: restart the game (works any time, also on the end screen)
	if (Status == 0x10) {
		resetGame();
	}

	if (!gameOver) {
		// btn0: move ship right
		if(Status == 0x01 && shipX < 6){	// ship is 3 pixels wide, so 6 is the rightmost position
			drawShip(shipX, 0, 0, 0);    	// clear old ship
			shipX++;
			drawShip(shipX, 0, 0, 255);  	// draw ship at new position
		}

		// btn1: move ship left
		if (Status == 0x02 && shipX > 1) {     // ship is 3 pixels wide, so 1 is the leftmost position
			drawShip(shipX, 0, 0, 0);
			shipX--;
			drawShip(shipX, 0, 0, 255);
		}

		// btn2: fire (only one bullet at a time)
		if (Status == 0x04 && !bulletActive) {
			bulletX = shipX;                   // straight above the ship's top pixel
			bulletY = 5;                       // ship top is row 6, so the bullet starts at row 5
			bulletActive = 1;
			SetPixel(bulletX, bulletY, 0, 255, 0);
		}

	}

	//****END OF OWN CODE*****************
}


