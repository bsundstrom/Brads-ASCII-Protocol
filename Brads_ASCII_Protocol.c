/******************************************************************************
*	BAP ASCII Communications Protocol
*	
*	This code implements the BAP ASCII protocol as specified in the BAP ASCII
*	Protocol Specification Revision D.
*	
*	Written By: Brad Sundstrom - Control Products, Inc. - 2007
*	
******************************************************************************/
#include "BAP_ASCII_Protocol.h"

BYTE cpi_ascii_packet[MAX_PACKET_LEN];
BYTE cpi_ascii_packet_ndx;

//if a size has been defined for uint8_reg array, then allocate memory for it.
#ifdef UINT8_REG_SZ
UINT8 uint8_reg[UINT8_REG_SZ];
#endif

//if a size has been defined for uint8_reg array, then allocate memory for it.
#ifdef INT8_REG_SZ
INT8 int8_reg[INT8_REG_SZ];
#endif

//if a size has been defined for uint16_reg array, then allocate memory for it.
#ifdef UINT16_REG_SZ
UINT16 uint16_reg[UINT16_REG_SZ];
#endif

//if a size has been defined for int16_reg array, then allocate memory for it.
#ifdef INT16_REG_SZ
INT16 int16_reg[INT16_REG_SZ];
#endif

comm_state_t comm_state;

void BAP_CharacterReceived(BYTE data, BYTE error, BYTE *packet, BYTE *packet_len)
{	
	if(error)
	{
		comm_state = WAIT_FOR_START_CHAR;
	}
	else
	{
		switch(comm_state)
		{
			case WAIT_FOR_START_CHAR:
				if(data == ':')
				{
					packet[0] = data;
					*packet_len = 1;
					comm_state = RECEIVING_PACKET;
				}
				break;
			
			case RECEIVING_PACKET:
                if(data == ':') //restart receiving buffer
                {
					packet[0] = data;
					*packet_len = 1;
                }
                else
                {
                    packet[(*packet_len)++] = data;
                    if(data == 0x0D) //0x0D is CR char
                        comm_state = WAIT_FOR_LINEFEED;
                }
                break;
			
			case WAIT_FOR_LINEFEED:
			if(data == 0x0A) //0x0A is LF char
			{
				packet[(*packet_len)++] = data;
				comm_state = PROCESS_PACKET;
			}
			else
				comm_state = WAIT_FOR_START_CHAR;
				break;
			
			//if we receive data while processing/sending data, ignore it
			case PROCESS_PACKET:
			case SEND_RESPONSE:
				break;
		}	
	}
}

