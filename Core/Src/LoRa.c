#include "LoRa.h"
#include <stdio.h>

/* ----------------------------------------------------------------------------- *\
		name        : newLoRa

		description : it's a constructor for LoRa structure that assign default values
									and pass created object (LoRa struct instanse)

		arguments   : Nothing

		returns     : A LoRa object whith these default values:
											----------------------------------------
										  |   carrier frequency = 433 MHz        |
										  |    spreading factor = 7				       |
											|           bandwidth = 125 KHz        |
											| 		    coding rate = 4/5            |
											----------------------------------------
\* ----------------------------------------------------------------------------- */
LoRa newLoRa(){
	LoRa new_LoRa;

	new_LoRa.frequency             = 433       ;
	new_LoRa.spredingFactor        = SF_7      ;
	new_LoRa.bandWidth			   = BW_125KHz ;
	new_LoRa.crcRate               = CR_4_5    ;
	new_LoRa.power				   = POWER_20db;
	new_LoRa.overCurrentProtection = 100       ;
	new_LoRa.preamble			   = 8         ;

	return new_LoRa;
}
/* ----------------------------------------------------------------------------- *\
		name        : LoRa_reset

		description : reset module

		arguments   :
			LoRa* LoRa --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_reset(LoRa* _LoRa){
	HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(_LoRa->reset_port, _LoRa->reset_pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_gotoMode

		description : set LoRa Op mode

		arguments   :
			LoRa* LoRa    --> LoRa object handler
			mode	        --> select from defined modes

		returns     : Nothing
\* ----------------------------------------------------------------------------- */


//data = 0x80;   // ALWAYS keep LoRa mode ON

/*uint8_t    read;
uint8_t    data;

read = LoRa_read(_LoRa, RegOpMode);
if(mode == SLEEP_MODE){
	data = (read & 0xF8) | 0x00;
	_LoRa->current_mode = SLEEP_MODE;
}else if (mode == STNBY_MODE){
	data = (read & 0xF8) | 0x01;
	_LoRa->current_mode = STNBY_MODE;
}else if (mode == TRANSMIT_MODE){
	data = (read & 0xF8) | 0x03;
	_LoRa->current_mode = TRANSMIT_MODE;
}else if (mode == RXCONTIN_MODE){
	data = (read & 0xF8) | 0x05;
	_LoRa->current_mode = RXCONTIN_MODE;
}else if (mode == RXSINGLE_MODE){
	data = (read & 0xF8) | 0x06;
	_LoRa->current_mode = RXSINGLE_MODE;
}
LoRa_write(_LoRa, RegOpMode, data);*/

/*uint8_t data = 0;
    switch(mode)
    {
        case SLEEP_MODE:     data = 0x00; break; // LoRa + Sleep
        case STNBY_MODE:     data = 0x01; break; // LoRa + Standby
        case TRANSMIT_MODE:  data = 0x03; break; // LoRa + TX
        case RXCONTIN_MODE:  data = 0x05; break; // LoRa + RX continuous
        case RXSINGLE_MODE:  data = 0x06; break; // LoRa + RX single
    }

    LoRa_write(_LoRa, RegOpMode, data);
    HAL_Delay(2);
    _LoRa->current_mode = mode;*/

/*
 *
 *
	uint8_t read = LoRa_read(_LoRa, RegOpMode);

	    // 1. Move to Standby first to stabilize if switching to high-power modes
	    if (mode == TRANSMIT_MODE || mode == RXCONTIN_MODE) {
	        uint8_t standby = (read & 0xF8) | 0x01;
	        LoRa_write(_LoRa, RegOpMode, standby);
	        HAL_Delay(1); // Short wait for PLL
	    }

	    // 2. Set the target mode (preserving bits 7-3)
	    uint8_t data = (read & 0xF8) | (mode & 0x07);
	    LoRa_write(_LoRa, RegOpMode, data);

	    _LoRa->current_mode = mode;
 */

