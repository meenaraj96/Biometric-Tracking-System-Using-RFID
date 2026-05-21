#include "MFRC522_STM32.h"
#include "main.h"

uint8_t atqa[2];

void MFRC522_Init(MFRC522_t *dev) {
    USER_LOG("MFRC522 Min Init started");
    // Hardware reset
    HAL_GPIO_WritePin(dev->rstPort, dev->rstPin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(dev->rstPort, dev->rstPin, GPIO_PIN_SET);
    HAL_Delay(50);

    // Soft reset
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_SoftReset);
    HAL_Delay(50);

    // Clear interrupts
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);

    // Flush FIFO
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);

    // Timer: ~25ms timeout
    MFRC522_WriteReg(dev, PCD_TModeReg, 0x80);      // Timer starts immediately
    MFRC522_WriteReg(dev, PCD_TPrescalerReg, 0xA9); // 80kHz clock
    MFRC522_WriteReg(dev, PCD_TReloadRegH, 0x03);   // 1000 ticks = ~12.5ms
    MFRC522_WriteReg(dev, PCD_TReloadRegL, 0xE8);

    // RF settings
    MFRC522_WriteReg(dev, PCD_TxAutoReg, 0x40);     // 100% ASK modulation
    MFRC522_WriteReg(dev, PCD_RFCfgReg, 0x7F);      // Max gain (48dB)
    MFRC522_WriteReg(dev, PCD_DemodReg, 0x4D);      // Sensitivity for clones

    // Enable antenna
    MFRC522_AntennaOn(dev);
    HAL_Delay(10);  // Let RF stabilize

    uint8_t version = MFRC522_ReadReg(dev, PCD_VersionReg);
    if ((version != 0x91)||(version != 0x92)){
    	USER_LOG("Version: 0x%02X (counterfeit OK for UID)", version);
    }
    else USER_LOG("Version: 0x%02X", version);
    uint8_t txCtrl = MFRC522_ReadReg(dev, PCD_TxControlReg);
    DEBUG_LOG("TxControlReg: 0x%02X (expect >= 0x03)", txCtrl);
    USER_LOG("MFRC522 Min Init complete");
}

void MFRC522_AntennaOff(MFRC522_t *dev) {
    MFRC522_ClearBitMask(dev, PCD_TxControlReg, 0x03);
    DEBUG_LOG("Antenna off");
}

void MFRC522_AntennaOn(MFRC522_t *dev) {
    MFRC522_SetBitMask(dev, PCD_TxControlReg, 0x03);
    DEBUG_LOG("Antenna on");
}

void MFRC522_Set_AntennaOn_Gain(MFRC522_t *dev, uint8_t gain)
{
    MFRC522_SetBitMask(dev, PCD_TxControlReg, 0x03);

    if (gain > 0x07) gain = 0x07;   // clamp

    uint8_t value = MFRC522_ReadReg(dev, PCD_RFCfgReg);

    value &= ~(0x70);           // clear gain bits
    value |= (gain << 4);       // set new gain

    MFRC522_WriteReg(dev, PCD_RFCfgReg, value);

    DEBUG_LOG("Antenna gain set: %d", gain);
}

uint8_t MFRC522_ReadReg(MFRC522_t *dev, uint8_t reg) {
    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;
    uint8_t val = 0;
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET); //Activate
    HAL_SPI_Transmit(dev->hspi, &addr, 1, HAL_MAX_DELAY);
	while (HAL_SPI_GetState(dev->hspi) != HAL_SPI_STATE_READY);
    HAL_SPI_Receive(dev->hspi, &val, 1, HAL_MAX_DELAY);
	while (HAL_SPI_GetState(dev->hspi) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET); //Deactivate
    HAL_Delay(1);
    DEBUG_LOG("ReadReg: 0x%02X -> 0x%02X", reg, val);
    return val;
}

void MFRC522_WriteReg(MFRC522_t *dev, uint8_t reg, uint8_t value) {
    uint8_t addr = (reg << 1) & 0x7E;
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET); //Activate
    HAL_SPI_Transmit(dev->hspi, &addr, 1, HAL_MAX_DELAY);
	while (HAL_SPI_GetState(dev->hspi) != HAL_SPI_STATE_READY);
    HAL_SPI_Transmit(dev->hspi, &value, 1, HAL_MAX_DELAY);
	while (HAL_SPI_GetState(dev->hspi) != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET); //Deactivate
    HAL_Delay(1);
    DEBUG_LOG("WriteReg: 0x%02X = 0x%02X", reg, value);
}

