/* 
 * File:   FixedPoint.h
 * Author: Tim Ryan
 *
 * Created on September 30, 2013, 12:06 PM
 */

#ifndef FIXEDPOINT_H
#define	FIXEDPOINT_H

#include <stddef.h>
#include <stdint.h>

#define INV_LOG2_10_dB      (uint16_t)0x0302        // 1/log10(2) * 10 * 256

#define int2fix(a)   (((int)(a))<<16)            //Convert char to fix. a is a char
#define fix2int(a)   ((signed char)((a)>>16))    //Convert fix to char. a is an int


extern char p[6];

int32_t log2fix (uint32_t x, size_t precision);
int32_t sm16x16 (int16_t x, int16_t y);
void sm24x24 (int32_t x, int32_t y);
void sm32x32 (int32_t x, int32_t y);

int32_t makeInt24(char p[]);

#endif	/* FIXEDPOINT_H */

