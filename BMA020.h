/*
 * Author: Robin Gerhartz
 * Datei: BMA020.h
 * Datum: 18.08.2012
 * URL: (kommt noch)
 *
 * Dies ist eine kleine BMA020 Library
 * die ich aus Lerngründen schreibe/geschrieben habe
 * und um das Rad nicht immer wieder neu zu erfinden ;)
 *
 * Sie funktioniert "hoffentlich"(hab nur m328p da)
 * auf allen gängigen AVR's mit I2C Schnittstelle.
 *
 * 	Die beschaltung sollte so aussehen (BMA020 Modul von elv [http://www.elv.de/3-achsen-beschleunigungssensor-3d-bs-komplettbausatz.html]):
 * 		An dem BMA020 Modul:
 * 			UIN - CSB
 * 			UIN - UPullup
 * 			UIN - +2,5V bis +6V
 * 			SDO - GND
 *
 * 		Vom Mikrocontroller zum BMA020 Modul:
 * 			SCI - SCK
 * 			SDA - SDI
 * 			GND - GND
 *
 * Hier ein Beispiel für den Arduino
 *
 * 	///////// Code ////////////
#include <BMA020.h>

void setup(){
	BMA::begin(RANGE_4g, 0); // Starte I2C verbindung
	Serial.begin(9600); // Starte Serial verbindung um auch was zu sehen
}

void loop(){
	BMA::read_all_axes(); // Lese alle Achsen aus
	Serial.print("X: ");
	Serial.print(BMA::X);
	Serial.print("   Y: ");
	Serial.print(BMA::Y);
	Serial.print("   Z: ");
	Serial.println(BMA::acc_z()); // Lese nur die Z-Achse aus

	Serial.print("X_MSB: ");
	Serial.print(BMA::X_MSB);
	Serial.print("   Y_MSB: ");
	Serial.print(BMA::Y_MSB);
	Serial.print("   Z_MSB: ");
	Serial.println(BMA::Z_MSB);

	Serial.println();
	Serial.println();
	Serial.println();

	delay(500);
}
 *	///////// Code ende /////////
 *
 * Für mehr Informationen zu den einzelnen Funktionen
 * guckt ins Datenblatt, alle "public" Funktionen heißen
 * genau wie dort, mit ausnahme von "begin" und "read_all_axes".
 */

#ifndef BMA020_H_
#define BMA020_H_

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <i2c_communicate.c>

// Defines

// Werte

// (wird bei set_range gebraucht)
#define RANGE_2g			0x00
#define RANGE_4g			0x08
#define RANGE_8g			0x10

// (wird bei set_bandwidth gebraucht)
#define BANDWIDTH_25hz		0x00
#define BANDWIDTH_50hz		0x01
#define BANDWIDTH_100hz		0x02
#define BANDWIDTH_190hz		0x03
#define BANDWIDTH_375hz		0x04
#define BANDWIDTH_750hz		0x05
#define BANDWIDTH_1500hz	0x06

// (wird da gebraucht wo ein boolean gefragt ist)
#ifndef TRUE
	#define TRUE 0x01
#endif
#ifndef FALSE
	#define FALSE 0x00
#endif

// Register (wenn du mal ne eigene Funktion schreiben willst)
#define REG_SETTINGS_0		0x11	/* Any Motion Duration
									 * High Gravitation Hysteresis
									 * Low Gravitation Hysteresis */

#define REG_AMT				0x10 	// Any Motion Threshold

#define REG_HGD				0x0F 	// High Gravitation Duration

#define REG_HGT				0x0E	// High Gravitation Threshold

#define REG_LGD				0x0D 	// Low Gravitation Duration

#define REG_LGT				0x0C	// Low Gravitation Threshold

#define REG_CUSTOM_1		0x13 	// (Can be used as memory)
#define REG_CUSTOM_0		0x12 	// (Can be used as memory)

#define REG_STATUS_0		0x09	/* Self Test Result
									 * Alert Phase
									 * Low Gravitation Latched
									 * High Gravitation Latched
									 * Low Gravitation Status
									 * High Gravitation Status */

#define REG_DATA_1			0x01 	// AL version
									// ML version

#define REG_DATA_0			0x00 	// Chip ID

// Werden benötigt
#define REG_CONTROL_3		0x15	/* 4 wire SPI Enable
									 * Enable Advanced Interrupt
									 * New Data Interrupt
									 * Latch Interrupt
									 * Shadow Disable
									 * Wake Up Pause
									 * Wake Up */

#define REG_CONTROL_1		0x0B	/* Alert
									 * Any Motion
									 * High Gravitation Counter
									 * Low Gravitation Counter
									 * High Gravitation Enable
									 * Low Gravitation Enable */

#define REG_CONTROL_0		0x0A	/* Reset Interrupt
									 * Self Test
									 * Soft Reset
									 * Sleep */

typedef uint8_t boolean;

namespace BMA{
	extern boolean ShadowDisable;
	extern boolean Sleep;
	extern int X,Y,Z;
	extern uint8_t X_LSB,X_MSB;
	extern uint8_t Y_LSB,Y_MSB;
	extern uint8_t Z_LSB,Z_MSB;

	extern int read_axis(uint8_t* tmp);
	extern uint8_t read_register(uint8_t reg);
	extern void set_register(uint8_t reg, uint8_t value);

	extern void begin();
	extern void begin(uint8_t range, uint8_t bandwidth);

	extern void sleep(boolean);
	extern void soft_reset();

	extern void set_range(uint8_t value);
	extern void set_bandwidth(uint8_t value);

	extern void shadow_disable(boolean);

	extern int acc_x();
	extern int acc_y();
	extern int acc_z();
	extern void read_all_axes();
}
#endif /* BMA020_H_ */
