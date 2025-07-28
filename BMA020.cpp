/*
 * Author: Robin Gerhartz
 * Datei: BMA020.cpp
 * Datum: 18.08.2012
 * URL: (kommt noch)
 *
 * Dies ist eine kleine BMA020 Library
 * die ich aus Lerngründen schreibe/geschrieben habe
 * und um das Rad nicht immer wieder neu zu erfinden ;)
 *
 *
 * Für mehr Informationen zu den einzelnen Funktionen
 * guckt ins Datenblatt, alle "public" Funktionen heißen
 * genau wie dort, mit ausnahme von "begin" und "read_all_axes".
 */

#include "BMA020.h"

// Defines
#define BMA020_SLAVE 		0x38

#define REG_CONTROL_2		0x14	/* Range
									 * Bandwidth */

#define REG_Z_MSB			0x07
#define REG_Z_LSB			0x06

#define REG_Y_MSB			0x05
#define REG_Y_LSB			0x04

#define REG_X_MSB			0x03
#define REG_X_LSB			0x02

#define acc_10Bit(LSB,MSB)	(((int8_t)MSB << 2) | LSB >> 6)

#ifndef cbi
	#define cbi(sfr, bit)   (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

namespace BMA{
	// Erzeuge Variabeln //
	boolean ShadowDisable = 0;
	boolean Sleep 		  = 0;

	int X 			= 0;
	uint8_t X_LSB 	= 0;
	uint8_t X_MSB 	= 0;

	int Y 			= 0;
	uint8_t Y_LSB 	= 0;
	uint8_t Y_MSB 	= 0;

	int Z 			= 0;
	uint8_t Z_LSB 	= 0;
	uint8_t Z_MSB 	= 0;

	// Liest eine Achse
	int read_axis(uint8_t* buffer){
		if(!ShadowDisable){
			// Lese LSB und MSB
			i2c_communicate(BMA020_SLAVE,buffer,1,buffer,2);
			return acc_10Bit(buffer[0], buffer[1]); // Fügt LSB und MSB nach Datenblatt zusammen;
		}else{
			buffer[1] = read_register(buffer[0]+0x1); // Lese nur MSB
			buffer[0] = 0;
			return buffer[1];
		}// if-else ShadowDisable
	}

	// Liest ein Register auf dem BMA020
	uint8_t read_register(uint8_t reg){
		i2c_communicate(BMA020_SLAVE, &reg, 1, &reg, 1);
		return reg;
	}

	// Schreibt ein Register auf den BMA020
	void set_register(uint8_t reg, uint8_t value){
		uint8_t buffer[2];
		buffer[0] = reg;
		buffer[1] = value;
		i2c_communicate(BMA020_SLAVE, buffer, 2, 0, 0);
	}

	void begin(){
		// Starte I2C Verbindung
		cbi(TWSR, TWPS0);
		cbi(TWSR, TWPS1);
		TWBR = (uint8_t)3400;
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
	}
	void begin(uint8_t range, uint8_t bandwidth){
		// Starte I2C Verbindung
		cbi(TWSR, TWPS0);
		cbi(TWSR, TWPS1);
		TWBR = (uint8_t)3400;
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

		uint8_t reg = read_register(REG_CONTROL_2); // Lese Register (Ist ein muss!!!)

		reg &= ~(0x1F); // Lösche Bit 0,1,2,3,4
		reg |= (range | bandwidth); // Setzte Messbereich und frequenz

		set_register(REG_CONTROL_2, reg); // Schreibe Register
	}

	// Setzt den BMA020 in den Schlafmodus oder weckt ihn auf
	void sleep(boolean value){
		if(value){
			set_register(REG_CONTROL_0, 0x01); // Setzte Bit 0 in Register 0x0A (BMA020 geht in den Schlafmodus)
			Sleep = 1;
		}else{
			set_register(REG_CONTROL_0, 0x00); // Lösche Bit 0 in Register 0x0A (weckt den BMA020)
			Sleep = 0;
		}
	}

	// Erzeugt einen "soft reset"
	void soft_reset(){
		set_register(REG_CONTROL_0, 0x02); // Schreibe Bit 0 in Register 0x0A
		_delay_us(15);
		Sleep = 0;
	}

