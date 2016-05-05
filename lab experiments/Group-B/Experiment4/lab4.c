#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"

uint32_t fuck = 0;

uint32_t ui32ADC0Value[4];
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;
unsigned int input;

void getTemperature()
{
	ROM_ADCIntClear(ADC0_BASE, 1);
	ROM_ADCProcessorTrigger(ADC0_BASE, 1);
	while(!ROM_ADCIntStatus(ADC0_BASE, 1, false))
	{

	}
	ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
	ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
	ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
	ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
}

uint32_t mode = 0, setTemp = 25, ledVal = 2, inputMode = 0;

void UARTIntHandler(void)
{
	fuck = fuck + 1;
	getTemperature();
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
	UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
	{
		unsigned char curr = UARTCharGet(UART0_BASE);
		if (curr == 'S')
		{
			mode = 1;
			inputMode = 1;
		}
		else if (mode == 1)
		{
			setTemp = curr - '0';
			mode = 2;
		}
		else if (mode == 2)
		{
			setTemp = setTemp*10 + curr - '0';
			mode = 3;
		}
		else if (mode == 3)
		{
			mode = 0;
			inputMode = 3;
		}

		if (setTemp < ui32TempValueC)
		{
			ledVal = 2;
		}
		else
		{
			ledVal = 8;
		}

//		UARTCharPut(UART0_BASE, UARTCharGet(UART0_BASE)); //echo character

//		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
//		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
}

void ledPinConfig(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);  // Pin-1 of PORT F set as output. Modifiy this to use other 2 LEDs.
}

int main(void) {

	ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);
	ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 1);

	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); //enable pin for LED PF2
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	IntMasterEnable(); //enable processor interrupts
	IntEnable(INT_UART0); //enable the UART interrupt

	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT); //only enable RX and TX interrupts
//	Current Temperature XX ºC

	ledPinConfig();

	unsigned char xx, yy;
	while (1) //let interrupt handler do the UART echo function
	{
		getTemperature();
		if (inputMode == 0)
		{
			UARTCharPut(UART0_BASE,'C');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'n');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'T');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'m');
			UARTCharPut(UART0_BASE,'p');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'a');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,' ');

			xx = ui32TempValueC % 10 + '0';
			yy = ui32TempValueC / 10 + '0';

			UARTCharPut(UART0_BASE, yy);
			UARTCharPut(UART0_BASE, xx);

			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE, 168);
			UARTCharPut(UART0_BASE,'C');
			UARTCharPut(UART0_BASE,',');
			UARTCharPut(UART0_BASE,' ');

			UARTCharPut(UART0_BASE,'S');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'T');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'m');
			UARTCharPut(UART0_BASE,'p');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'a');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,' ');

			xx = setTemp % 10 + '0';
			yy = setTemp / 10 + '0';

			UARTCharPut(UART0_BASE, yy);
			UARTCharPut(UART0_BASE, xx);

			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE, 168);
			UARTCharPut(UART0_BASE,'C');
			UARTCharPut(UART0_BASE,',');
			UARTCharPut(UART0_BASE,'\n');

		}
		else if (inputMode == 1)
		{
			inputMode = 2;
			UARTCharPut(UART0_BASE,'E');
			UARTCharPut(UART0_BASE,'n');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'T');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'m');
			UARTCharPut(UART0_BASE,'p');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'a');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'\n');
		}
		else if (inputMode == 2)
		{

		}
		else
		{
			inputMode = 0;
			UARTCharPut(UART0_BASE, '\r');
			UARTCharPut(UART0_BASE, '\n');
			UARTCharPut(UART0_BASE,'S');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'T');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'m');
			UARTCharPut(UART0_BASE,'p');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'a');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'r');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'u');
			UARTCharPut(UART0_BASE,'p');
			UARTCharPut(UART0_BASE,'d');
			UARTCharPut(UART0_BASE,'a');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'e');
			UARTCharPut(UART0_BASE,'d');
			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE,'t');
			UARTCharPut(UART0_BASE,'o');
			UARTCharPut(UART0_BASE,' ');

			xx = setTemp % 10 + '0';
			yy = setTemp / 10 + '0';

			UARTCharPut(UART0_BASE, yy);
			UARTCharPut(UART0_BASE, xx);

			UARTCharPut(UART0_BASE,' ');
			UARTCharPut(UART0_BASE, 168);
			UARTCharPut(UART0_BASE,'C');
			UARTCharPut(UART0_BASE,',');
			UARTCharPut(UART0_BASE,'\n');
		}
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ledVal);
		SysCtlDelay(10000000);
	}
//	{
		//
		input ++;
		unsigned char inputChar = UARTCharGet(UART0_BASE);
		input = inputChar;
		input ++;

		if (UARTCharsAvail(UART0_BASE)) UARTCharPut(UART0_BASE, inputChar);
//	}

}
