#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


// LOCK_F and CR_F - used for unlocking PORTF pin 0
#define LOCK_F (*((volatile unsigned long *)0x40025520))
#define CR_F   (*((volatile unsigned long *)0x40025524))


#define PWM_FREQUENCY 55

enum LEDState{
	RED_INC, GREEN_INC, BLUE_INC
};

enum State{
	START, DUMMY, S1P, S2P, S2PS1P, S2PS1R, S2PS1P2, S2PS1R2, S2PS1LP, S2PS1LR
};


unsigned int switch1State = 0, switch2State = 0; // 0 idle, 1 press, 2 release
uint8_t risingEdge1 = 0, risingEdge2 = 0;
uint32_t pressCount1 = 1;

unsigned char detectKeyPressSwitch1()
{
	unsigned char flag = 0;
	uint8_t D0 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	if(D0&16)		// true when the SW1 isn't pressed
	{
		if(risingEdge1 == 1)
		{
			switch1State = 0;
			flag = 2;
			pressCount1 = 0;
		}
		risingEdge1 = 0;
	}
	else
	{
		risingEdge1 = 1;
		if (switch1State == 0) {
			switch1State = 1;
		}
		else if (switch1State == 1) {
			switch1State = 2;
			flag = 1;
		}
		else
		{
			pressCount1++;
		}
	}
	return flag;
}

unsigned char detectKeyPressSwitch2()
{
	unsigned char flag = 0;
	uint8_t D0 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	if(D0&1)		// true when the SW1 isn't pressed
	{
		if(risingEdge2 == 1)
		{
			switch2State = 0;
			flag = 2;
		}
		risingEdge2 = 0;
	}
	else
	{
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


enum State currentState = START;
int mode = 0; // auto 0, mode 1,2,3
unsigned int sw2counter= 0;
enum LEDState led_state = GREEN_INC;
uint32_t ui32Period;
int32_t ui8Red;
int32_t ui8Blue;
int32_t ui8Green;
volatile uint32_t ui32Load;
uint32_t freq = 1;
uint32_t diff = 20;

void Timer0IntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Read the current state of the GPIO pin and
	// write back the opposite state

	sw2counter ++;


	switch(currentState)
	{
		case START:
			if (detectKeyPressSwitch2() == 1)
			{
				currentState = S2P;
			}
			if (detectKeyPressSwitch1() == 1)
			{
				currentState = S1P;
			}
			break;
		case S1P:
			if (detectKeyPressSwitch1() == 2)
			{
				currentState = START;
				if (mode == 0)
				{
					freq ++;
					if (freq > 20)
					{
						freq = 20;
					}
				}else if(mode == 1){
					ui8Red+=diff;
					if(ui8Red >= 255){
						ui8Red = 255;
					}
				} else if(mode == 2){
					ui8Blue+=diff;
					if(ui8Blue >= 255){
						ui8Blue = 255;
					}
				} else if(mode == 3){
					ui8Green+=diff;
					if(ui8Green >= 255){
						ui8Green = 255;
					}
				}
			}
			break;
		case S2P:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
				if (mode == 0)
				{
					freq--;
					if (freq < 1)
					{
						freq = 1;
					}
				}else if(mode == 1){
					ui8Red-=diff;
					if(ui8Red <= 4){
						ui8Red = 4;
					}
				} else if(mode == 2){
					ui8Blue-=diff;
					if(ui8Blue <= 4){
						ui8Blue = 4;
					}
				} else if(mode == 3){
					ui8Green-=diff;
					if(ui8Green <= 4){
						ui8Green = 4;
					}
				}

			}
			else if (detectKeyPressSwitch1() == 1)
			{
				currentState = S2PS1P;
			}
			break;
		case S2PS1P:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
			}
			else if (detectKeyPressSwitch1() == 2)
			{
				currentState = S2PS1R;
			}
			else
			{
				detectKeyPressSwitch1();
				if (pressCount1 > 500)
				{
					currentState = S2PS1LP;
				}
			}
			break;
		case S2PS1LP:
			if (detectKeyPressSwitch1() == 2)
			{
				currentState = S2PS1LR;
			}
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
			}
			break;

		case S2PS1LR:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
				mode = 3;
			}
			else if (detectKeyPressSwitch1() == 1)
			{
				currentState = DUMMY;
			}
			break;

		case S2PS1R:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
				mode = 1;
			}
			else if (detectKeyPressSwitch1() == 1)
			{
				currentState = S2PS1P2;
			}
			break;

		case S2PS1P2:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
			}
			else if (detectKeyPressSwitch1() == 2)
			{
				currentState = S2PS1R2;
			}
			break;
		case S2PS1R2:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
				mode = 2;
			}
			else if (detectKeyPressSwitch1() == 1)
			{
				currentState = DUMMY;
			}
			break;
		case DUMMY:
			if (detectKeyPressSwitch2() == 2)
			{
				currentState = START;
			}
			break;

	}

	if(mode == 0)
		switch(led_state){
			case GREEN_INC:
				ui8Green+=freq;
				ui8Red-=freq;
				if(ui8Green >= 255){
					ui8Green = 255;
					ui8Red = 4;
					led_state = BLUE_INC;
				}
				break;
			case BLUE_INC:
				ui8Blue+=freq;
				ui8Green-=freq;
				if(ui8Blue >= 255){
					ui8Blue = 255;
					ui8Green = 4;
					led_state = RED_INC;
				}
				break;
			case RED_INC:
				ui8Red+=freq;
				ui8Blue-=freq;
				if(ui8Red >= 255){
					ui8Red = 255;
					ui8Blue = 4;
					led_state = GREEN_INC;
				}
				break;
		}


		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Red * ui32Load / 1000);
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Blue * ui32Load / 1000);
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Green * ui32Load / 1000);



	/*
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
		sw2counter++;
	}
	*/
}

int main(void)
{
//	setup();
//	ledPinConfig();
//	switchPinConfig();

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


	volatile uint32_t ui32PWMClock;
	ui8Red = 4;
	ui8Blue = 4;
	ui8Green = 254;
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PF1_M1PWM5 );
	GPIOPinConfigure(GPIO_PF2_M1PWM6 );
	GPIOPinConfigure(GPIO_PF3_M1PWM7 );

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Red * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, ui8Blue * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, ui8Green * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

	while(1)
	{
		//SysCtlDelay(100000);
	}

}

