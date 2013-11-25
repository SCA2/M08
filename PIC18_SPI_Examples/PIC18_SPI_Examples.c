//***********************************************************************************
// This program shows examples of using the SPI port to read and write to a
// serial EEPROM (25LC128 for this example)
//
// Device: PIC18F24K20
// Compiler: Microchip XC8 v1.10
// IDE: MPLAB X v1.5
// Created: Dec 2012
//
// **********************************************************************************
// Pin Connections for this example
// **********************************************************************************
//         PIC18F24K20        25LC128 Serial EEPROM
//         -----------        ---------------------
//          RA6 (CS)  ------------> CS
//          RA7 (WP)  ------------> WP
//          RC3 (CK)  ------------> SCK
//          RC4 (SDI) <------------ SO
//          RC5 (SDO) ------------> SI

// Note: The HOLD pin on the Serial EEPROM is tied to Vcc.
//
//


#include <xc.h> // include standard header file

// set Config bits
#pragma config FOSC=INTIO67, WDTEN=OFF, CP0=OFF,CP1=OFF, CPD=OFF,BOREN=OFF
#pragma config IESO=OFF, FCMEN=OFF, FCMEN=OFF,PBADEN=OFF,CCP2MX=PORTBE
#pragma config STVREN=ON,BORV=27,LVP=OFF

// Definitions
#define _XTAL_FREQ  16000000        // this is used by the __delay_ms(xx) and __delay_us(xx) functions
#define SEE_WP      LATAbits.LATA7  // Write Protect Pin on Serial EEPROM
#define SEE_CS      LATAbits.LATA6  // Chip Select Pin on Serial EEPROM



//*************************************************************************************
unsigned int Read_SPI_StatusReg(void)
{
    unsigned int data_read;
    SEE_CS=0;
    WriteSPI(0x05);         // send read status register command
    data_read = ReadSPI();  // read data byte
    SEE_CS=1;

    return(data_read);  // return contents of status register
}

//**************************************************************************************
void Write_SPI_Byte(unsigned int address,unsigned int data)
{
    unsigned char address_hi, address_lo;

    // first break the address into hi and lo bytes
    address_hi = (address & 0xFF00) >> 8;
    address_lo = (address & 0x00FF);

    // write enable latch must be set before every write operation
    SEE_CS=0;               //
    WriteSPI(0x06);         // set write enable latch command
    SEE_CS=1;               //
    __delay_us(10);

    SEE_CS=0;
    WriteSPI(0x02);         // send write command
    WriteSPI(address_hi);   // send high address byte
    WriteSPI(address_lo);   // send low address byte
    WriteSPI(data);         // send data byte
    SEE_CS=1;

    //__delay_ms(5);                    // now you can wait for max write cycle time
    while(Read_SPI_StatusReg() & 0x01); // or poll the status register until the
                                        // WIP bit goes back low  
}

//**************************************************************************************
// page write is same as byte write, but after first byte you have to manually
// send additional data bytes using putsSPI and then set CS back high
//**************************************************************************************
void Write_SPI_Page(unsigned int address)
{
    unsigned char address_hi, address_lo;

    // first break the address into hi and lo bytes
    address_hi = (address & 0xFF00) >> 8;
    address_lo = (address & 0x00FF);

    // write enable latch must be set before every write operation
    SEE_CS=0;               //
    WriteSPI(0x06);         // set write enable latch command
    SEE_CS=1;               //
    __delay_us(10);

    SEE_CS=0;
    WriteSPI(0x02);         // send write command
    WriteSPI(address_hi);   // send high address byte
    WriteSPI(address_lo);   // send low address byte
   

    // NOTE: After calling the Write SPI Page you must send the actual data bytes
    // using the putsSPI command and then set the CS pin back high
    // to actually write the data to the serial eeprom.
    // Check data sheet for maximum page size for your serial eeprom.

}


//**************************************************************************************
unsigned char Read_SPI_Byte(unsigned int address)
{
    unsigned char data_read;
    unsigned char address_hi, address_lo;

    // first break the address into hi and lo bytes
    address_hi = (address & 0xFF00) >> 8;
    address_lo = (address & 0x00FF);

    SEE_CS=0;
    WriteSPI(0x03);         // send read command
    WriteSPI(address_hi);   // send high address byte
    WriteSPI(address_lo);   // send low address byte
    data_read = ReadSPI();  // read the data byte
    SEE_CS=1;
    return(data_read);      // return the data byte read from the eeprom
}

