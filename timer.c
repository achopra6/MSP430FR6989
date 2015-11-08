// UART BackChannel for MSP430FR6989 Launchpad
#include <msp430fr6989.h>

#define SCK 	BIT4    // Serial clock P1.4 SCL
#define CSG		BIT2	//CSG (slave select) P3.2
#define SDOG	BIT7	//SDOG on pin MISO P1.7
#define SDA		BIT6	//SDA   MOSI (SDA) P1.6
#define LED0	BIT0	//LED 0
#define LED1	BIT7	//LED 1

volatile unsigned char RXData = 0;
volatile unsigned char TXData;
int i=0;
void initialize_clock()
{
	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;

	// Startup clock system with max DCO setting ~8MHz
	CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
	CSCTL1 = DCOFSEL_3 | DCORSEL;
	CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
	CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
	CSCTL0_H = 0;                             // Lock CS registers

}

void initialize_uart()
{
	 //Configure GPIO
	P3SEL0 |= BIT4 | BIT5;                    // USCI_A0 UART operation using Backchannel
	P3SEL1 &= ~(BIT4 | BIT5);


	// Configure USCI_A0 for UART mode
	UCA1CTLW0 |= UCSWRST;                      // Put eUSCI in reset
	//UCA1CTLW0 &= ~0x0010;
	UCA1CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
	UCA1BR0 = 52; //9600
	UCA1BR1 = 0x00;
	UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
	UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
}

void initialize_timer()
{
	TA1CCTL0 = CCIE + CAP;						// Timer1_A3
	TA1CCR0 = 0x3E80;						// Count to 16000 , 16000*0.125us = 2ms
	TA1CTL = TASSEL_2 + MC_1 + TAIE;		// Timer MC = 1, TAIE = 1, TASSEL = 2
}

void print_uart(unsigned char *character) {
	while (*character != '\0') {
		while (!(UCA1IFG & UCTXIFG));
    	UCA1TXBUF = *character++;
    	//_delay_cycles(8000);					// Just prints 'H' without delay
	}
}

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    __enable_interrupt();
   // __bis_SR_register(LPM0 + GIE); // LPM0 with interrupts enabled
    __no_operation();
    initialize_clock();
    initialize_uart();
    initialize_timer();


    P1OUT |= LED0;
	P1DIR |= LED0; //LED

	P9OUT |= LED1;
	P9DIR |= LED1;

	P1OUT ^= BIT0;
	TXData = 0x01;
	UCB0TXBUF = TXData;
	for(;;) {

	//	print_uart("Hello World!\n");
	//	_delay_cycles(1000000);

		P9OUT ^= BIT7;				// Toggle P1.0 using exclusive-OR
	}
	return 0;
}

// Timer A3 interrupt service routine
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1 (void)
{
	P1OUT ^= BIT0;				// Toggle P1.0 using exclusive-OR
	switch(TA1IV);				// Read and Clear Interrupt flags
}

