/* 
 * File:   init_SPI.h
 * Author: Tim Ryan
 *
 * Created on September 23, 2013, 4:59 PM
 */

#ifndef INIT_SPI_H
#define	INIT_SPI_H

#define SPI_TRIS_SDI TRISBbits.TRISB5
#define SPI_TRIS_SDO TRISCbits.TRISC7
#define SPI_TRIS_SCK TRISBbits.TRISB4
#define SPI_LAT_SCK LATBbits.LATB4
#define SPI_PORT_SCK PORTBbits.RB4

void SPI1_init(void);
signed char SPI1_write(char d);
signed char SPI1_read(char *d);
void ISR_SPI1(void);

#endif	/* INIT_SPI_H */

