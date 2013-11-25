
//main.c:

/* TODO: peak display
 * TODO: +48V test
 * TODO: +48V error display
 * TODO: serial communication (midi?)
 * TODO: save user settings to flash
 * TODO: retrive user settings from flash
 */

#include <pic18f27j53.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "TLC59281.h"
#include "init_ADC.h"
#include "init_ECCP.h"
#include "init_SPI.h"
#include "FixedPoint.h"

#ifdef _PIC18F27J53_H_    // Setup configuration bits

// set Config bits
#pragma config CFGPLLEN = ON        // PLL Enable Configuration Bit
#pragma config PLLDIV = 2           // PLL Prescaler Divide by 2 (8 MHz oscillator input)
#pragma config WDTEN =	OFF         // Watchdog Timer
#pragma config STVREN = ON          // Stack Overflow/Underflow Reset
#pragma config XINST =	OFF         // Extended Instruction Set
#pragma config CPUDIV = OSC1        // CPU System Clock Postscaler 1:1
#pragma config CP0 = OFF            // Code Protect
#pragma config CLKOEC = OFF         // EC Clock Out Enable Bit
#pragma config SOSCSEL = HIGH       // T1OSC/SOSC Power Selection Bits
#pragma config IESO = ON            // Internal External Oscillator Switch Over Mode
#pragma config OSC = HSPLL          // Oscillator mode high speed w/ PLL
#pragma config FCMEN =	OFF         // Fail-Safe Clock Monitor
#pragma config WDTPS =	2           // Watchdog Postscaler
#pragma config RTCOSC = INTOSCREF   // RTCC Clock Select
#pragma config DSBOREN = OFF        // Deep Sleep BOR
#pragma config DSWDTPS = 2          // Deep Sleep Watchdog Postscaler
#pragma config DSWDTEN = OFF        // Deep Sleep Watchdog Timer
#pragma config DSWDTOSC = INTOSCREF    // DSWDT uses INTRC
#pragma config ADCSEL = BIT12       // ADC 10 or 12 Bit Select
#pragma config MSSP7B_EN = MSK7     // I2C address masking
#pragma config IOL1WAY = OFF        // IOLOCK One-Way Set Enable bit
#pragma config WPCFG =	OFF         // Write/Erase Protect Configuration Region
#pragma config LS48MHZ = SYS48X8    // Low speed USB
#endif

void OSC_init(void);

MeterChannel ChnlA, ChnlB;

char channel = 0;
char slice = 0;

const char init_red[] = {0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x80};
const char init_grn[] = {0x0F, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x7F, 0x7F};

int main(int argc, char** argv) {

    // Peripheral Initialisation

    OSC_init();
    ADC_init();
    Timer1_init();      // Multiplex display timer
    Timer3_init();      // ADC sample timer
    ECCP1_init();       // Resets Timer1
    ECCP2_init();       // Resets Timer3
    TLC59281_init();
    SPI1_init();
    MeterChannel_init(&ChnlA);
    MeterChannel_init(&ChnlB);

    INTCONbits.GIE = 1; // Enable interrupts
    INTCONbits.PEIE = 1;

    int16_t x = 0;
    int16_t y = 0;
    int32_t z = 0;

    //        ProcessDC(&BufferA[4][0], &BufferA[2][0]);    // 2 offset, 3 offset
    //        ProcessDC(&BufferB[4][0], &BufferB[2][0]);    // 2 offset, 3 offset

    while(1) {
        if(buffer_full) {
            if(buffer_num) {
                ProcessAC(&BufferA[3][0], &BufferA[0][0], &ChnlA);      // 10 signal, 11 +48
                ProcessAC(&BufferA[5][0], &BufferA[1][0], &ChnlB);      // 12 signal, 4 +48
            } else {
                ProcessAC(&BufferB[3][0], &BufferB[0][0], &ChnlA);      // 10 signal, 11 +48
                ProcessAC(&BufferB[5][0], &BufferB[1][0], &ChnlB);      // 12 signal, 4 +48
            }

            if(ChnlA.nAVG > 0) {
                x = log2fix((ChnlA.nAVG << 8), 8);                  // returns log base 2 to 8 places
                y = INV_LOG2_10_dB;                                 // convert to 10*log base 10 (decibel)
                ChnlA.dBAVG = sm16x16(x, y);                        // returns int32_t dB to 16 places
            } else {
                ChnlA.dBAVG = MIN_dB;
            }
            x = (int16_t)(ChnlA.dBAVG >> 8);
            y = (int16_t)INV_MAX_dB;
            z = sm16x16(x, y);
            ChnlA.nBar = z >> 16;
//            ChnlA.nBar = 8;

            if(ChnlA.nPeak > 0) {
                x = log2fix((ChnlA.nPeak << 8), 8);                 // returns log base 2 to 8 places
                y = INV_LOG2_10_dB;                                 // convert to 10*log base 10 (decibel)
                ChnlA.dBPeak = sm16x16(x, y);                       // returns int32_t dB to 16 places
            } else {
                ChnlA.dBPeak = MIN_dB;
            }
            x = (int16_t)(ChnlA.dBPeak >> 8);
            y = (int16_t)INV_MAX_dB;
            z = sm16x16(x, y);
            ChnlA.nDot = z >> 16;
//            ChnlA.nDot = 5;

            if(ChnlB.nAVG > 0) {
                x = log2fix((ChnlB.nAVG << 8), 8);                  // returns log base 2 to 8 places
                y = INV_LOG2_10_dB;                                 // convert to 10*log base 10 (decibel)
                ChnlB.dBAVG = sm16x16(x, y);                        // returns int32_t dB to 16 places
            } else {
                ChnlB.dBAVG = MIN_dB;
            }
            x = (int16_t)(ChnlB.dBAVG >> 8);
            y = (int16_t)INV_MAX_dB;
            z = sm16x16(x, y);
            ChnlB.nBar = z >> 16;
//            ChnlB.nBar = 8;

            if(ChnlB.nPeak > 0) {
                x = log2fix((ChnlB.nPeak << 8), 8);                 // returns log base 2 to 8 places
                y = INV_LOG2_10_dB;                                 // convert to 10*log base 10 (decibel)
                ChnlB.dBPeak = sm16x16(x, y);                       // returns int32_t dB to 16 places
            } else {
                ChnlB.dBPeak = MIN_dB;
            }
            x = (int16_t)(ChnlB.dBPeak >> 8);
            y = (int16_t)INV_MAX_dB;
            z = sm16x16(x, y);
            ChnlB.nDot = z >> 16;
//            ChnlB.nDot = 8;

            buffer_full = 0;
            ChnlA.direction = 0;
            ChnlB.direction = 0;
        }
    }
}

