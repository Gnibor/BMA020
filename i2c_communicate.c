#ifndef i2c_communicate_C_
#define i2c_communicate_C_

#include <inttypes.h>
#include <avr/io.h>

/*
 * i2c_communicate ist übernommen von Stefan Frings
 * URL: http://www.mikrocontroller.net/topic/266642
 *
 * mit kleinen änderungen
 */

static uint8_t i2c_communicate(uint8_t slave_address, uint8_t* send_data, uint8_t send_bytes, uint8_t* rcv_data, uint8_t rcv_bytes) {
    uint8_t rcv_count=0;

    // Adresse ein Bit nach links verschieben, um Platz für das r/w Flag zu schaffen
    slave_address=slave_address<<1;

    if (send_bytes>0) {
        // Sende Start
        TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
        while (!(TWCR & (1<<TWINT)));
        uint8_t status=TWSR & 0xf8;
        if (status != 0x08 && status != 0x10) goto error;

        // Sende Adresse (write mode)
        TWDR=slave_address;
        TWCR=(1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xf8) != 0x18) goto error;

        // Sende Daten
        while (send_bytes>0) {
            TWDR=*(send_data);
            TWCR=(1<<TWINT) | (1<<TWEN);
            while (!(TWCR & (1<<TWINT)));
            if ((TWSR & 0xf8) != 0x28) goto error;
            send_data++;
            send_bytes--;
        }
    }

    if (rcv_bytes>0) {
        // Sende START
        TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
        while (!(TWCR & (1<<TWINT)));
        uint8_t status=TWSR & 0xf8;
        if (status != 0x08 && status != 0x10) goto error;

        // Sende Adresse (read mode)
        TWDR=slave_address + 1;
        TWCR=(1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xf8) != 0x40) goto error;

        // Empfange Daten
        while (rcv_bytes>0) {
            if (rcv_bytes==1) {
                // das letzte Byte nicht mit ACK quittieren
                TWCR=(1<<TWINT) | (1<<TWEN);
            }
            else {
                // alle anderen Bytes mit ACK quittieren
                TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWEA);
            }
            while (!(TWCR & (1<<TWINT)));
            uint8_t status=TWSR & 0xf8;
            if (status!=0x50 && status != 0x58) goto error;
            *(rcv_data)=TWDR;
            rcv_data++;
            rcv_bytes--;
            rcv_count++;
        }

    }

    // Sende STOP
    TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    return rcv_count;

    error:
    // Sende STOP
    TWCR=(1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    return 0;
}

#endif
