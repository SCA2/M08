// TLC59281.c

//#include <xc.h>
#include <pic18f27j53.h>
#include "TLC59281.h"
#include "init_SPI.h"

TIMERBITS TIMERbits;

void TLC59281_init (void) {

    TLC1_LAT_LATCH = 1;     // Deselect chip
    TLC1_LAT_BLANK = 1;     // Deselect chip
    
    TLC1_TRIS_LATCH = 0;    // Configure as outputs
    TLC1_TRIS_BLANK = 0;

    TLC1_LAT_LATCH = 0;                         // Clear LAT
    while (TLC1_LAT_LATCH != TLC1_PORT_LATCH);  // Verify state
    TLC1_LAT_LATCH = 1;                         // Set LAT
    while (TLC1_LAT_LATCH != TLC1_PORT_LATCH);  // Verify state
    TLC1_LAT_BLANK = 0;                         // Clear BLANK
    while (TLC1_LAT_BLANK != TLC1_PORT_BLANK);  // Verify state
    TLC1_LAT_BLANK = 1;                         // Set BLANK
    while (TLC1_LAT_BLANK != TLC1_PORT_BLANK);  // Verify state

    CHNL1_TRIS = 0;         // Output
    CHNL2_TRIS = 0;         // Output

    CHNL1_LAT = 0;                      // Clear LAT
    while(CHNL1_LAT != CHNL1_PORT);     // Verify state
    CHNL1_LAT = 1;                      // Set LAT
    while(CHNL1_LAT != CHNL1_PORT);     // Verify state
    CHNL2_LAT = 0;                      // Clear LAT
    while(CHNL2_LAT != CHNL2_PORT);     // Verify state
    CHNL2_LAT = 1;                      // Set LAT
    while(CHNL2_LAT != CHNL2_PORT);     // Verify state
}


meterbuff interleave(colorbuff red, colorbuff grn) {

    int bitnum = sizeof(colorbuff) * CHARBITS;
    meterbuff leds = 0;
    meterbuff mask = 1;
        
    int i = 0;

    for (i = 0; i < bitnum; i++) {                //1010
        leds |= (red & mask << i) << (i + 1);     //0101
        leds |= (grn & mask << i) << i;           //10011001
    }

    return leds;
}


meterbuff maskBar(int a, meterbuff leds) {

    int bitnum = sizeof(colorbuff) * CHARBITS;
    meterbuff mask = 0xFFFF;

    if(a <= bitnum) {
        mask >>= (bitnum - a) * 2;
        leds &= mask;
    }

    return leds;
}


meterbuff maskDot(int a, meterbuff leds) {

    int bitnum = CHARBITS * sizeof(int);
    meterbuff mask = 1;
    a--;

    if(a < bitnum) {
        leds &= ~(mask << (a*2));   // turn off green LED
        leds |= (mask << (a*2+1));  // turn on red LED
    }

    return leds;
}


meterbuff reverse(meterbuff v) {

    meterbuff r = v & 0b11;                 // first get LSBs of v
    int s = sizeof(v) * CHARBITS / 2 - 1;   // extra shift needed at end

    for (s; s > 0; s--)
    {
      r <<= 2;
      v >>= 2;
      r |= v & 0b11;
    }

    return r;   //r will be reversed bit pairs of v;
}



void write_LEDs (meterbuff leds) {

    char lo8 = (char)(leds & 0x00FF);
    char hi8 = (char)((leds & 0xFF00) >> 8);

    // Write to TLC1
    TLC1_LAT_BLANK = 1;                         // Assert BLANK
    while(TLC1_LAT_BLANK != TLC1_PORT_BLANK);   // Verify state
    SPI1_write(hi8);                            // Write 8 bits
    SPI1_write(lo8);                            // Write 8 bits
    
    TLC1_LAT_LATCH = 0;                         // Clear LATCH
    while(TLC1_LAT_LATCH != TLC1_PORT_LATCH);   // Verify state
    TLC1_LAT_LATCH = 1;                         // Set LATCH
    while(TLC1_LAT_LATCH != TLC1_PORT_LATCH);   // Verify state
    TLC1_LAT_LATCH = 0;                         // Clear LATCH
    while(TLC1_LAT_LATCH != TLC1_PORT_LATCH);   // Verify state

    if(channel) {                               // enable channel B
        CHNL1_LAT = 1;                          // Set LAT
        while(CHNL1_LAT != CHNL1_PORT);         // Verify state
        CHNL2_LAT = 0;                          // Clear LAT
        while(CHNL2_LAT != CHNL2_PORT);         // Verify state
    } else {                                    // enable channel A
        CHNL1_LAT = 0;                          // Clear LAT
        while(CHNL1_LAT != CHNL1_PORT);         // Verify state
        CHNL2_LAT = 1;                          // Set LAT
        while(CHNL2_LAT != CHNL2_PORT);         // Verify state
    }

    TLC1_LAT_BLANK = 0;                         // Clear BLANK
    while(TLC1_LAT_BLANK != TLC1_PORT_BLANK);   // Verify state

    return;
}

