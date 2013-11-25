/* 
 * File:   init_ADC.h
 * Author: Tim Ryan
 *
 * Created on July 30, 2013, 12:32 PM
 */

#ifndef INIT_ADC_H
#define	INIT_ADC_H

#include <stdint.h>
#include "TLC59281.h"

#define MAX_SAMPLE          8               // Highest Analog input number in Channel Scan
#define MAX_CHANNEL         6               // Highest Analog input number in Channel Scan
#define MAX_BUFFER          2               // Highest Analog input number in Channel Scan
#define FIRST_CHANNEL       2
#define LAST_CHANNEL        12

#define AN2_TRIS TRISAbits.TRISA2
#define AN3_TRIS TRISAbits.TRISA3
#define AN4_TRIS TRISAbits.TRISA5
#define AN10_TRIS TRISBbits.TRISB1
#define AN11_TRIS TRISCbits.TRISC2
#define AN12_TRIS TRISBbits.TRISB0

typedef struct MeterChannel {
    uint32_t nAVG;
    int32_t dBAVG;
    uint32_t nPeak;
    int32_t dBPeak;
    uint8_t nBar;
    uint8_t nDot;
    int32_t last_t1;
    int32_t last_t2;
    uint8_t slope_t1;
    uint8_t slope_t2;
    uint8_t direction;

    colorbuff red_gradient[8];
    colorbuff grn_gradient[8];
} MeterChannel;

extern uint16_t BufferA[MAX_CHANNEL][MAX_SAMPLE];
extern uint16_t BufferB[MAX_CHANNEL][MAX_SAMPLE];

extern char buffer_full, buffer_num;

void ADC_init(void);
void MeterChannel_init(MeterChannel *c);
void ProcessAC(uint16_t *signalBuffer, uint16_t *biasBuffer, MeterChannel *c);
void ISR_ADC(void);

#endif	/* INIT_ADC_H */