/******************************************************************************
*	ConvertAsciiHex2ByteValue calculates the byte value of the 2-byte ASCII 
*	hex value that is pointed to by *ascii_bytes. For instance,
*	if ascii_bytes[0] = 0x09, ascii_bytes[1] = 0x0C, then this
*	this function saves a value of 0x9C (decimal 156) to the location *value.
*	If the ascii_bytes do not represent a valid hex number, then the function
*	returns zero, else it returns 1.
******************************************************************************/
BYTE BAP_ConvertAsciiHex2ByteValue(BYTE *ascii_bytes, BYTE *value)
{	
	BYTE temp_val;	
	temp_val = BAP_ConvertChar2Nibble(ascii_bytes[0]);
	if(temp_val != 0xFF)
	{
		*value = temp_val << 4;
		temp_val = BAP_ConvertChar2Nibble(ascii_bytes[1]);
		if(temp_val != 0xFF)
		{
			*value += temp_val;
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

/******************************************************************************
*	ConvertByteValue2AsciiHex converts the byte val passed as val into 2 ascii
*	characters which represent the value in hex format.
*	for example, 
*	if val = 156 (0x9C), then ascii_bytes[0] = 0x39 and ascii_bytes[1] = 0x43
******************************************************************************/
void BAP_ConvertByteValue2AsciiHex(BYTE val, BYTE *ascii_bytes)
{
	ascii_bytes[0] = BAP_ConvertNibble2Char(val >> 4);
	ascii_bytes[1] = BAP_ConvertNibble2Char(val);
}

/******************************************************************************
*	ConvertNibble2Char converts the lower nibble of the byte passed to it into
*	a hex character represented in ASCII encoding.
*	For example, 
*		if nibble = 0x07, this function returns 0x37 (decimal 55)
******************************************************************************/
BYTE BAP_ConvertNibble2Char(BYTE nibble)
{
    char ret_val;
    nibble &= 0x0F;
    if(nibble < 10)
        ret_val = nibble + 48;
    else
        ret_val = nibble + 55;
	return ret_val;
}

/******************************************************************************
*	ConvertChar2Nibble converts the ascii character code that is passed to it
*	into it's decimal reprentation. This function only accepts ASCII characters
*	0 - 9, A - F, and a - f. If any other characters are passed in, this 
*	function will return 0xFF to signal an invalid character.
*	for example,
*		if asc_char = 'A' (0x41), this function returns 0x0A
*		if asc_char = 'G' (0x47), this function returns 0xFF (Invalid char)
******************************************************************************/	
BYTE BAP_ConvertChar2Nibble(BYTE asc_char)
{
    BYTE ret_val;
    if(asc_char < 58 && asc_char > 47)
        ret_val = asc_char - 48;
    else if(asc_char > 64 && asc_char < 71)
        ret_val = asc_char - 55;
    else
        ret_val = 0xFF; //signal invalid char
	return ret_val;
}

void BAP_ParseRequestPacket(BYTE *packet, BYTE packet_length)
{
    if(packet[START_CHAR_POS] == ':')
    {
        //this indicates the start of a request packet
        BAP_ProcessRequest(packet, &packet_length);
		comm_state = SEND_RESPONSE;
        BAP_SendResponsePacket(packet, packet_length);
    }
}

/******************************************************************************
*	ProcessRequest() parses the packet and formats a response. The response
*	packet overwrites the request packet (uses the same buffer).
*	A pointer to the packet is passed to this function, along with a pointer to
*	the number of bytes in this packet (including the start char and end chars)
******************************************************************************/
void BAP_ProcessRequest(BYTE *packet, BYTE *packet_length)
{
	BYTE function_code;
    packet[START_CHAR_POS] = '|';
	//check if packet is too long
    if(*packet_length > MAX_PACKET_LENGTH)
    {
		BAP_ConstructErrorPacket(packet, packet_length, RX_PACKET_TOO_LONG);
    }
	else if(*packet_length % 2 == 0)
	{
		BAP_ConstructErrorPacket(packet, packet_length, INVALID_PACKET_LENGTH);
	}
    //Notice here we are figuring out what the function code is
    //At the same time we check that the function code is a valid hex number
	else if(BAP_ConvertAsciiHex2ByteValue(packet + FUNCTION_CHAR_POS, &function_code) != 0)
	{   //looks like a good packet, process it 
        BAP_DoRequest(function_code, packet, packet_length);
	}
	else	//the function code has invalid chars
	{
		BAP_ConstructErrorPacket(packet, packet_length, INVALID_CHARACTER);
	}
    //check if there is room to add the CR+LF
    if(*packet_length + 2 < MAX_PACKET_LEN) 
    {
        packet[(*packet_length)++] = 0x0D;
        packet[(*packet_length)++] = 0x0A;
    }
    else
    {
        BAP_ConstructErrorPacket(packet, packet_length, TX_PACKET_TOO_LONG);
        packet[(*packet_length)++] = 0x0D;
        packet[(*packet_length)++] = 0x0A;        
    }
}

/******************************************************************************
*	This function sends the entire response packet.
******************************************************************************/
void BAP_SendResponsePacket(BYTE *packet, BYTE packet_length)
{
    
	UINT16 ndx;
#ifdef MBS_RS485    
    ENTER_TRANSMIT_MODE();
#endif
	for(ndx = 0; ndx <= packet_length - 1; ndx++)
	{   
	    UART_XMIT_BYTE(packet[ndx]);
        while(!UART_CHK_DATA_REG_EMPTY());//wait for byte to be shifted from UDR
	}
    comm_state = WAIT_FOR_START_CHAR;
#ifdef MBS_RS485
    while(!UART_CHK_TX_CMPLT()); //wait for byte to be transmitted
    //now we need to delay for the parity bit to shift out
    for(ndx = FOSC / 19200; ndx > 0; ndx--);
    //give the stop bit time to be transmitted
    for(ndx = FOSC / 19200; ndx > 0; ndx--);
    ENTER_RECEIVE_MODE();
#endif
}

/******************************************************************************
*	ConstructErrorPacket() simply constructs an error packet based on the error
*	code that is passed to the function.
******************************************************************************/
void BAP_ConstructErrorPacket(BYTE *packet, BYTE *packet_length, BYTE error_code)
{
	packet[START_CHAR_POS] = '!'; //signal an error packet
	BAP_ConvertByteValue2AsciiHex(error_code, packet + DATA_DESCRIPTOR_POS);
	*packet_length = 5;	
}

/******************************************************************************
*   BAP_DoRequest() is where we actually carry out commands received from the 
*   master. This may include returning data or application specific stuff.
******************************************************************************/
void BAP_DoRequest(BYTE function_code, BYTE *packet, BYTE *packet_length)
{
    *packet_length = *packet_length - 2; //remove the CR + LF characters
    switch(function_code)
    {
    case 0:
        //echo the packet, don't change anything
        break;

    case 1: //Read Data Register
        BAP_ReadDataRegister(packet, packet_length);
        break;
        
    case 3: //Read Data Register w/ CRC Chksum
        BAP_ReadDataRegister(packet, packet_length);
        temp_uint16 = BAP_GetCRC(packet, packet_length);
        packet[packet_length++] = temp_uint16 >> 
        break;
        
    default:
        //this is an unsupported function
        BAP_ConstructErrorPacket(packet, packet_length, UNSUPPORTED_FUNCTION);
        break;
    }    
}

void BAP_ReadDataRegister(BYTE *packet, BYTE *packet_length)
{
    BYTE data_descriptor;
    BYTE temp_byte_low, temp_byte_high;
    UINT16 reg_num;
    //Decode the data descriptor byte
	BAP_ConvertAsciiHex2ByteValue(packet + DATA_DESCRIPTOR_POS, &data_descriptor);    
    //figure out what kind of data type to process
    switch(data_descriptor)
    {
#ifdef UINT8_REG_SZ
    case 0: //UINT8
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS, &temp_byte_high);
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS + 1, &temp_byte_low);
        reg_num = temp_byte_high << 8 + temp_byte_low;
        if(reg_num < UINT8_REG_SZ)
        {
            BAP_ConvertByteValue2AsciiHex(uint8_reg[reg_num], packet + DATA_START_POS);
            *packet_length = 7; //start char + function chars + data_des chars. + value
        }
        else //a register has been requested that does not exist
            BAP_ConstructErrorPacket(packet, packet_length, INVALID_DATA);
        break;
#endif
        
#ifdef INT8_REG_SZ
    case 1: //INT8 (BYTE)
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS, &temp_byte_high);
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS + 1, &temp_byte_low);
        reg_num = temp_byte_high << 8 + temp_byte_low;
        if(reg_num < INT8_REG_SZ)
        {
            BAP_ConvertByteValue2AsciiHex(int8_reg[reg_num], packet + DATA_START_POS);
            *packet_length = 7; //start char + function chars + data_des chars. + value
        }
        else //a register has been requested that does not exist
            BAP_ConstructErrorPacket(packet, packet_length, INVALID_DATA);
        break;
#endif        
        
#ifdef UINT16_REG_SZ
    case 2: //UINT16
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS, &temp_byte_high);
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS + 1, &temp_byte_low);
        reg_num = temp_byte_high << 8 + temp_byte_low;
        if(reg_num < UINT16_REG_SZ)
        {
            temp_byte_high = uint16_reg[reg_num] >> 8;
            BAP_ConvertByteValue2AsciiHex(temp_byte_high, packet + DATA_START_POS);
            temp_byte_high = (BYTE)uint16_reg[reg_num];
            BAP_ConvertByteValue2AsciiHex(temp_byte_high, packet + DATA_START_POS + 1);        
            *packet_length = 8; //start char + function chars + data_des chars. + value
        }
        else //a register has been requested that does not exist
            BAP_ConstructErrorPacket(packet, packet_length, INVALID_DATA);
        break;      