void LoRa_gotoMode(LoRa* _LoRa, int mode)	{
	uint8_t    read;
	uint8_t    data;

	read = LoRa_read(_LoRa, RegOpMode);
	data = read;
	//78 - 0111 1000
	if(mode == SLEEP_MODE){
		data = (read & 0xF8) | 0x00;
		_LoRa->current_mode = SLEEP_MODE;
	}else if (mode == STNBY_MODE){
		data = (read & 0xF8) | 0x01;
		_LoRa->current_mode = STNBY_MODE;
	}else if (mode == TRANSMIT_MODE){
		data = (read & 0xF8) | 0x03;
		_LoRa->current_mode = TRANSMIT_MODE;
	}else if (mode == RXCONTIN_MODE){
		data = (read & 0xF8) | 0x05;
		_LoRa->current_mode = RXCONTIN_MODE;
	}else if (mode == RXSINGLE_MODE){
		data = (read & 0xF8) | 0x06;
		_LoRa->current_mode = RXSINGLE_MODE;
	}

	LoRa_write(_LoRa, RegOpMode, data);

	/*printf("After goto %d Mode: 0x%X\r\n", _LoRa->current_mode, LoRa_read(_LoRa, RegOpMode));
	//HAL_Delay(10);
	 if (mode == TRANSMIT_MODE){
	printf("After Transmit: 0x%X\r\n", LoRa_read(_LoRa, RegOpMode));
	 }*/
}


