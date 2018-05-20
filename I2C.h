/* 
 * File:   I2C.h
 * Author: Michael Ding
 *
 * Created summer 2016
 *
 * Edited by Tyler Gamvrelis, summer 2017
 */

#ifndef I2C_H
#define	I2C_H

/********************************** Includes **********************************/
#include <xc.h>
#include "configBits.h"

/********************************** Defines ***********************************/
#define ACK     0
#define NACK    1

/****************************** Public Interfaces *****************************/
void I2C_Master_Init(const unsigned long c);
void I2C_Master_Start(void);
void I2C_Master_RepeatedStart(void);
void I2C_Master_Stop(void);
void I2C_Master_Write(unsigned byteToWrite);
unsigned char I2C_Master_Read(unsigned char ackBit);

#endif /* I2C_H */