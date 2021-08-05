/*
 * rfm69.c
 *
 *  Created on: Aug 2, 2021
 *      Author: Vitech-UA
 */

#include "rfm69.h"

extern SPI_HandleTypeDef hspi1;

#define rfm_spi hspi1
#define RFM69_XO               32000000    ///< Internal clock frequency [Hz]
#define RFM69_FSTEP            61.03515625 ///< Step width of synthesizer [Hz]
#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_MAX_DATA_LEN       61
#define RF69_TX_LIMIT_MS   1000
// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40
#define RF69_CSMA_LIMIT_MS 1000

volatile uint8_t _mode;        // current transceiver state
volatile bool _inISR;
volatile uint8_t PAYLOADLEN;

volatile uint8_t DATALEN;
volatile uint8_t SENDERID;
volatile uint8_t TARGETID;
volatile uint8_t PAYLOADLEN;
volatile uint8_t ACK_REQUESTED;
volatile uint8_t ACK_RECEIVED;
volatile int16_t RSSI;          // most accurate RSSI during reception (closest to the reception)
volatile bool _inISR;

uint8_t _powerLevel;
uint8_t _address;
uint8_t _interruptPin;
uint8_t _interruptNum;
uint8_t _address;
bool _isRFM69HW = false;
bool _promiscuousMode;

void rfm69_select(void) {

	RFM69_SELECT_GPIO->BSRR |= RFM69_SELECT_PIN << 16U; // RESET

}

void rfm69_release(void) {

	RFM69_SELECT_GPIO->BSRR |= RFM69_SELECT_PIN; // SET
}

void rfm69_up_reset_pin(void) {
	RFM69_RESET_GPIO->BSRR |= RFM69_RESET_PIN; //SET
}
void rfm69_down_reset_pin(void) {
	RFM69_RESET_GPIO->BSRR |= RFM69_RESET_PIN << 16U; //SET
}

uint8_t rfm69_read_register(uint8_t reg) {

	uint8_t regval = 0;
	uint8_t zero_byte = 0;
	uint8_t read_data = reg & 0x7F;

	rfm69_select();
	HAL_SPI_Transmit(&rfm_spi, &read_data, 1, 100);
	HAL_SPI_TransmitReceive(&rfm_spi, (uint8_t*) &zero_byte, (uint8_t*) &regval,
			1, 100);
	rfm69_release();

	return regval;

}

void rfm69_write_register(uint8_t reg, uint8_t value) {
	rfm69_select();
	uint8_t write_data = reg | 0x80;
	HAL_SPI_Transmit(&rfm_spi, (uint8_t*) &write_data, 1, 100);
	HAL_SPI_Transmit(&rfm_spi, (uint8_t*) &value, 1, 100);
	rfm69_release();

}

uint32_t rfm69_get_frequency(void) {
	return RFM69_FSTEP
			* (((uint32_t) rfm69_read_register(REG_FRFMSB) << 16)
					+ ((uint16_t) rfm69_read_register(REG_FRFMID) << 8)
					+ rfm69_read_register(REG_FRFLSB));
}

void rfm69_set_frequency(unsigned int frequency) {

	// calculate register value
	frequency /= RFM69_FSTEP;

	// set new frequency
	rfm69_write_register(0x07, frequency >> 16);
	rfm69_write_register(0x08, frequency >> 8);
	rfm69_write_register(0x09, frequency);
}

