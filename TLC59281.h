/* 
 * File:   TLC59281.h
 * Author: Tim Ryan
 *
 * Created 05/12/2013
 *
 */

#ifndef TLC59281_H
#define	TLC59281_H

#include <stdint.h>

#define TLC1_TRIS_LATCH TRISBbits.TRISB2
#define TLC1_TRIS_BLANK TRISBbits.TRISB3
#define TLC1_LAT_LATCH LATBbits.LATB2
#define TLC1_LAT_BLANK LATBbits.LATB3
#define TLC1_PORT_LATCH PORTBbits.RB2
#define TLC1_PORT_BLANK PORTBbits.RB3

#define CHNL1_TRIS TRISAbits.TRISA0
#define CHNL2_TRIS TRISAbits.TRISA1
#define CHNL1_LAT LATAbits.LATA0
#define CHNL2_LAT LATAbits.LATA1
#define CHNL1_PORT PORTAbits.RA0
#define CHNL2_PORT PORTAbits.RA1

#define MAX_COLORSLICE 8
#define MAX_BARGRAPH 2
#define CHARBITS 8
#define METER_STEPS 8
#define METER_SPAN 72
#define MAX_dB 66
#define MIN_dB 0
#define INV_MAX_dB 0x001F

typedef unsigned int meterbuff;
typedef unsigned char colorbuff;

typedef struct tagTIMERBITS {
    unsigned T1:1;
    unsigned T2:1;
    unsigned T3:1;
    unsigned T4:1;
    unsigned T5:4;
} TIMERBITS;
extern TIMERBITS TIMERbits;

typedef struct tagFLAGBITS {
    unsigned InputChannel:1;
    unsigned MuxSlice:1;
    unsigned T3:1;
    unsigned T4:1;
    unsigned T5:4;
} FLAGBITS;
extern FLAGBITS FLAGbits;

extern unsigned char red_gradient[];
extern unsigned char grn_gradient[];

extern char channel;
extern char slice;
extern char flag;

void delay(int delay_time);
void TLC59281_init (void);
meterbuff interleave(colorbuff red, colorbuff grn);
meterbuff maskBar(int a, meterbuff b);
meterbuff maskDot(int a, meterbuff b);
meterbuff reverse(meterbuff b);
void write_LEDs (meterbuff leds);

#endif	/* LCD_H */

