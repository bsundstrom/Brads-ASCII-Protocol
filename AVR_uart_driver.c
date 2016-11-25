/******************************************************************************
*	mega8_uart.c
*
*	This code implements a driver for the UART on Atmel AVR Mega48/88/168
*
*	Written by: Brad Sundstrom
*
*	Target: Atmel ATmega32
*	Compiler: IAR
*
*	Required Support Files:
*	std_data_types_avr_iar.h - defines common datatypes (written by Brad S.)
******************************************************************************/
#include "AVR_uart_driver.h"

void InitializeUART0(void)
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(UART0_BAUD>>8);
	UBRR0L = (unsigned char)UART0_BAUD;
	UCSR0A = 0;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1 << RXCIE0);
	/* Set frame format: 8data, 1 stop bit, no parity */
	UCSR0C = (3<<UCSZ00) | (1<<UPM01); //URSEL must be set when writting this UCSRC
}


/******************************************************************************
*	This ISR fires as soon as the UART has received a byte
******************************************************************************/
#pragma vector = USART0_RX_vect
__interrupt void UART_RCV_COMPLETE(void)
{
#pragma diag_suppress=Pa082
    CPI_CharacterReceived(UDR0,
						UART_CHK_PARITY_ERR() || 
						UART_CHK_FRAMING_ERR() || 
						UART_CHK_DATA_OVERRUN_ERR(), 
						cpi_ascii_packet, 
						&cpi_ascii_packet_ndx);
}

