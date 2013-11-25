
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
#include "TLC59281.h"
#include "init_ADC.h"

/*---------------------------------------------------------------------
  Function Name: Timer1_init
  Description:   Initialize Timer1 module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void Timer1_init( void ){

    T1CONbits.TMR1ON = 0;           // Turn module off
    T1GCONbits.TMR1GE = 0;          // Disable gate function

    TMR1H = 0;
    TMR1L = 0;
    
    T1CONbits.TMR1CS = 0b00;        // Clock source = FOSC/4
    T1CONbits.T1CKPS = 0b00;        // Prescale value = 1:1
    T1CONbits.T1OSCEN = 0;          // Disable T1 oscillator
    T1CONbits.NOT_T1SYNC = 1;       // Do not sync to external clock
    T1CON |= 0b00000010;            // Read in 16 bit mode

    PIR1bits.TMR1IF = 0;            // Clear interrupt flag
    PIE1bits.TMR1IE = 0;            // Disable interrupt

    T1CONbits.TMR1ON = 1;           // Turn module on
}

/*---------------------------------------------------------------------
  Function Name: Timer1_init
  Description:   Initialize Timer1 module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void Timer3_init( void ){

    T3CONbits.TMR3ON = 0;           // Turn module off
    T3GCONbits.TMR3GE = 0;          // Disable gate function

    TMR3H = 0;
    TMR3L = 0;

    T3CONbits.TMR3CS = 0b00;        // Clock source = FOSC/4
    T3CONbits.T3CKPS = 0b00;        // Prescale value = 1:1
    T3CONbits.T3OSCEN = 0;          // Disable T1 oscillator
    T3CONbits.NOT_T3SYNC = 1;       // Do not sync to external clock
    T3CON |= 0b00000010;            // Read in 16 bit mode

    PIR2bits.TMR3IF = 0;            // Clear interrupt flag
    PIE2bits.TMR3IE = 0;            // Disable interrupt

    T3CONbits.TMR3ON = 1;           // Turn module on
}

/*---------------------------------------------------------------------
  Function Name: ECCP1_init
  Description:   Initialize ECCP1 module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void ECCP1_init( void ){

    CCP1CONbits.CCP1M = 0b1011;     // Compare mode; ECCP1 resets TMR1, starts A/D, sets CCP1IF bit
    CCP1CONbits.DC1B = 0;           // Unused in compare mode
    CCP1CONbits.P1M = 0;            // Unused in compare mode

    CCPR1H = 0x2E;
    CCPR1L = 0xE0;

    CCPTMRS0bits.C1TSEL = 0b000;    // ECCP1 is based off of TMR1/TMR2
    PIR1bits.CCP1IF = 0;            // Clear interrupt flag
    PIE1bits.CCP1IE = 1;            // Enable interrupt
}

/*---------------------------------------------------------------------
  Function Name: ECCP2_init
  Description:   Initialize ECCP2 module
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/
void ECCP2_init( void ){

    CCP2CONbits.CCP2M = 0b1011;     // Compare mode; ECCP2 resets TMR3, starts A/D, sets CCP1IF bit
    CCP2CONbits.DC2B = 0;           // Unused in compare mode
    CCP2CONbits.P2M = 0;            // Unused in compare mode

    CCPR2H = 0x04;
    CCPR2L = 0x80;
    
    CCPTMRS0bits.C2TSEL = 0b001;    // ECCP2 is based off of TMR3/TMR4
    PIR2bits.CCP2IF = 0;            // Clear interrupt flag
    PIE2bits.CCP2IE = 1;            // Enable interrupt
}

/*---------------------------------------------------------------------
  Function Name: ISR_ECCP1
  Description:   ECCP1 interrupt handler
  Inputs:        None
  Returns:       None
-----------------------------------------------------------------------*/

extern MeterChannel ChnlA, ChnlB;

void ISR_ECCP1(void){

    meterbuff leds = 0;
    uint8_t red, grn;

    if(++channel >= MAX_BARGRAPH) channel = 0;      // cycle through channels
    if(channel == 0)
        if(++slice >= MAX_COLORSLICE) slice = 0;    // increment color slice on channel 0
    
    if(channel) {   // channel B
        red = ChnlB.red_gradient[slice];
        grn = ChnlB.grn_gradient[slice];
        leds = interleave(red, grn);
        leds = maskBar(ChnlB.nBar, leds);
        leds = maskDot(ChnlB.nDot, leds);
        if(ChnlB.direction)
            leds = reverse(leds);
    } else {  // channel A
        red = ChnlA.red_gradient[slice];
        grn = ChnlA.grn_gradient[slice];
        leds = interleave(red, grn);
        leds = maskBar(ChnlA.nBar, leds);
        leds = maskDot(ChnlA.nDot, leds);
        if(ChnlA.direction)
            leds = reverse(leds);
    }

    write_LEDs(leds);

}


void ISR_ECCP2(void) {
    ADCON0bits.GO = 1;    // simulator
}


void ISR_Timer1(void) {
}


void ISR_Timer3(void) {
}