#endif
        
#ifdef INT16_REG_SZ
    case 3: //INT16
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS, &temp_byte_high);
        BAP_ConvertAsciiHex2ByteValue(packet + DATA_START_POS + 1, &temp_byte_low);
        reg_num = temp_byte_high << 8 + temp_byte_low;
        if(reg_num < INT16_REG_SZ)
        {
            temp_byte_high = int16_reg[reg_num] >> 8;
            BAP_ConvertByteValue2AsciiHex(temp_byte_high, packet + DATA_START_POS);
            temp_byte_high = (BYTE)int16_reg[reg_num];
            BAP_ConvertByteValue2AsciiHex(temp_byte_high, packet + DATA_START_POS + 1);        
            *packet_length = 8; //start char + function chars + data_des chars. + value
        }
        else //a register has been requested that does not exist
            BAP_ConstructErrorPacket(packet, packet_length, INVALID_DATA);
        break;      
#endif        
        
    default: //we do not recognize the data descriptor field
        BAP_ConstructErrorPacket(packet, packet_length, INVALID_DATA_DESCRIPTOR);
        break;
    }    
}

#ifdef SUPPORT_CRC_CHECKSUM
/******************************************************************************
*	BAP_GetCRC() returns the CRC of the buffer that is passed to it.
******************************************************************************/
UINT16 BAP_GetCRC(volatile BYTE *buffer, BYTE buffer_size)
{
    BYTE i;
    BYTE j;
    BYTE lsbit;
	UINT16 crc_value = INIT_REMAINDER;

	for (i = 0; i < buffer_size; i++)
    {
        crc_value ^= buffer[i];
        for (j = 0; j < 8; j++)
	    {		
            lsbit = crc_value & 0x01;
            crc_value >>= 1;
            if (lsbit)
                crc_value ^= 0xA001;		
        }
	}
    crc_value ^= FINAL_XOR;
	return crc_value;
}
#endif