	/*
	 * Ändert den Messbereich
	 *
	 * 	mögliche werte:
	 * 		RANGE_2g
	 * 		RANGE_4g
	 * 		RANGE_8g
	 */
	void set_range(uint8_t value){
		if(!Sleep){
			uint8_t reg = read_register(REG_CONTROL_2); // Lese Register (Ist ein muss!!!)

			reg &= ~(0x18); // Lösche Bit 3,4
			reg |= value; // Setzte Messbereich

			set_register(REG_CONTROL_2, reg); // Schreibe Register
		}
	}

	/*
	 * Ändert die Messfrequenz
	 *
	 * 	mögliche werte:
	 * 		BANDWIDTH_25hz
	 * 		BANDWIDTH_50hz
	 *		BANDWIDTH_100hz
	 *		BANDWIDTH_190hz
	 *		BANDWIDTH_375hz
	 *		BANDWIDTH_750hz
	 *		BANDWIDTH_1500hz
	 */
	void set_bandwidth(uint8_t value){
		if(!Sleep){
			uint8_t reg = read_register(REG_CONTROL_2); // Lese Register (Ist ein muss!!!)

			reg &= ~(0x07); // Lösche Bit 0,1,2
			reg |= value; // Setzte Messfrequenz

			set_register(REG_CONTROL_2, reg); // Schreibe Register
		}
	}

	// Erlaubt das auslesen des MSB ohne den LSB auslesen zu müssen
	void shadow_disable(boolean value){
		if(!Sleep){
			if(value){
				set_register(REG_CONTROL_3, 0x88); // Setzte Bit 3 in Register 0x15 (erlauben)
				ShadowDisable = 1;
			}else{
				set_register(REG_CONTROL_3, 0x80); // Lösche Bit 3 in Register 0x15 (verbieten)
				ShadowDisable = 0;
			}
		}
	}

	// Auslesen der einzelnen Achsen
	// mit "shadow disable" überprüfung
	// und Wertspeicherung in Variabeln
	int acc_x(){
		if(!Sleep){
			uint8_t tmp[2];
			tmp[0] = REG_X_LSB;
			X 		= read_axis(tmp);
			X_LSB 	= tmp[0];
			X_MSB 	= tmp[1];
			return X;
		}else return 0;
	}
	int acc_y(){
		if(!Sleep){
			uint8_t tmp[2];
			tmp[0] = REG_Y_LSB;
			Y 		= read_axis(tmp);
			Y_LSB 	= tmp[0];
			Y_MSB 	= tmp[1];
			return Y;
		}else return 0;
	}
	int acc_z(){
		if(!Sleep){
			uint8_t tmp[2];
			tmp[0] = REG_Z_LSB;
			Z 		= read_axis(tmp);
			Z_LSB 	= tmp[0];
			Z_MSB 	= tmp[1];
			return Z;
		}else return 0;
	}

	// Auslesen aller Achsen mit "shadow disable" überprüfung
	// nutzbar über Variabeln
	void read_all_axes(){
		if(!Sleep){
			if(!ShadowDisable){
				uint8_t buffer[6]; // Erzeuge Buffer zum lesen

				// Lese alle Achsen
				buffer[0] = REG_X_LSB;
				i2c_communicate(BMA020_SLAVE,buffer,1,buffer,6);

				X_LSB 	= buffer[0];
				X_MSB 	= buffer[1];
				X		= acc_10Bit(buffer[0], buffer[1]); // Fügt LSB und MSB nach Datenblatt zusammen

				Y_LSB 	= buffer[2];
				Y_MSB 	= buffer[3];
				Y 		= acc_10Bit(buffer[2], buffer[3]); // Fügt LSB und MSB nach Datenblatt zusammen

				Z_LSB 	= buffer[4];
				Z_MSB 	= buffer[5];
				Z 		= acc_10Bit(buffer[4], buffer[5]); // Fügt LSB und MSB nach Datenblatt zusammen
			}else{
				acc_x();
				acc_y();
				acc_z();
			}
		}
	}
}// Ende namespace
