/*
 * File:   lcd.c
 * Author: Michael Ding
 *
 * Created on July 18, 2016, 12:11 PM
 * Edited by Tyler Gamvrelis, summer 2017
 */

#include "lcd.h"
#include "codes.h"

void lcdInst(char data){
    /* Sends a command to a display control register.
     * 
     * Arguments: data, the command byte for the Hitachi controller
     * 
     * Returns: none
     */
    
    RS = 0;
    lcdNibble(data);
    __delay_us(100);
}

void putch(char data){
    /* Sends a character to the display for printing.
     * 
     * Arguments data, the character (byte) to be sent
     * 
     * Returns: none
     */
    
    RS = 1;
    lcdNibble(data);
    __delay_us(100);
}

void lcdNibble(char data){
    /* Low-level byte-sending implementation.
     * 
     * Arguments: data, the byte to be sent
     * 
     * Returns: none
     */
    
    char temp = (unsigned char) (data & 0xF0);
    LATD = (unsigned char) (LATD & 0x0F);
    LATD = (unsigned char) (temp | LATD);

    __PULSE_E();
    
    /* Send the 4 most significant bits (MSb). */
    data = (unsigned char) (data << 4);
    temp = (unsigned char) (data & 0xF0);
    LATD = (unsigned char) (LATD & 0x0F);
    LATD = (unsigned char) (temp | LATD);

    __PULSE_E();
}

void initLCD(void){
    /* Initializes the character LCD.
     *
     * Arguments: none
     *
     * Returns: none
     */
    
    __delay_ms(15);
    lcdInst(0b00110011);
    lcdInst(0b00110010);
    lcdInst(0b00101000);
    lcdInst(0b00001111);
    lcdInst(0b00000110);
    __lcd_clear();
    
    /* Enforce on: display, cursor, and cursor blinking. */
    __lcd_display_control(1, 1, 1);
}

void lcd_set_cursor(unsigned char x, unsigned char y){
    /* Moves the cursor to a specific position using xy-coordinates. The origin,
     * (0, 0) is the top-left character of the display.
     *
     * Arguments: x, the x-coordinate (min: 0, max: 255)
     *            y, the y-coordinate (min:0, max: 255)
     *
     * Returns: none
     */
    
    __lcd_home();
    for(unsigned char i = 0; i < y; i++){__lcd_newline();}
    if(x > 0){
        lcd_shift_cursor(x, 1);
    }
}

void lcd_shift_cursor(unsigned char numChars, unsigned char direction){
    /* Shifts cursor numChars characters.
     *
     * Arguments: numChars, the number of character positions by which the
     *              cursor is to be moved (min: o, max: 255).
     *            direction, the direction for which the shift is to occur.
     *              Direction = 1 --> right
     *              Direction = 0 --> left
     *
     * Returns: none
     */
    
    for(unsigned char n = numChars; n > 0; n--){
        lcdInst((unsigned char)(0x10 | (direction << 2)));
    }
}

void lcd_shift_display(unsigned char numChars, unsigned char direction){
    /* Shifts display numChars characters.
     * 
     * Arguments: numChars, the number of character positions by which the
     *              cursor is to be moved (min: o, max: 255).
     *            direction, the direction for which the shift is to occur.
     *              Direction = 1 --> right
     *              Direction = 0 --> left
     *
     * Returns: none
     */
    
    for(unsigned char n = numChars; n > 0; n--){
        lcdInst((unsigned char)(0x18 | (direction << 2)));
    }
}

