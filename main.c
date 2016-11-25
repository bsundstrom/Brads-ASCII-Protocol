#include <inavr.h>
#include "std_data_types_avr_iar.h"
#include "Brads_ASCII_Protocol.h"
#include "AVR_uart_driver.h"

int main( void )
{
    InitializeUART0();
    __enable_interrupt();
    DDRD |= (1 << 6);
    ENTER_RECEIVE_MODE();
    
    for(;;)
    {
        uint16_reg[0]++;
        uint16_reg[1]--;
        uint8_reg[0] = (BYTE)(uint16_reg[0] + uint16_reg[1]);
        uint8_reg[1]++;
        //we do nothing 
        if(comm_state == PROCESS_PACKET)
        {
            BAP_ParseRequestPacket(cpi_ascii_packet, cpi_ascii_packet_ndx);    
        }
    }
}



