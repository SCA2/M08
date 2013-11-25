/* 
 * File:   init_ECCP.h
 * Author: Tim Ryan
 *
 * Created on September 20, 2013, 11:24 AM
 */

#ifndef INIT_ECCP_H
#define	INIT_ECCP_H

void ECCP1_init(void);
void ECCP2_init(void);
void ISR_ECCP1(void);
void ISR_ECCP2(void);
void Timer1_init(void);
void Timer3_init(void);
void ISR_Timer1(void);
void ISR_Timer3(void);

#endif	/* INIT_ECCP_H */