//**************************************************************************************
// Read_Multiple_SPI_Bytes is similar to single byte read, however, after
// calling this routine you must call the getsSPI command to grab each data byte
// you want and then raise the CS line to finish the command
//**************************************************************************************
void Read_Multiple_SPI_Bytes(unsigned int address)
{

    unsigned char address_hi, address_lo;

    // first break the address into hi and lo bytes
    address_hi = (address & 0xFF00) >> 8;
    address_lo = (address & 0x00FF);

    SEE_CS=0;
    WriteSPI(0x03);         // send read command
    WriteSPI(address_hi);   // send high address byte
    WriteSPI(address_lo);   // send low address byte

    // NOTE: After calling the Read_Multiple_SPI_Bytes you must read the actual
    // data bytes using the getsSPI command and then set the CS pin back high
    // to finish the command

}

//*************************************************************************************
void Read_eeprom_array(void)
{
    unsigned char i,eeprom_address;
    unsigned char eeprom_array[30]; // set up array to store a chunk of eeprom data
    unsigned int number_of_bytes = 20;  // define/set number of bytes to read

    eeprom_address=0;       // init starting address

    for (i=0;i<=number_of_bytes;i++)
    {
        eeprom_array[i] = Read_SPI_Byte(eeprom_address);    // read data into array
        eeprom_address++;
    }
}

//*************************************************************************************
void main ( )
{
    volatile unsigned char eeprom_address,eeprom_data;
    unsigned char sync_mode;    // SPI clock source/speed selection
    unsigned char bus_mode;     // SPI bus mode selection
    unsigned char smp_phase;    // SPI data sampling selection
                                
    // set up oscillator control register
    OSCCONbits.IRCF=0x07;       //set OSCCON IRCF bits to select OSC frequency=16Mhz
    OSCCONbits.SCS=0x02;        //set the SCS bits to select internal oscillator block
   
    SEE_WP=1;           // set write protect pin high
    SEE_CS=1;           // disable SEE for now
    
    //TRISA=0x00; // PORT A Assignments
    TRISAbits.TRISA6 = 0;	// RA6 = CS pin on serial EEPROM
    TRISAbits.TRISA7 = 0;	// RA7 = WP pin on serial EEPROM

    //*************************************************************************
    // SPI module Configuration
    // Uncomment your selection for each variable
    // See device data sheet and PIC18 Perpheral Library docs for more details
    //*************************************************************************

    // Select Master or Slave and clock
    //sync_mode = SPI_FOSC_4;       // Master mode, clock is Fosc/4
    sync_mode = SPI_FOSC_16;        // Master mode, clock is Fosc/16
    //sync_mode = SPI_FOSC_64;      // Master mode, clock is Fosc/64
    //sync_mode = SPI_FOSC_TMR2;    // Master mode, clock is TMR2/2
    //sync_mode = SLV_SSON;         // Slave mode, clock is SCK pin, SS pin control enabled
    //sync_mode = SLV_SSOFF;        // Slave mode, clock is SCK pin, SS pin control disabled

    // Select SPI bus mode
    bus_mode = MODE_00; // use mode 00 for communicating with serial eeprom
    //bus_mode = MODE_01;
    //bus_mode = MODE_10;
    //bus_mode = MODE_11;

    // Select when data is sampled
    //smp_phase = SMPEND; //Input data sampled at end of data output time
    smp_phase = SMPMID; //Input data sampled at middle of data output time


    OpenSPI(sync_mode,bus_mode,smp_phase ); // turn on the SPI port

    //**************************************************************************
    // Now just do some sample SPI operations to show syntax
    //**************************************************************************

    //**************************************************************************
    // first, write a byte of data to a random location
    //**************************************************************************
    eeprom_address=0x01;
    eeprom_data=0xA5;
    Write_SPI_Byte(eeprom_address,eeprom_data); // write the value 0xA5 to address 0x01


    //**************************************************************************
    // now read a single byte from a random address in the eeprom
    //**************************************************************************
    eeprom_address=0x01;    // set address to read
    eeprom_data = Read_SPI_Byte(eeprom_address);    // read the data

    if (eeprom_data == 0xA5)    // do something with the data - this is just
        eeprom_address++;       // for example purposes.



    //**************************************************************************
    // now use a 'page write' to write a string of data starting at address 0x00
    //**************************************************************************
    unsigned char data_string[17] = "Microchip Rocks\0";
    //first write the first byte
    eeprom_address=0x00;
    Write_SPI_Page(eeprom_address);     // initiate page write starting at address 0x00
    putsSPI(data_string);               // now write the string
    SEE_CS=1;                           // end the write command
    while(Read_SPI_StatusReg() & 0x01); // poll status register until the WIP goes low


    //**************************************************************************
    // now read 20 bytes of data into an array using a 'continuous' read
    //**************************************************************************
    unsigned char incoming_data_string[20];
    eeprom_address=0x00;                      // set starting address to read
    Read_Multiple_SPI_Bytes(eeprom_address);  // initiate a multiple-byte read command
    getsSPI(incoming_data_string,20);         // read the data into the string
       
 
    CloseSPI(); // Turn off SPI module
    while (1); // just sit here
}








