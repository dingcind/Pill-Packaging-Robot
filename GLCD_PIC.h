/* 
 * File:   GLCD_PIC.h
 * Author: Tyler Gamvrelis
 *
 * Created on August 13, 2017, 6:00 PM
 */

#ifndef GLCD_PIC_H
#define	GLCD_PIC_H

/********************************** Includes **********************************/
#include <xc.h>
#include "configBits.h"

/*********************************** Macros ***********************************/
/* Display write commands (one-liners and 2-liners only). */
#define __NOP()     glcdTransfer(INST_NOP, CMD)      // Empty processor cycle
#define __SWRESET(){\
    /* All registers to default state. */   \
    glcdTransfer(INST_SWRESET, CMD); /* Issue command. */   \
    __delay_ms(130); /* Delay specified on pg. 83 of datasheet. */  \
}
#define __SLPIN(){\
    /* Enter sleep mode. */ \
    glcdTransfer(INST_SLPIN, CMD); /* Issue command. */ \
    __delay_ms(130); /* Delay specified on pg. 93 of datasheet to stabilize
                        power circuits. */  \
}
#define __SLPOUT(){\
    /* Exit sleep mode. */ \
    glcdTransfer(INST_SLPOUT, CMD); /* Issue command. */ \
    __delay_ms(130); /* Delay specified on pg. 94 of datasheet to stabilize
                        timing for supply voltages and clock circuits. */  \
}
#define __SETMADCTL(){\
    /* Set the mirror/exchange parameters. */ \
    glcdTransfer(INST_MADCTL, CMD);\
    glcdTransfer(MADCTLbits.reg, MEMWRITE);\
}
#define __PTLON()   glcdTransfer(INST_PTLON, CMD)    // Partial mode on
#define __NORON()   glcdTransfer(INST_NORON, CMD)    // Partial mode off (normal)      
#define __INVOFF()  glcdTransfer(INST_INVOFF, CMD)   // Display inversion off   
#define __INVON()   glcdTransfer(INST_INVON, CMD)    // Display inversion on      
#define __DISPOFF() glcdTransfer(INST_DISPOFF, CMD)  // Turn off display
#define __DISPON()  glcdTransfer(INST_DISPON, CMD)   // Turn on display
#define __RAMWR()   glcdTransfer(INST_RAMWR, CMD)    // Enable display data RAM writing
#define __TEOFF()   glcdTransfer(INST_TEOFF, CMD);   // Tearing effect off
#define __IDMOFF()  glcdTransfer(INST_IDMOFF, CMD)   // Idle mode off
#define __IDMON()   glcdTransfer(INST_IDMON, CMD)    // Idle mode on

/*********************************** Defines **********************************/
/* RD1 is RS, which is DCX (data/command flag) in the datasheet.
 * 
 * RS = 0 --> command to display controller
 * RS = 1 --> data for display data RAM
 */
#define RS_GLCD          LATDbits.LATD1
#define TRIS_RS_GLCD     TRISDbits.TRISD1

/* RD0 is CS, which is CSX (chip enable) in the datasheet. 
 * 
 * CS = 0 --> display selected for control via SPI
 * CS = 1 --> display not selected for control
 */
#define CS_GLCD          LATDbits.LATD0
#define TRIS_CS_GLCD     TRISDbits.TRISD0

/* Arguments for low-level driver, glcdTransfer. */
#define CMD             1
#define MEMWRITE        0

/* Origin locations (relative to the display when mounted on the DevBugger). */
#define ORIGIN_TOP_LEFT        0
#define ORIGIN_TOP_RIGHT       1
#define ORIGIN_BOTTOM_RIGHT    2
#define ORIGIN_BOTTOM_LEFT     3

/********************************** Constants *********************************/
/* Display dimensions addressable in ST7735 controller display data RAM. */
const unsigned char GLCD_ADDRESSABLE_SIZE_HORZ = 128; // 128 pixels
const unsigned char GLCD_ADDRESSABLE_SIZE_VERT = 160; // 160 pixel

/* Display dimensions as seen in the real world. */
const unsigned char GLCD_SIZE_HORZ = 128;    // 0 to 127 --> 128 pixels
const unsigned char GLCD_SIZE_VERT = 128;    // 0 to 127 --> 128 pixels

/* Display command codes (write only, since we don't have hardware to read).
 * These are static which makes them only visible to this compilation unit. */