/* ----------------------------------------------------------------------------- *\
		name        : LoRa_readReg

		description : read a register(s) by an address and a length,
									then store value(s) at outpur array.
		arguments   :
			LoRa* LoRa        --> LoRa object handler
			uint8_t* address  -->	pointer to the beginning of address array
			uint16_t r_length -->	detemines number of addresse bytes that
														you want to send
			uint8_t* output		--> pointer to the beginning of output array
			uint16_t w_length	--> detemines number of bytes that you want to read

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_readReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* output, uint16_t w_length){
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	//HAL_Delay(10);
	HAL_SPI_Transmit(_LoRa->hSPIx, address, r_length, TRANSMIT_TIMEOUT);
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	HAL_SPI_Receive(_LoRa->hSPIx, output, w_length, RECEIVE_TIMEOUT);
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_writeReg

		description : write a value(s) in a register(s) by an address

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			uint8_t* address  -->	pointer to the beginning of address array
			uint16_t r_length -->	detemines number of addresse bytes that
														you want to send
			uint8_t* output		--> pointer to the beginning of values array
			uint16_t w_length	--> detemines number of bytes that you want to send

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_writeReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* values, uint16_t w_length){
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	//HAL_Delay(10);
	HAL_SPI_Transmit(_LoRa->hSPIx, address, r_length, TRANSMIT_TIMEOUT);
	//for(volatile int i=0; i<50; i++);  // small delay (~1–2 µs)
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	HAL_SPI_Transmit(_LoRa->hSPIx, values, w_length, TRANSMIT_TIMEOUT);
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
	//HAL_Delay(50);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setLowDaraRateOptimization

		description : set the LowDataRateOptimization flag. Is is mandated for when the symbol length exceeds 16ms.

		arguments   :
			LoRa*	LoRa         --> LoRa object handler
			uint8_t	value        --> 0 to disable, otherwise to enable

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setLowDaraRateOptimization(LoRa* _LoRa, uint8_t value){
	uint8_t	data;
	uint8_t	read;

	read = LoRa_read(_LoRa, RegModemConfig3);
	
	if(value)
		data = read | 0x08;
	else
		data = read & 0xF7;

	LoRa_write(_LoRa, RegModemConfig3, data);
	HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setAutoLDO

		description : set the LowDataRateOptimization flag automatically based on the symbol length.

		arguments   :
			LoRa*	LoRa         --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setAutoLDO(LoRa* _LoRa){
	double BW[] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0, 500.0};
	
	LoRa_setLowDaraRateOptimization(_LoRa, (long)((1 << _LoRa->spredingFactor) / ((double)BW[_LoRa->bandWidth])) > 16.0);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setFrequency

		description : set carrier frequency e.g 433 MHz

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   freq        --> desired frequency in MHz unit, e.g 434

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setFrequency(LoRa* _LoRa, int freq){
	uint8_t  data;
	uint32_t F;
	F = (freq * 524288)>>5;

	// write Msb:
	data = F >> 16;
	LoRa_write(_LoRa, RegFrMsb, data);
	HAL_Delay(5);

	// write Mid:
	data = F >> 8;
	LoRa_write(_LoRa, RegFrMid, data);
	HAL_Delay(5);

	// write Lsb:
	data = F >> 0;
	LoRa_write(_LoRa, RegFrLsb, data);
	HAL_Delay(5);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setSpreadingFactor

		description : set spreading factor, from 7 to 12.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   SP          --> desired spreading factor e.g 7

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setSpreadingFactor(LoRa* _LoRa, int SF){
	uint8_t	data;
	uint8_t	read;

	if(SF>12)
		SF = 12;
	if(SF<7)
		SF = 7;

	read = LoRa_read(_LoRa, RegModemConfig2);
	HAL_Delay(10);

	data = (SF << 4) + (read & 0x0F);
	LoRa_write(_LoRa, RegModemConfig2, data);
	HAL_Delay(10);
	
	LoRa_setAutoLDO(_LoRa);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setPower

		description : set power gain.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   power       --> desired power like POWER_17db

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setPower(LoRa* _LoRa, uint8_t power){
	LoRa_write(_LoRa, RegPaConfig, power);
	HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setOCP

		description : set maximum allowed current.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   current     --> desired max currnet in mA, e.g 120

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setOCP(LoRa* _LoRa, uint8_t current){
	uint8_t	OcpTrim = 0;

	if(current<45)
		current = 45;
	if(current>240)
		current = 240;

	if(current <= 120)
		OcpTrim = (current - 45)/5;
	else if(current <= 240)
		OcpTrim = (current + 30)/10;

	OcpTrim = OcpTrim + (1 << 5);
	LoRa_write(_LoRa, RegOcp, OcpTrim);
	HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setTOMsb_setCRCon

		description : set timeout msb to 0xFF + set CRC enable.

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setTOMsb_setCRCon(LoRa* _LoRa){
	uint8_t read, data;

	read = LoRa_read(_LoRa, RegModemConfig2);

	data = read | 0x03; //0000 0011 - CRC disable
	//data = read | 0x07; //0000 0111 - CRC enable
	LoRa_write(_LoRa, RegModemConfig2, data);\
	HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setTOMsb_setCRCon

		description : set timeout msb to 0xFF + set CRC enable.

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncword){
	LoRa_write(_LoRa, RegSyncWord, syncword);
	HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_read

		description : read a register by an address

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D

		returns     : register value
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_read(LoRa* _LoRa, uint8_t address){
	uint8_t read_data=0;
	uint8_t data_addr;

	data_addr = address & 0x7F;	// Ensure MSB is 0 for Read - 0111 1111
	LoRa_readReg(_LoRa, &data_addr, 1, &read_data, 1);
	//HAL_Delay(5);

	return read_data;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_write

		description : write a value in a register by an address

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D
			uint8_t value       --> value that you want to write

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value){
	uint8_t data;
	uint8_t addr;

	addr = address | 0x80;	// Ensure MSB is 1 for Write - 1000 0000
	data = value;
	LoRa_writeReg(_LoRa, &addr, 1, &data, 1);
	//HAL_Delay(5);
}

/*void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value){
	uint8_t buffer[2];

	buffer[0] = address | 0x80;  // write bit
	buffer[1] = value;

	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(_LoRa->hSPIx, buffer, 2, 100);

	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}

uint8_t LoRa_read(LoRa* _LoRa, uint8_t address){
	uint8_t tx[2];
	uint8_t rx[2];

	tx[0] = address & 0x7F;
	tx[1] = 0x00;

	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);

	HAL_SPI_TransmitReceive(_LoRa->hSPIx, tx, rx, 2, 100);

	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);

	return rx[1];
}*/

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_BurstWrite

		description : write a set of values in a register by an address respectively

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D
			uint8_t *value      --> address of values that you want to write

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_BurstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length){
	uint8_t addr;
	addr = address | 0x80;

	//NSS = 1
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_RESET);
	
	HAL_SPI_Transmit(_LoRa->hSPIx, &addr, 1, TRANSMIT_TIMEOUT);
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	//Write data in FiFo
	HAL_SPI_Transmit(_LoRa->hSPIx, value, length, TRANSMIT_TIMEOUT);
	while (HAL_SPI_GetState(_LoRa->hSPIx) != HAL_SPI_STATE_READY);
	//NSS = 0
	//HAL_Delay(5);
	HAL_GPIO_WritePin(_LoRa->CS_port, _LoRa->CS_pin, GPIO_PIN_SET);
}
/* ----------------------------------------------------------------------------- *\
		name        : LoRa_isvalid

		description : check the LoRa instruct values

		arguments   :
			LoRa* LoRa --> LoRa object handler

		returns     : returns 1 if all of the values were given, otherwise returns 0
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_isvalid(LoRa* _LoRa){

	return 1;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_transmit

		description : Transmit data

		arguments   :
			LoRa*    LoRa     --> LoRa object handler
			uint8_t  data			--> A pointer to the data you wanna send
			uint8_t	 length   --> Size of your data in Bytes
			uint16_t timeOut	--> Timeout in milliseconds
		returns     : 1 in case of success, 0 in case of timeout
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout){
	uint8_t read;

	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STNBY_MODE);
	HAL_Delay(5);
	//printf("After standby: 0x%X\r\n", LoRa_read(_LoRa, RegOpMode));
	LoRa_clearIrqFlags(_LoRa);
	read = LoRa_read(_LoRa, RegFiFoTxBaseAddr);
	LoRa_write(_LoRa, RegFiFoAddPtr, read);
	LoRa_write(_LoRa, RegPayloadLength, length);
	LoRa_BurstWrite(_LoRa, RegFiFo, data, length);
	//printf("PayloadLength: %d\r\n", LoRa_read(_LoRa, RegPayloadLength));
	LoRa_gotoMode(_LoRa, TRANSMIT_MODE);
	//HAL_Delay(5);
	//printf("After Transmit: 0x%X\r\n", LoRa_read(_LoRa, RegOpMode));
	//printf("IRQ Flags: 0x%X\r\n", LoRa_read(_LoRa, RegIrqFlags));

	while(1){
		read = LoRa_read(_LoRa, RegIrqFlags);
		if((read & 0x08)!=0){ //WAIT until TX_DONE flag set
			//printf("DIO mapping: 0x%X\r\n", LoRa_read(_LoRa, RegDioMapping1));
			//printf("IRQ Flags: 0x%X\r\n", LoRa_read(_LoRa, RegIrqFlags));
			//printf("DIO0 pin state: %d\r\n", HAL_GPIO_ReadPin(DIO0_GPIO_Port, DIO0_Pin));
			LoRa_write(_LoRa, RegIrqFlags, 0xFF);
			//LoRa_write(_LoRa, RegIrqFlags, 0x08); //Clear TxDone flag
			LoRa_gotoMode(_LoRa, mode); // Return to previous mode
			HAL_Delay(5);
			return 1;
		}
		else{
			if(--timeout==0){
				LoRa_gotoMode(_LoRa, mode); // Return to previous mode
				HAL_Delay(5);
				return 0;
			}
		}
		HAL_Delay(1);
	}

	// Wait for TX done
		/*while(timeout--){
			read = LoRa_read(_LoRa, RegIrqFlags);

			if(read & 0x08){ // TxDone
				printf("After Transmit: 0x%X\r\n", LoRa_read(_LoRa, RegOpMode));
				printf("TxDone in RegIrqFlags\r\n");
				// ✅ Clear all IRQs (safer)
				LoRa_write(_LoRa, RegIrqFlags, 0xFF);

				// Return to previous mode
				LoRa_gotoMode(_LoRa, mode);

				return 1;
			}
			else{
				LoRa_gotoMode(_LoRa, mode);
				return 0;
			}

			HAL_Delay(1);
		}*/
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_startReceiving

		description : Start receiving continuously

		arguments   :
			LoRa*    LoRa     --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_startReceiving(LoRa* _LoRa){
	LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_Receive

		description : Read received data from module

		arguments   :
			LoRa*    LoRa     --> LoRa object handler
			uint8_t  data			--> A pointer to the array that you want to write bytes in it
			uint8_t	 length   --> Determines how many bytes you want to read

		returns     : The number of bytes received
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout){
	uint8_t read, read1;
	uint8_t number_of_bytes;
	uint8_t min = 0;

	for(int i=0; i<length; i++)
		data[i]=0;

	LoRa_gotoMode(_LoRa, STNBY_MODE);
	LoRa_clearIrqFlags(_LoRa);
	LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
	while(timeout--) {
		read = LoRa_read(_LoRa, RegIrqFlags);
		if((read & 0x40) != 0){
			//LoRa_write(_LoRa, RegIrqFlags, 0xFF);
			read1 = LoRa_read(_LoRa, RegFiFoRxBaseAddr);
			number_of_bytes = LoRa_read(_LoRa, RegRxNbBytes);
			read = LoRa_read(_LoRa, RegFiFoRxCurrentAddr);
			LoRa_write(_LoRa, RegFiFoAddPtr, read);
			min = length >= number_of_bytes ? number_of_bytes : length;
			for(int i=0; i<min; i++)
				data[i] = LoRa_read(_LoRa, RegFiFo);
			return min;
			LoRa_clearIrqFlags(_LoRa);
		}
		LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
		HAL_Delay(1);
	}
    //return min;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_getRSSI

		description : initialize and set the right setting according to LoRa sruct vars

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Returns the RSSI value of last received packet.
\* ----------------------------------------------------------------------------- */
int LoRa_getRSSI(LoRa* _LoRa){
	uint8_t read;
	read = LoRa_read(_LoRa, RegPktRssiValue);
	return -164 + read;
}
// -----------------------------------------------------------------------------  //
void LoRa_clearIrqFlags(LoRa* _LoRa)
{
	//uint8_t before = LoRa_read(_LoRa, 0x12);
    //printf("IRQ before clear: 0x%X\r\n", before);
	uint8_t addr = RegIrqFlags;
    uint8_t val  = 0xFF;
    LoRa_write(_LoRa, addr, val);
    //uint8_t after = LoRa_read(_LoRa, 0x12);
    //printf("IRQ after clear: 0x%X\r\n", after);
}

void LoRa_clearRxDone(LoRa* _LoRa)
{
	uint8_t addr = RegIrqFlags;
    uint8_t val  = RegDioMapping1;
    LoRa_writeReg(_LoRa, &addr, 1, &val, 1);
}

void LoRa_forceLoRaMode(LoRa* _LoRa)
{
    // Step 1: Sleep
	LoRa_write(_LoRa, 0x01, 0x00); //Force FSK Sleep
	HAL_Delay(5);

    // Step 2: LoRa sleep (bit7 = 1)
    LoRa_write(_LoRa, 0x01, 0x80); // Sleep + LoRa
    HAL_Delay(5);

    // Step 3: Standby in LoRa
    LoRa_write(_LoRa, 0x01, 0x81); // Standby + LoRa
    HAL_Delay(5);

    uint8_t op = LoRa_read(_LoRa, 0x01);
    printf("OP_MODE after wake: 0x%X\r\n", op);
}

// -----------------------------------------------------------------------------  //
void LoRa_FIFO_Config(LoRa* _LoRa)
{
	// Set TX FIFO base address
		LoRa_write(_LoRa, RegFiFoTxBaseAddr, 0x80);

	// Set RX FIFO base address
		LoRa_write(_LoRa, RegFiFoRxBaseAddr, 0x00);

	// Reset FIFO pointer
		LoRa_write(_LoRa, RegFiFoAddPtr, 0x00);
}


/* ----------------------------------------------------------------------------- *\
		name        : LoRa_init

		description : initialize and set the right setting according to LoRa sruct vars

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
uint16_t LoRa_init(LoRa* _LoRa){
	uint8_t    data;
	uint8_t    read;

	if(LoRa_isvalid(_LoRa)){
		// goto sleep mode:
			LoRa_gotoMode(_LoRa, SLEEP_MODE);
			HAL_Delay(20);

			uint8_t op;
			do {
			    op = LoRa_read(_LoRa, RegOpMode);
			} while ((op & 0x07) != 0x00);   // wait for sleep mode

			//printf("Confirmed Sleep: 0x%X\r\n", op);

		// turn on LoRa mode:
			read = LoRa_read(_LoRa, RegOpMode);
			HAL_Delay(10);
			data = read | 0x80;
			LoRa_write(_LoRa, RegOpMode, data);
			HAL_Delay(100);
			uint8_t op1 = LoRa_read(_LoRa, RegOpMode);
			//printf("After lora mode set: 0x%X\r\n", op1);

			/*if ((op1 & 0x80) == 0) {
			    LoRa_write(_LoRa, RegOpMode, 0x80);
			    HAL_Delay(10);
			    op1 = LoRa_read(_LoRa, RegOpMode);
			    printf("Retry LoRa set: 0x%X\r\n", op1);
			}*/

		/* Configure LoRa modem */
			//LoRa_write(_LoRa, RegModemConfig1, 0x72);   // RegModemConfig1
			//LoRa_write(_LoRa, RegModemConfig2, 0x74);   // RegModemConfig2
			//LoRa_write(_LoRa, RegModemConfig3, 0x04);   // RegModemConfig3

		// COnfigure FIFO
			//LoRa_FIFO_Config(_LoRa);

		//Clear IRQ Flags
			LoRa_clearIrqFlags(_LoRa);

		// goto standby mode:
			LoRa_gotoMode(_LoRa, STNBY_MODE);
			_LoRa->current_mode = STNBY_MODE;
			HAL_Delay(10);
			op = LoRa_read(_LoRa, RegOpMode);
			//printf("After Standby set: 0x%X\r\n", op);

		// set frequency:
			LoRa_setFrequency(_LoRa, _LoRa->frequency);

		// set output power gain:
			LoRa_setPower(_LoRa, _LoRa->power);

		// set over current protection:
			LoRa_setOCP(_LoRa, _LoRa->overCurrentProtection);

		// set LNA gain:
			LoRa_write(_LoRa, RegLna, 0x23);

		// set spreading factor, CRC on, and Timeout Msb:
			LoRa_setTOMsb_setCRCon(_LoRa);
			LoRa_setSpreadingFactor(_LoRa, _LoRa->spredingFactor);

		// set Timeout Lsb:
			LoRa_write(_LoRa, RegSymbTimeoutL, 0xFF);

		// set bandwidth, coding rate and expilicit mode:
			// 8 bit RegModemConfig --> | X | X | X | X | X | X | X | X |
			//       bits represent --> |   bandwidth   |     CR    |I/E|
			data = 0;
			data = (_LoRa->bandWidth << 4) + (_LoRa->crcRate << 1);
			LoRa_write(_LoRa, RegModemConfig1, data);
			LoRa_setAutoLDO(_LoRa);

		// set preamble:
			LoRa_write(_LoRa, RegPreambleMsb, _LoRa->preamble >> 8);
			LoRa_write(_LoRa, RegPreambleLsb, _LoRa->preamble >> 0);

		// DIO mapping:   --> DIO: RxDone
			read = LoRa_read(_LoRa, RegDioMapping1);
			data = read | 0x3F;
			LoRa_write(_LoRa, RegDioMapping1, data);

			read = LoRa_read(_LoRa, RegVersion);
			if(read == 0x12)
				return LORA_OK;
			else
				return LORA_NOT_FOUND;
	}
	else {
		return LORA_UNAVAILABLE;
	}
}


