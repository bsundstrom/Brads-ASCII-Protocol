/******************************************************************************
*   The following macros are target-specific and may need to be changed...
******************************************************************************/
#ifndef AVR_UART_DRIVER_H
#define AVR_UART_DRIVER_H

#include <inavr.h>
#include <iom324P.h>
#include "std_data_types_avr_iar.h"
#include "AVR_uart_driver.h"
#include "CPI_ASCII_Protocol.h"

#define MBS_RS485 1

#define BAUDRATE 19200UL
#define FOSC 20000000UL

#define UART0_BAUD (FOSC / (16 * BAUDRATE) - 1)

#define UART_XMIT_BYTE(x) (UDR0 = x)
#define UART_ENABLE_UART_DATA_EMPTY_INT() (UCSR0B |= (1 << UDRIE0))
#define UART_DISABLE_UART_DATA_EMPTY_INT() (UCSR0B &= ~(1 << UDRIE0))
#define UART_ENABLE_UART_XMIT_CMPLT_INT() (UCSR0B |= (1 << TXCIE0))
#define UART_DISABLE_UART_XMIT_CMPLT_INT() (UCSR0B &= ~(1 << TXCIE0))

//this function is application specific and should clear the RS485 tx enable pin
#define ENTER_RECEIVE_MODE() (PORTD &= ~(1 << 6))
#define ENTER_TRANSMIT_MODE() (PORTD |= (1 << 6))

//This will return true if there is unread data in the uart data reg
#define UART_CHK_RX_CMPLT() (UCSR0A & (1 << RXC0))

//This will return true if the data in tx reg has been shifted out
#define UART_CHK_TX_CMPLT() (UCSR0A & (1 << TXC0))

//This will return true if the data reg is empty (ready for data)
#define UART_CHK_DATA_REG_EMPTY() (UCSR0A & (1 << UDRE0))

//this will return true if a framing error has occurred
#define UART_CHK_FRAMING_ERR() (UCSR0A & (1 << FE0))

//this will return true if a parity error has occurred
#define UART_CHK_PARITY_ERR() (UCSR0A & (1 << UPE0))

//this will return true if a data overrun error has occurred
#define UART_CHK_DATA_OVERRUN_ERR() (UCSR0A & (1 << DOR0))

void InitializeUART0(void);
#pragma vector = USART0_TX_vect
__interrupt void UART_TX_COMPLETE(void);
#pragma vector = USART0_UDRE_vect	
__interrupt void UART_TX_REG_EMPTY(void);
#pragma vector = USART0_RX_vect
__interrupt void UART_RCV_COMPLETE(void);
#endif
