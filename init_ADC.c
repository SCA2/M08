
/**********************************************************************
* © 2013 Seventh Circle Audio LLC
*
* FileName:        init_ADC.c
* Dependencies:    p33EP256GP502.h
* Processor:       dsPIC33E
* Compiler:        MPLAB® C30 v2.01 or higher
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Richard Fischer   07/14/05  ADC module initialization
* Priyabrata Sinha  01/27/06  Ported to non-prototype devices
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
*
*
**********************************************************************/

#include <p18f27j53.h>
#include <string.h>
#include "FixedPoint.h"
#include "init_ADC.h"

#define dBu_SCALAR          0x001A81A7        //53.0129         // Scale input voltage
//#define VU_TC1              0.999933336     // VU attack time constant
//#define VU_TC2              0.99980002      // VU decay time constant
//#define PPM_TC1             0.980198673     // PPM attack time constant
//#define PPM_TC2             0.996007989     // PPM decay time constant
#define VU_TC1              0x00FFFBA2      // VU attack time constant - scaled Q24
#define VU_TC2              0x00FFF2E3      // VU decay time constant - scaled Q24
#define PPM_TC1             0x00FEFA61      // PPM attack time constant - scaled Q24
#define PPM_TC2             0x00FEFA61      // PPM decay time constant - scaled Q24

#define MAX_SLOPE           100
#define MIN_SLOPE           -100


// Number of locations for ADC buffer = 14 (AN0 to AN13) x 8 = 112 words
// Align the buffer to 128 words or 256 bytes. This is needed for peripheral indirect mode
unsigned int BufferA[MAX_CHANNEL][MAX_SAMPLE];
unsigned int BufferB[MAX_CHANNEL][MAX_SAMPLE];

char channel_num = FIRST_CHANNEL;   // AD channel index {2,3,4,10,11,12}
char sample_num = 0;    // AD sample index {0-7}
char buffer_num = 0;    // AD buffer index {0-1}
char buffer_full = 0;   //
char nextChannel[] = {00, 00, 03, 10, 04, 00, 00, 00, 00, 00, 12, 11, 02};         // 2, 3, 10, 12
char getBuffer[] =   {00, 00, 00, 01, 02, 00, 00, 00, 00, 00, 03, 04, 05};         // 2, 3, 10, 12

/*---------------------------------------------------------------------
  Function Name: ADC_init
  Description:   Initialize ADC module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void ADC_init( void ){

    //Configure port pins for analog input
    AN2_TRIS = 1;
    AN3_TRIS = 1;
    AN4_TRIS = 1;
    AN10_TRIS = 1;
    AN11_TRIS = 1;
    AN12_TRIS = 1;

    ANCON0bits.PCFG0 = 1;
    ANCON0bits.PCFG1 = 1;
    ANCON0bits.PCFG2 = 0;
    ANCON0bits.PCFG3 = 0;
    ANCON0bits.PCFG4 = 0;
    ANCON1bits.PCFG8 = 1;
    ANCON1bits.PCFG9 = 1;
    ANCON1bits.PCFG10 = 0;
    ANCON1bits.PCFG11 = 0;
    ANCON1bits.PCFG12 = 0;

    ADCON0bits.ADON = 0;                // Turn AD off for configuration
    ADCON0bits.VCFG = 0;                // Vref+ = AVdd, Vref- = AVss
    ADCON0bits.CHS = FIRST_CHANNEL;     // Channel select

    ADCON1bits.ADFM = 1;		// Results right justified
    ADCON1bits.ADCAL = 0;               // Normal operation
    ADCON1bits.ACQT = 0b010;		// Acquisition time
    ADCON1bits.ADCS = 0b110;		// Conversion clock = FOSC/64

    ADCTRIGbits.TRIGSEL = 0;            // Selects ECCP2 trigger

    PIR1bits.ADIF = 0;                  // Clear the A/D interrupt flag bit
    PIE1bits.ADIE = 1;                  // Enable A/D interrupt
    ADCON0bits.ADON = 1;		// Turn on the A/D converter
}

void ISR_ADC(void) {

    uint16_t tmp = ADRESH; tmp <<= 8; tmp |= ADRESL;

    switch(channel_num) {
        case 2: break;                                      // +48V sense 1
        case 3: break;                                      // +48V sense 2
        case 10: tmp = 0x07FF; break;                       // ChnlA test signal
        case 12: tmp = 0x0040; break;                       // ChnlB test signal
//        case 10: if(tmp > 0x07FF) tmp -= 0x0800; break;     // ChnlA signal
//        case 12: if(tmp > 0x07FF) tmp -= 0x0800; break;     // ChnlB signal
    }

    if(buffer_num == 0)
        BufferA[getBuffer[channel_num]][sample_num] = tmp;
    else
        BufferB[getBuffer[channel_num]][sample_num] = tmp;

    channel_num = nextChannel[channel_num];                     // cycle through AD channels
    if(channel_num == FIRST_CHANNEL) {
        if(++sample_num >= MAX_SAMPLE) sample_num = 0;          // cycle through samples
        if(sample_num == 0) {
            if(++buffer_num >= MAX_BUFFER) buffer_num = 0;      // cycle through buffers
            buffer_full = 1;
        }
    }

    ADCON0bits.CHS = channel_num;                               // Channel select
}

volatile int32_t x;
volatile int32_t y;
char p[6];

void ProcessAC(uint16_t *samples, uint16_t *bias, MeterChannel *c) {

    int i = 0;
    volatile int32_t t1;
    volatile int32_t t2;

    int32_t VU_Attack = (long)VU_TC1;
//    int32_t VU_Decay = (long)VU_TC2;
    int32_t PPM_Attack = (long)PPM_TC1;
//    int32_t PPM_Decay = (long)PPM_TC2;

    for(i = 0; i < MAX_SAMPLE; i++){

        t1 = samples[i];
        t1 = sm16x16(t1, t1);   // sample = sample**2
/*
        if(t1 > c->last_t1 && c->slope_t1 < MAX_SLOPE)
            c->slope_t1++;
        if(t1 < c->last_t1 && c->slope_t1 > MIN_SLOPE)
            c->slope_t1--;
*/
        x = PPM_Attack;
        y = (c->nPeak - t1);
        sm24x24(x, y);
        t2 = makeInt24(p);
        c->nPeak = t1 + t2;

        x = VU_Attack;
        y = (c->nAVG - t1);
        sm24x24(x, y);
        t2 = makeInt24(p);
        c->nAVG = t1 + t2;
    }
}

extern const char init_red[];
extern const char init_grn[];

void MeterChannel_init(MeterChannel *c) {
    c->nAVG = 0;
    c->dBAVG = 0;
    c->nPeak = 0;
    c->dBPeak = 0;
    c->nBar = 0;
    c->nDot = 0;
    c->last_t1 = 0;
    c->last_t2 = 0;
    c->slope_t1 = 0;
    c->slope_t2 = 0;

    c->direction = 0;
    strncpy(c->red_gradient, init_red, 8);
    strncpy(c->grn_gradient, init_grn, 8);
}
