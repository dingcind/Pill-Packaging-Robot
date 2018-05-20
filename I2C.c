/*
 * File:   I2C.c
 * Author: Michael Ding
 *
 * Created on August 4, 2016, 3:22 PM
 *
 * Edited by Tyler Gamvrelis, summer 2017
 */

/********************************** Includes **********************************/
#include "I2C.h"

/****************************** Public Interfaces *****************************/
void I2C_Master_Init(const unsigned long clockFreq){
  /* Initializes the MSSP module for I2C mode. All configuration register bits
   * are written to because operating in SPI mode could change them.
   *
   * Arguments: clockFreq, the frequency at which data is to be transferred via
   *                the I2C bus. This value is used to generate the baud rate
   *                according to the formula:
   * 
   *                    clock = FOSC / (4 * (SSPADD + 1))
   * 
   *                Thus, the argument clockFrequency sets the 7 bits of control
   *                signals in the SSPADD register. Thus, the following are the
   *                limitations on the value of clockFreq for FOSC = 40 MHz:
   *                    Minimum: 78125
   *                    Maximum: 10000000
   *
   * Returns: none
   */

    /* Disable the MSSP module. */
    SSPCON1bits.SSPEN = 0;
    
    /* Force data and clock pin data directions. */
    TRISCbits.TRISC3 = 1; // SCL (clock) pin
    TRISCbits.TRISC4 = 1; // SDA (data) pin
    
    /* See PIC18F4620 datasheet, section 17.4 for I2C configuration. */
    SSPSTAT = 0b10000000; // Disable slew rate control for cleaner signals
    SSPCON1 = 0b00101000; // Clear errors & enable the serial port in master mode
    SSPCON2 = 0b00000000; // Set entire I2C operation to idle
    
    /* See section 17.4.6 in the PIC18F4620 datasheet for master mode details.
     * Below, the baud rate is configured by writing to the SSPADD<6:0>
     * according to the formula given on page 172. */
    SSPADD = (_XTAL_FREQ / (4 * clockFreq)) - 1;
}

static void I2C_Master_Wait(){
    /* Private function used to poll the MSSP module status. The static keyword
     * makes it so that files besides I2C.c cannot "see" this function. This
     * function exits when the I2C module is idle.
     *
     * Arguments: none
     *
     * Returns: none
     */
    
    /* Wait while:
     *   1. A transmit is in progress (SSPSTAT & 0x04)
     *   2. A Start/Repeated Start/Stop/Acknowledge sequence has not yet been
     *      cleared by hardware
     */
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}

void I2C_Master_Start(){
    /* Initiates Start condition on SDA and SCL pins. Automatically cleared by 
     * hardware.
     * 
     * Arguments: none
     * 
     * Returns: none
     */
    
    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPCON2bits.SEN = 1; // Initiate Start condition
}

void I2C_Master_RepeatedStart(){
    /* Initiates Repeated Start condition on SDA and SCL pins. Automatically
     * cleared by hardware.
     * 
     * Arguments: none
     * 
     * Returns: none
     */
    
    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPCON2bits.RSEN = 1; // Initiate Repeated Start condition
}

void I2C_Master_Stop(){
    /* Initiates Stop condition on SDA and SCL pins. Automatically cleared by 
     * hardware.
     *
     * Arguments: none
     * 
     * Returns: none
     */
    
    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPCON2bits.PEN = 1; // Initiate Stop condition
}

void I2C_Master_Write(unsigned byteToWrite){
  /* Writes a byte to the slave device currently being addressed.
   * 
   * Arguments: byteToWrite, the byte to be written to the slave device
   *
   * Returns: none
   */

    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPBUF = byteToWrite; // Write byte to the serial port buffer for transmission
}

unsigned char I2C_Master_Read(unsigned char ackBit){
  /* Reads a byte from the slave device currently being addressed.
   *
   * Arguments: ackBit, the acknowledge bit
   *                ackBit == 0 --> acknowledge bit sent; ready for next bit
   *                ackBit == 1 --> no acknowledge bit (NACK); done reading data
   *
   * Returns: byte received
   */

    unsigned char receivedByte;

    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPCON2bits.RCEN = 1; // Enable receive mode for I2C module

    I2C_Master_Wait(); // Wait until receive buffer is full
    receivedByte = SSPBUF; // Read received by from the serial port buffer

    I2C_Master_Wait(); // Ensure I2C module is idle
    SSPCON2bits.ACKDT = ackBit; // Acknowledge data bit
    SSPCON2bits.ACKEN = 1; // Initiate acknowledge bit transmission sequence
    
    return receivedByte;
}