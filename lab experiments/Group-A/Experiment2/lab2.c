/**
	Team Id: 4
	Author List: 
		Rohit Kumar, 120050028
		Suman Sourabh, 120050031

		File name: lab2.c
		Theme : Timer Interrupts
		Functions: 
			setup()
			ledPinConfig()
			switchPinConfig()
			detectKeyPressSwitch1()
			detectKeyPressSwitch2()

		Global Variables: 
			switch1State
			switch2State
			risingEdge1
			risingEdge2
			sw2counter
			ledVal
 **/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))
/*
 ------ Global Variable Declaration
*/
/*
* Function Name: setup()
* Input: none
* Output: none
* Description: Set crystal frequency and enable GPIO Peripherals  
* Example Call: setup();
*/
void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
}
/*
* Function Name: ledPinConfig()
* Input: none
* Output: none
* Description: Set PORTF Pin 1, Pin 2, Pin 3 as output.
* Example Call: ledPinConfig();
*/
void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.
}
/*
* Function Name: switchPinConfig()
* Input: none
* Output: none
* Description: Set PORTF Pin 0 and Pin 4 as input. Note that Pin 0 is locked.
* Example Call: switchPinConfig();
*/
void switchPinConfig(void)
{
	// Following two line removes the lock from SW2 interfaced on PORTF Pin0 -- leave this unchanged
	LOCK_F=0x4C4F434BU;
	CR_F=GPIO_PIN_0|GPIO_PIN_4;
	// GPIO PORTF Pin 0 and Pin4
	GPIODirModeSet(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4,GPIO_DIR_MODE_IN); // to ask GPIO_PORTF_BASE
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4 );
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4,GPIO_STRENGTH_12MA,GPIO_PIN_TYPE_STD_WPU);
}

unsigned int switch1State = 0, switch2State = 0; // 0 idle, 1 press, 2 release
uint8_t risingEdge1 = 0, risingEdge2 = 0;

/*

* Function Name: detectKeyPressSwitch1()
* Input: none
* Output: unsigned char which denotes whether switch 1 is pressed or not
* Description: Uses FSM of switch clicks to detect presses and adjusts intensity of LEDs
* Example Call: detectKeyPressSwitch1();

*/
unsigned char detectKeyPressSwitch1()
{
	unsigned char flag = 0;
	uint8_t D0 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	if(D0&16)		// true when the SW1 isn't pressed
	{
//		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		if(risingEdge1 == 1)
		{
			switch1State = 0;
		}
		risingEdge1 = 0;
	}
	else
	{
//		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ledVal);
		risingEdge1 = 1;
		if (switch1State == 0) {
			switch1State = 1;
		}
		else if (switch1State == 1) {
			switch1State = 2;
			flag = 1;
		}
	}
	return flag;
}

/*

* Function Name: detectKeyPressSwitch2()
* Input: none
* Output: unsigned char which denotes whether switch 2 is pressed or not
* Description: Uses FSM of switch clicks to detect presses and adjusts intensity of LEDs
* Example Call: detectKeyPressSwitch1();

*/
unsigned char detectKeyPressSwitch2()
{
	unsigned char flag = 0;
	uint8_t D0 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	if(D0&1)		// true when the SW1 isn't pressed
	{
//		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
		if(risingEdge2 == 1)
		{
			switch2State = 0;
		}
		risingEdge2 = 0;
	}
	else
	{
//		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ledVal);
		risingEdge2 = 1;
		if (switch2State == 0) {
			switch2State = 1;
		}
		else if (switch2State == 1) {
			switch2State = 2;
			flag = 1;
		}
	}
	return flag;
}
uint32_t sw2counter = 0;
uint8_t ledVal = 8;

/*
* Function Name: Timer0IntHandler
* Input:
* Output: 
* Logic: 
	Timer 0 Interrupt Handler
	In each timer interrupt 
		temperature, motion and proximity sensors are sensed
* Example Call:
	Timer0IntHandler()

*/
void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Read the current state of the GPIO pin and
	// write back the opposite state
//	sw2counter ++;
	if (detectKeyPressSwitch1()) {
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ledVal);
		if(ledVal == 8)
		{
			ledVal = 2;
		}
		else
		{
			ledVal = ledVal*2;
		}
	}
	else if (switch1State == 0)	// release event
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
	}
	if (detectKeyPressSwitch2())
	{
		sw2counter ++;
	}
}
int main(void)
{
	setup();
	ledPinConfig();
	switchPinConfig();
	uint32_t ui32Period;
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ui32Period = (SysCtlClockGet() / 100) / 2;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
	/*---------------------
		* Write your code here
		* You can create additional functions
	---------------------*/
}