void lcd_prescription(char codeNum){
    // line 1 of LCD
    __lcd_home();
    printf("Prescription:     ");

    //line 2 of LCD
    if (codeNum == 1){
    // choose option 1
        __lcd_newline();
        printf("%s", P_code1);
        printf("%s", P_code2);
        printf("%s", P_code3);
        printf("%s", P_code4);
        printf("%s", P_code5);
        printf("%s", P_code6);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

    else if (codeNum == 2){
    // choose option 2
        __lcd_newline();
        printf("%s", P_code2);
        printf("%s", P_code3);
        printf("%s", P_code4);
        printf("%s", P_code5);
        printf("%s", P_code6);
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

    else if (codeNum == 3){
    // choose option 3
        __lcd_newline();
        printf("%s", P_code3);
        printf("%s", P_code4);
        printf("%s", P_code5);
        printf("%s", P_code6);
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

    else if (codeNum == 4){
    // choose option 4
        __lcd_newline();
        printf("%s", P_code4);
        printf("%s", P_code5);
        printf("%s", P_code6);
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("  ");
        __delay_ms(400);
    }    
    
    else if (codeNum == 5){
    // choose option 4
        __lcd_newline();
        printf("%s", P_code5);
        printf("%s", P_code6);
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("  ");
        __delay_ms(400);
    } 
 
    else if (codeNum == 6){
    // choose option 4
        __lcd_newline();
        printf("%s", P_code6);
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("  ");
        __delay_ms(400);
    } 
    
    else if (codeNum == 7){
    // choose option 4
        __lcd_newline();
        printf("%s", P_code7);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("   ");
        __delay_ms(400);
    } 
}

void lcd_pill_R(char pillNumR){
    //asks user how many R pills and returns selection
    
    __lcd_home();
    printf("How many R's?");
    __lcd_newline();
    
    if (pillNumR == 1){

        printf("%d ", one);
        printf("%d ", two);
        printf("%d ", three);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumR == 2){
        
        printf("%d ", two);
        printf("%d ", three);
        printf("                  ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumR == 3){
        
        printf("%d ", three);
        printf("                   ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

}

void lcd_pill_F(char pillNumF){
    //asks user how many F pills and returns selection
    
    __lcd_home();
    printf("How many F's?");
    __lcd_newline();
    
    if (pillNumF == 1){

        printf("%d ", one);
        printf("%d ", two);
        printf("%d ", three);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumF == 2){
        
        printf("%d ", two);
        printf("%d ", three);
        printf("                  ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumF == 3){
        
        printf("%d ", three);
        printf("                   ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

}

void lcd_pill_L(char pillNumL){
    //asks user how many L pills and returns selection
    
    __lcd_home();
    printf("How many L's?");
    __lcd_newline();
    
    if (pillNumL == 1){

        printf("%d ", one);
        printf("%d ", two);
        printf("%d ", three);
        printf("           ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumL == 2){
        
        printf("%d ", two);
        printf("%d ", three);
        printf("                  ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }
    
    else if (pillNumL == 3){
        
        printf("%d ", three);
        printf("                   ");
        
        __delay_ms(200);
        __lcd_newline();
        printf(" ");
        __delay_ms(400);
    }

}

void lcdPillNum(){
    char tmp = 0;
    
    if (pillNumR >= 1 && choosingR != 0){
        lcd_pill_R(pillNumR);
        tmp = 1;
    }
    
    if (pillNumF >= 1 && choosingF != 0 && tmp == 0){
        lcd_pill_F(pillNumF);
        tmp = 1;
    }
    
    if (pillNumL >= 1 && choosingL != 0 && tmp == 0){
        lcd_pill_L(pillNumL);
    }
    
}

void lcd_ToD(char ToD){
    // line 1 of LCD
    __lcd_home();
    printf("Time of Day: ");

    //line 2 of LCD
    if (ToD == 1){
    // choose option 1
        __lcd_newline();
        printf("Morning Afternoon Both Alternating");
        
        __delay_ms(200);
        __lcd_newline();
        printf("       ");
        __delay_ms(400);
    }

    else if (ToD == 2){
    // choose option 2
        __lcd_newline();
        printf("Afternoon Both Alternating");
        
        __delay_ms(200);
        __lcd_newline();
        printf("         ");
        __delay_ms(400);
    }

    else if (ToD == 3){
    // choose option 3
        __lcd_newline();
        printf("Both Alternating    ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("    ");
        __delay_ms(400);
    }

    else if (ToD == 4){
    // choose option 4
        __lcd_newline();
        printf("Alternating        ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("           ");
        __delay_ms(400);
    }    
    
 
}

void lcd_freq(char freq){
    // line 1 of LCD
    __lcd_home();
    printf("Days of Week:     ");

    //line 2 of LCD
    if (freq == 1){
    // choose option 1
        __lcd_newline();
        printf("Everyday SunTueThuSat MonWedFri");
        
        __delay_ms(200);
        __lcd_newline();
        printf("        ");
        __delay_ms(400);
    }

    else if (freq == 2){
    // choose option 2
        __lcd_newline();
        printf("SunTueThurSat MonWedFri");
        
        __delay_ms(200);
        __lcd_newline();
        printf("             ");
        __delay_ms(400);
    }

    else if (freq == 3){
    // choose option 3
        __lcd_newline();
        printf("MonWedFri               ");
        
        __delay_ms(200);
        __lcd_newline();
        printf("         ");
        __delay_ms(400);
    }
 
}

char * convertMonth(int month){

    switch(month){
        case 1:
            return "Jan";
            break;
            
        case 2: 
            return "Feb";
            break;
            
        case 3: 
            return "Mar";
            break;
            
        case 4: 
            return "Apr";
            break;
            
        case 5: 
            return "May";
            break;
            
        case 6:
            return "Jun";
            break;
            
        case 7:
            return "Jul";
            break;
            
        case 8:
            return "Aug";
            break;
            
        case 9:
            return "Sep";
            break;
            
        case 10:
            return "Oct";
            break;
            
        case 11:
            return "Nov";
            break;
            
        case 12:
            return "Dec";
            break;
            
        default:
            return "Bad Input";
            break;
    }

}

void EEPROM_write(unsigned short address, unsigned char data){    
    EECON1bits.WREN = 1;    // Enable writing of EEPROM
    
    // Set address registers
    EEADRH = (unsigned char)(address >> 8);
    EEADR = (unsigned char)address;
    
    EEDATA = data;          // Write our data to the SFR
    EECON1bits.EEPGD = 0;   // Select EEPROM data mem. instead of program mem.
    EECON1bits.CFGS = 0;    // Access flash/EEPROM, NOT config. registers
    
    di(); // Disable interrupts for critical write sequence
    // Mandatory write initialization sequence
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;
    ei(); // Enable interrupts

    // Poll EEIF for write completion (we could use an ISR for it but we'd run
    // into race conditions anyway when we're writing a bunch of data)
    while(PIR2bits.EEIF == 0) {continue;} 
    PIR2bits.EEIF = 0; // Clear interrupt after it occurs
    EECON1bits.WREN = 0; // Disable write for data integrity
    
}

unsigned char EEPROM_read(unsigned short address){
    // Set address registers
    EEADRH = (unsigned char)(address >> 8);
    EEADR = (unsigned char)address;

    EECON1bits.EEPGD = 0;       // Select EEPROM Data Memory
    EECON1bits.CFGS = 0;        // Access flash/EEPROM, NOT config. registers
    EECON1bits.RD = 1;          // Start a read cycle

    // A read should only take one cycle, and then the hardware will clear
    // the RD bit
    while(EECON1bits.RD == 1) {continue;}

    return EEDATA;  // Return data
}