/*
 * RFM69.h
 *
 *  Created on: Jun 24, 2021
 *      Author: Vitech-UA
 */

#ifndef RFM69_H_
#define RFM69_H_

#include <SPI.h>

#define RF69_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR   0
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000
#define RF69_FSTEP  61.03515625 // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet)

// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40

#define RFM69_ACK_TIMEOUT   30  // 30ms roundtrip req for 61byte packets

//Native hardware ListenMode is experimental
//It was determined to be buggy and unreliable, see https://lowpowerlab.com/forum/low-power-techniques/ultra-low-power-listening-mode-for-battery-nodes/msg20261/#msg20261
//uncomment to try ListenMode, adds ~1K to compiled size
//FYI - 10bit addressing is not supported in ListenMode
//#define RF69_LISTENMODE_ENABLE

#if defined(RF69_LISTENMODE_ENABLE)
  // By default, receive for 256uS in listen mode and idle for ~1s
  #define  DEFAULT_LISTEN_RX_US 256
  #define  DEFAULT_LISTEN_IDLE_US 1000000
#endif

class RFM69: public SPI {
public:
	RFM69(SPI_TypeDef *spi, SPI_DataSize_t size,
            GPIO_TypeDef *slaveSelectPort,
			  uint16_t slaveSelectPin,
			  GPIO_TypeDef *interruptPort,
			  uint16_t interruptPin,
			  bool isRFM69HW);
	bool rfm_init(uint8_t freqBand, uint16_t nodeID, uint8_t networkID);
protected:
	uint8_t _slaveSelectPin;
	uint8_t _interruptPin;
	uint8_t _interruptNum;
	uint16_t _address;
	bool _spyMode;
	uint8_t _powerLevel;
	bool _isRFM69HW;
	uint8_t _mode;        // current transceiver state

};

#endif /* RFM69_H_ */