void MFRC522_SetBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask) {
    uint8_t tmp = MFRC522_ReadReg(dev, reg);
    MFRC522_WriteReg(dev, reg, tmp | mask);
    DEBUG_LOG("SetBitMask: 0x%02X |= 0x%02X", reg, mask);
}

void MFRC522_ClearBitMask(MFRC522_t *dev, uint8_t reg, uint8_t mask) {
    uint8_t tmp = MFRC522_ReadReg(dev, reg);
    MFRC522_WriteReg(dev, reg, tmp & (~mask));
    DEBUG_LOG("ClearBitMask: 0x%02X &= ~0x%02X", reg, mask);
}

uint8_t MFRC522_RequestA(MFRC522_t *dev, uint8_t *atqa) {
    DEBUG_LOG("RequestA");
    MFRC522_AntennaOff(dev);  // Reset RF
    HAL_Delay(5);  // Allow chip to stabilize
    MFRC522_AntennaOn(dev);
    HAL_Delay(5);  // Ensure RF is ready
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);      // Clear IRQs
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);   // Flush FIFO
    MFRC522_WriteReg(dev, PCD_BitFramingReg, 0x07);  // 7 bits for REQA
    MFRC522_WriteReg(dev, PCD_FIFODataReg, PICC_REQA);
    HAL_Delay(2);  // Increased for counterfeit chip stability
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Transceive);
    MFRC522_SetBitMask(dev, PCD_BitFramingReg, 0x80);

    // Poll for completion (25ms timeout)
    uint32_t timeout = HAL_GetTick() + 25;
    while (HAL_GetTick() < timeout) {
        uint8_t status2 = MFRC522_ReadReg(dev, PCD_Status2Reg);
        uint8_t RxMode = MFRC522_ReadReg(dev, PCD_RxModeReg); //Bit3-RxNoErr
    	uint8_t comirq = MFRC522_ReadReg(dev, PCD_ComIrqReg); //Bit5-RIRq
    	//if the RxModeReg register’s RxNoErr bit is set to logic1,
    	//the RxIRq bit is only set to logic1 when data bytes are available in the FIFO
        //if (status2 & 0x01) {  // Command complete
    	if ((comirq & 0x30)||(comirq & 0x20))	{ //RxIRq triggered // RxIRq or IdleIRq // 0011 or 0010
            uint8_t err = MFRC522_ReadReg(dev, PCD_ErrorReg);
            if (err & 0x1D) {  // Protocol/parity/buffer errors
                DEBUG_LOG("RequestA error: 0x%02X", err);
                MFRC522_AntennaOff(dev);
                HAL_Delay(5);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle); // Stop command
                return STATUS_ERROR;
            }
            else if (err & 0x1B)
                return STATUS_ERROR;
            uint8_t fifoLvl = MFRC522_ReadReg(dev, PCD_FIFOLevelReg);
            if (fifoLvl >= 2) {  // ATQA is 2 bytes
                atqa[0] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                atqa[1] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                DEBUG_LOG("RequestA ATQA: 0x%02X 0x%02X", atqa[0], atqa[1]);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle); // Stop command
                HAL_Delay(2);  // Post-command delay
                return STATUS_OK;
            }
            DEBUG_LOG("RequestA bad FIFO level: %d", fifoLvl);
            MFRC522_AntennaOff(dev);
            HAL_Delay(5);
            MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
            return STATUS_ERROR;
        }
		if (comirq & 0x01) // TimerIRq
			return STATUS_TIMEOUT;
        HAL_Delay(1);  // Mimic debug log timing
    }
    DEBUG_LOG("RequestA timeout");
    MFRC522_AntennaOff(dev);
    HAL_Delay(5);
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
    return STATUS_TIMEOUT;
}

