/* 
 * File:   FixedPoint.c
 * Author: Tim Ryan
 *
 * Created on September 30, 2013, 11:37 AM
 */

#include <xc.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "FixedPoint.h"

int32_t log2fix (uint32_t x, size_t precision)
{
    volatile int32_t b = 1U;
    volatile int32_t y = 0;
    volatile int32_t scale_1 = 1U;
    volatile int32_t scale_2 = 2U;

    scale_1 <<= precision;
    scale_2 <<= precision;
    b <<= (precision - 1);

    if (precision < 1 || precision > 31) {
        errno = EINVAL;
        return INT32_MAX; // indicates an error
    }

    if (x == 0) {
        return INT32_MIN; // represents negative infinity
    }

    while (x < scale_1) {
        x <<= 1;
        y -= scale_1;
    }

    while (x >= scale_2) {
        x >>= 1;
        y += scale_1;
    }

    uint32_t z = x;

    for (size_t i = 0; i < precision; i++) {
        z = z * z >> precision;
        if (z >= scale_2) {
            z >>= 1;
            y += b;
        }
        b >>= 1;
    }

    return y;
}


int32_t sm16x16 (int16_t a, int16_t b) {

/*******************************************************************************
Signed multiply 16x16
Multiply a 16 bit number _x by a 16 bit number _y (low byte in low memory)
FSR0 points to y
FSR1 points to x
FSRs are unchanged
WREG,PRODL,PRODH changed
Puts the 32 bit result in _q
Algorithm merely sums the 4 partial products, however, the
order is chosen to minimize left over carry bits
******************************************************************************/

    volatile int32_t q = 0;

    asm("movlw high(sm16x16@a)");
    asm("movwf FSR0H");
    asm("movlw low(sm16x16@a)");
    asm("movwf FSR0L");

    asm("movlw high(sm16x16@b)");
    asm("movwf FSR1H");
    asm("movlw low(sm16x16@b)");
    asm("movwf FSR1L");

    asm("MOVF INDF0, W");
    asm("MULWF INDF1");             // _x[0] * _y[0] -> PRODH:PRODL
    asm("MOVFF PRODH, sm16x16@q+1");
    asm("MOVFF PRODL, sm16x16@q+0");
    asm("MOVF PREINC0, W");
    asm("MULWF PREINC1");           // _x[1] * _y[1] -> PRODH:PRODL
    asm("MOVFF PRODH, sm16x16@q+3");
    asm("MOVFF PRODL, sm16x16@q+2");

    asm("MOVF POSTDEC0, W");
    asm("MOVF INDF0, W");
    asm("MULWF POSTDEC1");          // _x[0] * _y[1] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF sm16x16@q+1, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC sm16x16@q+2, F");
    asm("CLRF WREG");
    asm("ADDWFC sm16x16@q+3, F");

    asm("MOVF POSTINC0, W");
    asm("MOVF INDF0, W");
    asm("MULWF INDF1");             // _x[1] * _y[0] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF sm16x16@q+1, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC sm16x16@q+2, F");
    asm("CLRF WREG");
    asm("ADDWFC sm16x16@q+3, F");

    if(b < 0) {
        asm("MOVF sm16x16@a+0, W");
        asm("SUBWF sm16x16@q+2");
        asm("MOVF sm16x16@a+1, W");
        asm("SUBWFB sm16x16@q+3");
    }

    if(a < 0) {
        asm("MOVF sm16x16@b+0, W");
        asm("SUBWF sm16x16@q+2");
        asm("MOVF sm16x16@b+1, W");
        asm("SUBWFB sm16x16@q+3");
    }

    return q;
}


void sm24x24 (int32_t x, int32_t y) {

/*******************************************************************************
Signed multiply 24x24
Multiply a 24 bit number x by a 24 bit number y (low byte in low memory)
FSR0 points to x
FSR1 points to y
FSRs are unchanged
WREG,PRODL,PRODH changed
Puts the 48 bit result in _p
Algorithm merely sums the 9 partial products, however, the
order is chosen to minimize left over carry bits
******************************************************************************/

    asm("MOVLW high(sm24x24@x)");
    asm("MOVWF FSR0H");
    asm("MOVLW low(sm24x24@x)");
    asm("MOVWF FSR0L");

    asm("MOVLW high(sm24x24@y)");
    asm("MOVWF FSR1H");
    asm("MOVLW low(sm24x24@y)");
    asm("MOVWF FSR1L");

    asm("MOVF POSTINC0, W");
    asm("MULWF POSTINC1");             // x[0++] * y[0++] -> PRODH:PRODL
    asm("MOVFF PRODL, _p+0");
    asm("MOVFF PRODH, _p+1");
    
    asm("MOVF POSTINC0, W");
    asm("MULWF POSTINC1");           // x[1++] * y[1++] -> PRODH:PRODL
    asm("MOVFF PRODL, _p+2");
    asm("MOVFF PRODH, _p+3");

    asm("MOVF INDF0, W");
    asm("MULWF POSTDEC1");           // _x[2] * _y[2--] -> PRODH:PRODL
    asm("MOVFF PRODL, _p+4");
    asm("MOVFF PRODH, _p+5");

    asm("MOVF POSTDEC0, W");
    asm("MULWF POSTINC1");          // _x[2--] * _y[1++] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+3, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    asm("MOVF POSTDEC0, W");
    asm("MULWF INDF1");             // _x[1--] * _y[2] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+3, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    asm("MOVF POSTINC0, W");
    asm("MULWF POSTDEC1");             // _x[0++] * _y[2--] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+2, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+3, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    asm("MOVF POSTINC0, W");        // _x[1++]
    asm("MOVF POSTDEC1, W");        // _y[1--]

    asm("MOVF POSTDEC0, W");
    asm("MULWF INDF1");             // _x[2--] * _y[0] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+2, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+3, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    asm("MOVF POSTDEC0, W");
    asm("MULWF POSTINC1");          // _x[1--] * _y[0++] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+1, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+2, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+3, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    asm("MOVF INDF0, W");
    asm("MULWF INDF1");             // _x[0] * _y[1] -> PRODH:PRODL
    asm("MOVF PRODL, W");
    asm("ADDWF _p+1, F");           // Add cross
    asm("MOVF PRODH, W");           // products
    asm("ADDWFC _p+2, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+3, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+4, F");
    asm("CLRF WREG");
    asm("ADDWFC _p+5, F");

    if(y < 0) {
        asm("MOVF sm24x24@x+0, W");
        asm("SUBWF _p+3");
        asm("MOVF sm24x24@x+1, W");
        asm("SUBWFB _p+4");
        asm("MOVF sm24x24@x+2, W");
        asm("SUBWFB _p+5");
    }

    if(x < 0) {
        asm("MOVF sm24x24@y+0, W");
        asm("SUBWF _p+3");
        asm("MOVF sm24x24@y+1, W");
        asm("SUBWFB _p+4");
        asm("MOVF sm24x24@y+2, W");
        asm("SUBWFB _p+5");
    }
}


int32_t makeInt24(char p[]) {

    int32_t t = 0;
    int32_t r = 0;

    if(p[5] & 0x80) {   // extend sign
        t = 0xFF;
        t <<= 8;
    }

    t |= p[5];
    t <<= 8; t |= p[4];
    t <<= 8; t |= p[3];

    r = p[2];
    r <<= 8; r |= p[1];
    r <<= 8; r |= p[0];

    r += 1;
    r >>= 23;
    t += r;

    return t;
}