void OSC_init(void) {

    OSCTUNEbits.INTSRC = 0;             // 31 kHz derived from INTRC
    OSCTUNEbits.PLLEN = 1;              // PLL enabled
    OSCTUNEbits.TUN = 0;                // No +/- tune

    OSCCONbits.SCS = 0b00;              // 00 = Primary clock source
    OSCCONbits.IDLEN = 0;               // Device enters Sleep mode on SLEEP instruction
    OSCCONbits.IRCF = 0b111;            // 111 = 8 MHz

    OSCCON2bits.SOSCDRV = 1;
    OSCCON2bits.SOSCGO = 1;

    while(OSCCONbits.OSTS != 1);        // Wait for start-up timeout
    while(OSCCONbits.FLTS != 1);        // Wait for INTOSC to stabilize
    while(OSCCON2bits.PRISD != 1);      //
    while(OSCCON2bits.SOSCRUN != 0);    // Verify primary oscillator
}

void GetUserData (void) {
    // User settings
    /*
    unsigned char value1, value2, value3;
    unsigned int DEEdata = 0;
    unsigned int DEEaddr1 = 4, DEEaddr2 = 261, DEEaddr3 = 302;

    // Write settings to EEPROM (test)
    DataEEWrite(DEEdata,DEEaddr1);
    Nop(); Nop();
    DataEEWrite(DEEdata*10,DEEaddr2);
    Nop(); Nop();
    DataEEWrite(DEEdata*20,DEEaddr3);
    Nop(); Nop();

    // Read settings from EEPROM
    value1 = DataEERead(DEEaddr1);
    Nop(); Nop();
    value2 = DataEERead(DEEaddr2);
    Nop(); Nop();
    value3 = DataEERead(DEEaddr3);
    Nop(); Nop();
    */
}


void interrupt ISR_All(void) {

    if(PIR1bits.TMR1IF) {
        ISR_Timer1();
        PIR1bits.TMR1IF = 0;    // Clear Timer interrupt flag
        return;
    }

    if(PIR2bits.TMR3IF) {
        ISR_Timer3();
        PIR2bits.TMR3IF = 0;    // Clear Timer interrupt flag
        return;
    }

    if(PIR1bits.ADIF) {
        PIE1bits.ADIE = 0;      // Disable A/D interrupt
        ISR_ADC();
        PIR1bits.ADIF = 0;      // Clear ADC interrupt flag
        PIE1bits.ADIE = 1;      // Enable A/D interrupt
        return;
    }

    if(PIR1bits.CCP1IF) {
        ISR_ECCP1();            // Handle compare interrupt
        PIR1bits.CCP1IF = 0;    // Clear flag
    }

    if(PIR2bits.CCP2IF) {
        ISR_ECCP2();            // Handle compare interrupt
        PIR2bits.CCP2IF = 0;    // Clear flag
    }

    if(PIR1bits.SSP1IF) {
        ISR_SPI1();             // Handle SPI1 interrupt
        PIR1bits.SSP1IF = 0;    // Simulator
    }
}