uint8_t MFRC522_Anticoll(MFRC522_t *dev, uint8_t *uid) {  // Returns 4-byte UID + BCC
    DEBUG_LOG("Anticoll");
    MFRC522_WriteReg(dev, PCD_ComIrqReg, 0x7F);      // Clear IRQs
    MFRC522_WriteReg(dev, PCD_FIFOLevelReg, 0x80);   // Flush FIFO
    MFRC522_WriteReg(dev, PCD_BitFramingReg, 0x00);  // Full frame
    MFRC522_WriteReg(dev, PCD_FIFODataReg, PICC_SEL_CL1);  // 0x93
    MFRC522_WriteReg(dev, PCD_FIFODataReg, 0x20);    // Fixed CRC
    HAL_Delay(2);  // Delay for stability
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Transceive);
    MFRC522_SetBitMask(dev, PCD_BitFramingReg, 0x80);

    uint32_t timeout = HAL_GetTick() + 25;
    while (HAL_GetTick() < timeout) {
        uint8_t status2 = MFRC522_ReadReg(dev, PCD_Status2Reg);
        uint8_t RxMode1 = MFRC522_ReadReg(dev, PCD_RxModeReg); //Bit3-RxNoErr
    	uint8_t comirq1 = MFRC522_ReadReg(dev, PCD_ComIrqReg);
        //if (status21 & 0x01) {  // Command complete
    	if ((comirq1 & 0x30)||(comirq1 & 0x20)) { //RxIRq triggered // RxIRq or IdleIRq
            uint8_t err1 = MFRC522_ReadReg(dev, PCD_ErrorReg);
            if (err1 & 0x1D) {
                DEBUG_LOG("Anticoll error: 0x%02X", err);
                MFRC522_AntennaOff(dev);
                HAL_Delay(5);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                return STATUS_ERROR;
            }
            else if (err1 & 0x1B)
                return STATUS_ERROR;
            uint8_t fifoLvl1 = MFRC522_ReadReg(dev, PCD_FIFOLevelReg);
            if (fifoLvl1 == 5) {  // 4-byte UID + BCC
                for (int i = 0; i < 5; i++) {
                    uid[i] = MFRC522_ReadReg(dev, PCD_FIFODataReg);
                }
                // Validate BCC
                uint8_t calcBcc = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
                if (uid[4] != calcBcc) {
                    DEBUG_LOG("Anticoll bad BCC: calc=0x%02X, got=0x%02X", calcBcc, uid[4]);
                    MFRC522_AntennaOff(dev);
                    HAL_Delay(5);
                    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                    return STATUS_ERROR;
                }
                DEBUG_LOG("Anticoll UID: %02X %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3], uid[4]);
                MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
                HAL_Delay(2);  // Post-command delay
                return STATUS_OK;
            }
            DEBUG_LOG("Anticoll bad FIFO level: %d", fifoLvl);
            MFRC522_AntennaOff(dev);
            HAL_Delay(5);
            MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
            return STATUS_ERROR;
        }
		if (comirq1 & 0x01) // TimerIRq
			return STATUS_TIMEOUT;
        HAL_Delay(1);  // Mimic debug log timing
    }
    DEBUG_LOG("Anticoll timeout");
    MFRC522_AntennaOff(dev);
    HAL_Delay(5);
    MFRC522_WriteReg(dev, PCD_CommandReg, PCD_Idle);
    return STATUS_TIMEOUT;
}

uint8_t MFRC522_ReadUid(MFRC522_t *dev, uint8_t *uid) {  // Output: uid[4]
    DEBUG_LOG("Reading UID...");
    // Card detected, read UID
    uint8_t rawUid[5];
    if (MFRC522_Anticoll(dev, rawUid) != STATUS_OK) {
    	DEBUG_LOG("Anticollision failed");
        return STATUS_ERROR;
    }
    // Copy UID (drop BCC)
    for (int i = 0; i < 4; i++) {
        uid[i] = rawUid[i];
    }
    DEBUG_LOG("Card UID: %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3]);
    return STATUS_OK;
}

uint8_t waitcardRemoval (MFRC522_t *dev){
    USER_LOG("Waiting for card removal...");
    while (1) {
        if (MFRC522_RequestA(dev, atqa) != STATUS_OK) {
        	USER_LOG("Card removed");
            return STATUS_OK; // Card removed, return success
        }
        HAL_Delay(100); // Poll every 100ms to check if card is still present
    }
}

uint8_t waitcardDetect (MFRC522_t *dev){
	atqa[0] = atqa[1] = 0;
	//USER_LOG("Waiting for the card...");
	//while (1){
	    if (MFRC522_RequestA(dev, atqa) == STATUS_OK) {
	    	USER_LOG("Card detected");
	        return STATUS_OK;
	    }
	    //HAL_Delay(100);	// Poll every 100ms to check if card is  present
	//}
}