bool rfm69_init(uint8_t freqBand, uint8_t nodeID, uint8_t networkID) {
	_powerLevel = 10;
	const uint8_t CONFIG[][2] =
			{ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF
					| RF_OPMODE_STANDBY }, { REG_DATAMODUL,
			RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK
					| RF_DATAMODUL_MODULATIONSHAPING_00 }, {
			REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET
					| RF_DATAMODUL_MODULATIONTYPE_FSK
					| RF_DATAMODUL_MODULATIONSHAPING_00 },
					{ REG_BITRATEMSB, RF_BITRATEMSB_55555 }, // default: 4.8 KBPS
					{ REG_BITRATELSB, RF_BITRATELSB_55555 },
					{ REG_FDEVMSB, RF_FDEVMSB_50000 }, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
					{ REG_FDEVLSB, RF_FDEVLSB_50000 },
					//{ REG_FRFMSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMSB_315 : (freqBand==RF69_433MHZ ? RF_FRFMSB_433 : (freqBand==RF69_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
					//{ REG_FRFMID, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMID_315 : (freqBand==RF69_433MHZ ? RF_FRFMID_433 : (freqBand==RF69_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
					//{ REG_FRFLSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFLSB_315 : (freqBand==RF69_433MHZ ? RF_FRFLSB_433 : (freqBand==RF69_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },
					// RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
					/* 0x19 */{ REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16
							| RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
					//for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
					{ REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
					{ REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
					{ REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
					{ REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
					///* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
					{ REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO
							| RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
					{ REG_SYNCVALUE1, 0x2D }, // attempt to make this compatible with sync1 byte of RFM12B lib
					{ REG_SYNCVALUE2, networkID }, // NETWORK ID
					{ REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE
							| RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON
							| RF_PACKET1_CRCAUTOCLEAR_ON
							| RF_PACKET1_ADRSFILTERING_OFF },
					{ REG_PAYLOADLENGTH, 66 }, // in variable length mode: the max frame size, not used in TX
					///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
					{ REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY
							| RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
					{ REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS
							| RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
					//for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
					/* 0x6F */{ REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
					{ 255, 0 } };

	rfm69_release();
	unsigned long start = HAL_GetTick();
	uint8_t timeout = 50;
	do
		rfm69_write_register(REG_SYNCVALUE1, 0xAA);
	while (rfm69_read_register(REG_SYNCVALUE1) != 0xaa
			&& HAL_GetTickFreq() - start < timeout);
	start = HAL_GetTick();
	do
		rfm69_write_register(REG_SYNCVALUE1, 0x55);
	while (rfm69_read_register(REG_SYNCVALUE1) != 0x55
			&& HAL_GetTickFreq() - start < timeout);
	for (uint8_t i = 0; CONFIG[i][0] != 255; i++)
		rfm69_write_register(CONFIG[i][0], CONFIG[i][1]);

	rfm69_encrypt(false);
	rfm69_set_hi_power(_isRFM69HW);
	rfm69_set_mode(RF69_MODE_STANDBY);
	start = HAL_GetTick();
	while (((rfm69_read_register(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY)
			== 0x00) && HAL_GetTick() - start < timeout)
		; // wait for ModeReady
	if (HAL_GetTick() - start >= timeout) {
		return false;
	}
	_inISR = false;
	//attachInterrupt(_interruptNum, RFM69::isr0, RISING);
	//selfPointer = this;
	_address = nodeID;
	return true;

}

void rfm69_set_mode(uint8_t newMode) {
	if (newMode == _mode)
		return;

	switch (newMode) {
	case RF69_MODE_TX:
		rfm69_write_register(REG_OPMODE,
				(rfm69_read_register(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
		if (_isRFM69HW)
			rfm69_setHighPowerRegs(true);
		break;
	case RF69_MODE_RX:
		rfm69_write_register(REG_OPMODE,
				(rfm69_read_register(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
		if (_isRFM69HW)
			rfm69_setHighPowerRegs(false);
		break;
	case RF69_MODE_SYNTH:
		rfm69_write_register(REG_OPMODE,
				(rfm69_read_register(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
		break;
	case RF69_MODE_STANDBY:
		rfm69_write_register(REG_OPMODE,
				(rfm69_read_register(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
		break;
	case RF69_MODE_SLEEP:
		rfm69_write_register(REG_OPMODE,
				(rfm69_read_register(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
		break;
	default:
		return;
	}

	// we are using packet mode, so this check is not really needed
	// but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
	while (_mode == RF69_MODE_SLEEP
			&& (rfm69_read_register(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY)
					== 0x00)
		; // wait for ModeReady

	_mode = newMode;
}

void rfm69_setHighPowerRegs(bool onOff) {
	rfm69_write_register(REG_TESTPA1, onOff ? 0x5D : 0x55);
	rfm69_write_register(REG_TESTPA2, onOff ? 0x7C : 0x70);
}

// for RFM69HW only: you must call rfm69_set_hi_power(true) after initialize() or else transmission won't work
void rfm69_set_hi_power(bool onOff) {
	_isRFM69HW = onOff;
	rfm69_write_register(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
	if (_isRFM69HW) // turning ON
		rfm69_write_register(REG_PALEVEL,
				(rfm69_read_register(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON
						| RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
	else
		rfm69_write_register(REG_PALEVEL,
				RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF
						| _powerLevel); // enable P0 only
}

uint8_t rfm69_readTemperature(void) // returns centigrade
{
	rfm69_set_mode(RF69_MODE_STANDBY);
	rfm69_write_register(REG_TEMP1, RF_TEMP1_MEAS_START);
	while ((rfm69_read_register(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING))
		;
	return ~rfm69_read_register(REG_TEMP2) + COURSE_TEMP_COEF;
} // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void rfm69_setPowerLevel(uint8_t powerLevel) {
	_powerLevel = (powerLevel > 31 ? 31 : powerLevel);
	if (_isRFM69HW)
		_powerLevel /= 2;
	rfm69_write_register(REG_PALEVEL,
			(rfm69_read_register(REG_PALEVEL) & 0xE0) | _powerLevel);
}

uint8_t rfm69_getPowerLevel() {
	return _powerLevel;
}

void rfm69_sleep() {
	rfm69_set_mode(RF69_MODE_SLEEP);
}
uint8_t rfm69_getInterruptPin() {
	return _interruptPin;
}

// set the IRQ pin number (on STM32 same as interrupt pin)
void rfm69_setInterruptNumber(uint8_t newInterruptNumber) {
	_interruptNum = newInterruptNumber;
}

// get the IRQ pin number (on STM32 same as interrupt pin)
uint8_t rfm69_getInterruptNumber() {
	return _interruptNum;
}
void rfm69_setAddress(uint8_t addr) {
	_address = addr;
	rfm69_write_register(REG_NODEADRS, _address);
}

// get this node's address
uint8_t rfm69_getAdress() {
	return _address;
}

//set this node's network id
void rfm69_setNetwork(uint8_t networkID) {
	rfm69_write_register(REG_SYNCVALUE2, networkID);
}

//get this node's network id
uint8_t rfm69_getNetwork() {
	return rfm69_read_register(REG_SYNCVALUE2);
}

// get the received signal strength indicator (RSSI)
int16_t rfm69_readRSSI(bool forceTrigger) {
	int16_t rssi = 0;
	if (forceTrigger) {
		// RSSI trigger not needed if DAGC is in continuous mode
		rfm69_write_register(REG_RSSICONFIG, RF_RSSI_START);
		while ((rfm69_read_register(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00)
			; // wait for RSSI_Ready
	}
	rssi = -rfm69_read_register(REG_RSSIVALUE);
	rssi >>= 1;
	return rssi;
}

bool rfm69_canSend() {
	if (_mode == RF69_MODE_RX
			&& PAYLOADLEN == 0&& rfm69_readRSSI(true) < CSMA_LIMIT) // if signal stronger than -100dBm is detected assume channel activity
					{
		rfm69_set_mode(RF69_MODE_STANDBY);
		return true;
	}
	return false;
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool rfm69_receiveDone() {
	//ATOMIC_BLOCK(ATOMIC_FORCEON)
	//{
	//noInterrupts(); // re-enabled in unselect() via setMode() or via receiveBegin()
	if (_mode == RF69_MODE_RX && PAYLOADLEN > 0) {
		rfm69_set_mode(RF69_MODE_STANDBY); // enables interrupts
		return true;
	} else if (_mode == RF69_MODE_RX) // already in RX no payload yet
	{
		//interrupts(); // explicitly re-enable interrupts
		return false;
	}
	rfm69_receive_begin();
	return false;
	//}
}

void rfm69_receive_begin(void) {
	DATALEN = 0;
	SENDERID = 0;
	TARGETID = 0;
	PAYLOADLEN = 0;
	ACK_REQUESTED = 0;
	ACK_RECEIVED = 0;
	RSSI = 0;
	if (rfm69_read_register(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY)
		rfm69_write_register(REG_PACKETCONFIG2,
				(rfm69_read_register(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
	rfm69_write_register(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
	rfm69_set_mode(RF69_MODE_RX);
}

void rfm69_send(uint8_t toAddress, const void *buffer, uint8_t bufferSize,
bool requestACK) {
	rfm69_write_register(REG_PACKETCONFIG2,
			(rfm69_read_register(REG_PACKETCONFIG2) & 0xFB)
					| RF_PACKET2_RXRESTART); // avoid RX deadlocks
	uint32_t now = HAL_GetTick();
	while (!rfm69_canSend && HAL_GetTick() - now < RF69_CSMA_LIMIT_MS)
		rfm69_receiveDone();
	rfm69_sendFrame(toAddress, buffer, bufferSize, requestACK, false);
}
void rfm69_sendFrame(uint8_t toAddress, const void *buffer, uint8_t bufferSize,
bool requestACK, bool sendACK) {
	rfm69_set_mode(RF69_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
	while ((rfm69_read_register(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00)
		; // wait for ModeReady
	rfm69_write_register(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
	if (bufferSize > RF69_MAX_DATA_LEN)
		bufferSize = RF69_MAX_DATA_LEN;

	// control byte
	uint8_t CTLbyte = 0x00;
	if (sendACK)
		CTLbyte = RFM69_CTL_SENDACK;
	else if (requestACK)
		CTLbyte = RFM69_CTL_REQACK;

	// write to FIFO
	rfm69_select();
	HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t) REG_FIFO | 0x80, 1, 100);
	HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t) bufferSize + 3, 1, 100);
	HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t) toAddress, 1, 100);
	HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t) _address, 1, 100);
	HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t) CTLbyte, 1, 100);

	for (uint8_t i = 0; i < bufferSize; i++) {
		HAL_SPI_Transmit(&RFM69_SPI_PORT, ((uint8_t*) &buffer)[i], 1, 100);
	}
	rfm69_release();

	// no need to wait for transmit mode to be ready since its handled by the radio
	rfm69_set_mode(RF69_MODE_TX);
	uint32_t txStart = HAL_GetTick();

	while (rfm69_read_register(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT == 0x00)
		; // wait for ModeReady
	rfm69_set_mode(RF69_MODE_STANDBY);
}

// To enable encryption: radio.encrypt("ABCDEFGHIJKLMNOP");
// To disable encryption: radio.encrypt(null) or radio.encrypt(0)
// KEY HAS TO BE 16 bytes !!!
void rfm69_encrypt(const char *key) {
	rfm69_set_mode(RF69_MODE_STANDBY);
	if (key != 0) {
		rfm69_select();
		uint8_t send_data[1] = { REG_AESKEY1 | 0x80 };

		HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t*) &send_data, 1, 100);

		for (uint8_t i = 0; i < 16; i++) {
			HAL_SPI_Transmit(&RFM69_SPI_PORT, (uint8_t*) &key[i], 1, 100);
		}
		rfm69_release();
	}
	rfm69_write_register(REG_PACKETCONFIG2,
			(rfm69_read_register(REG_PACKETCONFIG2) & 0xFE) | (key ? 1 : 0));
}
void rfm69_promiscuous(bool onOff) {
  _promiscuousMode = onOff;
  //writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | (onOff ? RF_PACKET1_ADRSFILTERING_OFF : RF_PACKET1_ADRSFILTERING_NODEBROADCAST));
}
