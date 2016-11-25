#ifndef BAP_ASCII_PROTOCOL_H
#define BAP_ASCII_PROTOCOL_H

#include <inavr.h>
#include "std_data_types_avr_iar.h"
#include "AVR_uart_driver.h"

#define MAX_PACKET_LEN 19 //this should always be odd and at least 7
extern BYTE cpi_ascii_packet[MAX_PACKET_LEN];
extern BYTE cpi_ascii_packet_ndx;

//comment the #define below if not supporting CRC checksums
#define SUPPORT_CRC_CHECKSUM 1

//comment the #define below if not using uint8 registers
#define UINT8_REG_SZ 8 //define the number of uint8 registers available

#ifdef UINT8_REG_SZ
extern UINT8 uint8_reg[UINT8_REG_SZ];
#endif

//comment the #define below if not using uint8 registers
#define INT8_REG_SZ 8 //define the number of uint8 registers available

#ifdef INT8_REG_SZ
extern INT8 int8_reg[INT8_REG_SZ];
#endif

//comment the #define below if not using uint8 registers
#define UINT16_REG_SZ 8 //define the number of uint8 registers available

#ifdef UINT16_REG_SZ
extern UINT16 uint16_reg[UINT16_REG_SZ];
#endif

//comment the #define below if not using uint8 registers
#define INT16_REG_SZ 8 //define the number of uint8 registers available

#ifdef INT16_REG_SZ
extern INT16 int16_reg[INT16_REG_SZ];
#endif

typedef enum
{
    UNSUPPORTED_FUNCTION,
    INVALID_DATA_DESCRIPTOR,
    INVALID_DATA,
    RX_PACKET_TOO_LONG,
    INVALID_CHARACTER,
	INVALID_PACKET_LENGTH,
    TX_PACKET_TOO_LONG
} error_types_t;

typedef enum
{
    START_CHAR_POS = 0,
    FUNCTION_CHAR_POS = 1,
    DATA_DESCRIPTOR_POS = 3,
    DATA_START_POS = 5,
} character_positions_t;

typedef enum
{
	WAIT_FOR_START_CHAR,
	RECEIVING_PACKET,
	WAIT_FOR_LINEFEED,
	PROCESS_PACKET,
	SEND_RESPONSE
} comm_state_t;

extern comm_state_t comm_state;


#define MAX_PACKET_LENGTH 20
extern BYTE packet[MAX_PACKET_LENGTH];

void BAP_CharacterReceived(BYTE data, BYTE error, BYTE *parcket, BYTE *packet_len);
BYTE BAP_ConvertAsciiHex2ByteValue(BYTE *ascii_bytes, BYTE *value);
void BAP_ConvertByteValue2AsciiHex(BYTE val, BYTE *ascii_bytes);
BYTE BAP_ConvertNibble2Char(BYTE nibble);
BYTE BAP_ConvertChar2Nibble(BYTE asc_char);
void BAP_ParseRequestPacket(BYTE *packet, BYTE packet_length);
void BAP_ProcessRequest(BYTE *packet, BYTE *packet_length);
void BAP_SendResponsePacket(BYTE *packet, BYTE packet_length);
void BAP_ConstructErrorPacket(BYTE *packet, BYTE *packet_length, BYTE error_code);
void BAP_DoRequest(BYTE function_code, BYTE *packet, BYTE *packet_length);
void BAP_ReadDataRegister(BYTE *packet, BYTE *packet_length);

#ifdef SUPPORT_CRC_CHECKSUM
UINT16 BAP_GetCRC(volatile BYTE *buffer, BYTE buffer_size);
#endif

#endif
