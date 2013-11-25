/**********************************************************************
* © 2013 Seventh Circle Audio LLC
*
* FileName:        init_ECCP.c
* Dependencies:    p18f27j53.h
* Processor:       dsPIC33E
* Compiler:        MPLAB® C30 v2.01 or higher
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ADDITIONAL NOTES:
*
**********************************************************************/

#include <p18f27j53.h>
#include "init_SPI.h"

void SPI1_init(void) {

    SSPCON1bits.SSPEN = 0;  // disable SPI1

    SPI_LAT_SCK = 0;                    // Clock line to 0
    while(SPI_PORT_SCK != SPI_LAT_SCK); // Verify pin state
    SPI_TRIS_SCK = 0;                   // Output
    SPI_TRIS_SDI = 1;                   // Input
    SPI_TRIS_SDO = 0;                   // Output

    /* SSPxSTAT
    bit 7 SMP: Sample bit
        1 = Input data sampled at end of data output time
        0 = Input data sampled at middle of data output time
    bit 6 CKE: SPI Clock Select bit(1)
        1 = Transmit occurs on transition from active to Idle clock state
        0 = Transmit occurs on transition from Idle to active clock state
    bit 0 BF: Buffer Full Status bit
        1 = Receive complete, SSPxBUF is full
        0 = Receive not complete, SSPxBUF is empty
    */

    SSP1STATbits.SMP = 1;
    SSP1STATbits.CKE = 0;

    /* SSPxCON1
    bit 7 WCOL: Write Collision Detect bit
        1 = The SSPxBUF register is written while it is still transmitting the previous word
        0 = No collision
    bit 6 SSPOV: Receive Overflow Indicator bit (SPI Slave mode)
        1 = A new byte is received while the SSPxBUF register is still holding the previous data.
        0 = No overflow
    bit 5 SSPEN: Master Synchronous Serial Port Enable bit(2)
        1 = Enables serial port and configures SCKx, SDOx, SDIx and SSx as serial port pins
        0 = Disables serial port and configures these pins as I/O port pins
    bit 4 CKP: Clock Polarity Select bit
        1 = Idle state for clock is a high level
        0 = Idle state for clock is a low level
    bit 3-0 SSPM<3:0>: Master Synchronous Serial Port Mode Select bits(3)
        0101 = SPI Slave mode, clock = SCKx pin; SSx pin control disabled, SSx can be used as I/O pin
        0100 = SPI Slave mode, clock = SCKx pin; SSx pin control enabled
        0011 = SPI Master mode, clock = TMR2 output/2
        0010 = SPI Master mode, clock = FOSC/64
        0001 = SPI Master mode, clock = FOSC/16
        1010 = SPI Master mode, clock = FOSC/8
        0000 = SPI Master mode, clock = FOSC/4
     */

    SSPCON1bits.CKP = 1;
    SSPCON1bits.SSPM = 0;
    SSPCON1bits.SSPEN = 1;
}


signed char SPI1_write(char d) {

    unsigned char TempVar;

    TempVar = SSP1BUF;          // Clears BF
    PIR1bits.SSP1IF = 0;        // Clear interrupt flag
    SSP1CON1bits.WCOL = 0;	// Clear any previous write collision
    SSP1BUF = d;                // write byte to SSP1BUF register
    if (SSP1CON1 & 0x80)        // test if write collision occurred
        return (-1);            // if WCOL bit is set return negative #
    else
        while(!PIR1bits.SSP1IF);    // wait until bus cycle complete

    return (0);                 // if WCOL bit is not set return non-negative#
}

signed char SPI1_read(char *d) {

    if(SSPCON1bits.SSPEN == 1) {
        while(SSP1STATbits.BF != 1);    // wait for transmit
        *d = SSP1BUF;
    }
    return (0);
}

void ISR_SPI1(void){
}