static const unsigned char INST_NOP = 0x00;         // Empty processor cycle
static const unsigned char INST_SWRESET = 0x01;     // All registers to default state
static const unsigned char INST_SLPIN = 0x10;       // Enter sleep mode
static const unsigned char INST_SLPOUT = 0x11;      // Exit sleep mode
static const unsigned char INST_PTLON = 0x12;       // Partial mode on
static const unsigned char INST_NORON = 0x13;       // Partial mode off (normal)
static const unsigned char INST_INVOFF = 0x20;      // Display inversion off
static const unsigned char INST_INVON = 0x21;       // Display inversion on
static const unsigned char INST_GAMSET = 0x26;      // Set gamma
static const unsigned char INST_DISPOFF = 0x28;     // Turn off display
static const unsigned char INST_DISPON = 0x29;      // Turn on display
static const unsigned char INST_CASET = 0x2A;       // Set column address
static const unsigned char INST_RASET = 0x2B;       // Set row address
static const unsigned char INST_RAMWR = 0x2C;       // Enables RAM writes
static const unsigned char INST_PTLAR = 0x30;       // Partial start/end address
static const unsigned char INST_TEOFF = 0x34;       // Tearing effect off
static const unsigned char INST_TEON = 0x35;        // Tearing effect on
static const unsigned char INST_MADCTL  = 0x36;     // Memory data access control
static const unsigned char INST_IDMOFF = 0x38;      // Idle mode off
static const unsigned char INST_IDMON = 0x39;       // Idle mode on
static const unsigned char INST_COLMOD = 0x3A;      // Interface pixel format
static const unsigned char INST_FRMCTR1 = 0xB1;     // Frame rate control (normal mode/full colors)
static const unsigned char INST_FRMCTR2 = 0xB2;     // Frame rate control (idle mode/8-colors)
static const unsigned char INST_FRMCTR3 = 0xB3;     // Frame rate control (partial mode/full colors)
static const unsigned char INST_INVCTR = 0xB4;      // Display inversion control
static const unsigned char INST_PWCTR1 = 0xC0;      // Power control1
static const unsigned char INST_PWCTR2 = 0xC1;      // Power control2
static const unsigned char INST_PWCTR3 = 0xC2;      // Power control3
static const unsigned char INST_PWCTR4 = 0xC3;      // Power control4
static const unsigned char INST_PWCTR5 = 0xC4;      // Power control5
static const unsigned char INST_VMCTR1 = 0xC5;      // VCOM control 1
static const unsigned char INST_VMOFCTR2 = 0xC7;    // VCOM control 2	

/* 18-bit color depth information here.
 * 
 * Each color is encoded by 3 bytes. For each byte, the 6 most significant bits
 * contain useful information and the two lest significant bits are don't cares.
 * Color = B2B1B0 = RGB.
 * 
 * Thus, for each color component (red, green and blue or RGB for short), there 
 * are 2^6 possible values. Since there are three different colors, there are
 * thus (2^6)^3 = 2^18 = 262,144 colors.
 * 
 * The RGB color model is additive, that is, an arbitrary color in the RGB color
 * space is formed by superimposing the intensities of the separate components.
 * Thus, black is zero intensity for all components, and white is full intensity
 * for all components.
 */
const unsigned long BLACK = 0x000000;
const unsigned long GREY = 0x808080;
const unsigned long WHITE = 0xFFFFFF;
const unsigned long RED = 0xFF0000;
const unsigned long ORANGE = 0xFF8C00;
const unsigned long YELLOW = 0xFFFF00;
const unsigned long GREEN = 0x00FF00;
const unsigned long BLUE = 0x0000FF;
const unsigned long INDIGO = 0x4B0082;
const unsigned long VIOLET = 0x9400D3;

/************************************ Types ***********************************/
/* Define union for MADCTL for easy bit addressing. This will be useful for 
 * determining if rotations are in effect. A union is basically used to provide
 * several different ways of using the same space in memory. In this case, we 
 * are using the same space in memory to define a struct with individual bits we
 * can address, and an unsigned char (byte) that encompasses these bits. This
 * way, we can easily set and test the individual bits while still being able to
 * pass the entire byte into a function without sacrificing efficiency.
 * 
 *  Bits:       MY, mirror y-axis (0 --> top to bottom)
 *              MX, mirror x-axis (0 --> left to right)
 *              MV, row/column exchange (0 --> normal)
 *              ML, scan direction parameter (0 --> refresh top to bottom)
 *              RGB, red, green, and blue pixel position change (0 --> RGB)
 *              MH, horizontal refresh order (0 --> refresh left to right)
 * Unsigned char (byte):
 *              reg, encompasses all the bytes in the struct
 */
typedef union{
    struct{
        unsigned        :1; // LSb
        unsigned        :1;
        unsigned MH     :1;
        unsigned RGB    :1;
        unsigned ML     :1;
        unsigned MV     :1;
        unsigned MX     :1;
        unsigned MY     :1; // MSb 
    };
    unsigned char reg; // Alternate way of accessing the above 8 bits, as a byte
}MADCTLbits_t;

/**************************** External variables ******************************/
extern MADCTLbits_t MADCTLbits;

/****************************** Public Interfaces *****************************/
/* Fundamental communication interface. */
void glcdTransfer(unsigned char byte, unsigned char cmd);

/* Fundamental drawing interfaces. */
void glcdDrawRectangle(unsigned char XS, unsigned char XE,\
                        unsigned char YS, unsigned char YE,\
                        unsigned long color);
void glcdDrawPixel(unsigned char XS, unsigned char YS, unsigned long color);

/* The following two functions may be used by students who:
 *  1. Want to maximize write efficiency to the GLCD by minimizing color depth
 *  2. Want to change the location of the origin (i.e. rotate the display). */
void glcdSetCOLMOD(unsigned char numBitsPerPixel);
void glcdSetOrigin(unsigned char corner);

/* Initialization sequence. */
void initGLCD(void);

#endif	/* GLCD_PIC_H */
