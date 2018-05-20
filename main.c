
/***** Includes *****/
#include <xc.h>
#include "configBits.h"
#include "lcd.h"
#include "codes.h"
#include "GLCD_PIC.h"
#include "I2C.h"
#include "UART_PIC.h"
#include "SPI_PIC.h"

/*Macros*/
#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10

/***** Constants *****/
const char keys[] = "123A456B789C*0#D"; 
const char happynewyear[7] = {  0x30, // 45 Seconds 
                                0x16, // 59 Minutes
                                0x22, // 24 hour mode, set to 23:00
                                0x02, // Day of Week
                                0x08, // Day of Month
                                0x04, // Month
                                0x18  // Year
};

/****** Variable Codes ******/
char codeNum = 1;
/*codeNum legend*/
    // 1 = R
    // 2 = F
    // 3 = L
    // 4 = RF
    // 5 = RL   
    // 6 = FL
    // 7 = RFL

char ToD = 1; 
/*ToD legend*/
    // 1 = O (morning)
    // 2 = A (afternoon)
    // 3 = B (both)
    // 4 = N (alternating)
    // 5 = notN (inverted alternating) 

char freq = 1; 
/*freq legend*/
    // 1 = everyday
    // 2 = every other day starting Sunday 
    // 3 = every other day starting Monday 


/*Number of R, F, and L*/
char pillNumR = 0;
char pillNumF = 0;
char pillNumL = 0;
char remainingR = 0;
char remainingF = 0;
char remainingL = 0;

/*In process of selecting R, F, or L*/
char choosingR = 1;
char choosingF = 1;
char choosingL = 1;

/*stand by mode*/
char StandbyMode = 1;

/*timer*/
int startMin;
int startSec;
int endMin;
int endSec;
int operationMin;
int operationSec;

/*User Input Mode*/
//UserInputMode = 1 when receiving instructions, 0 when completed
char UserInputMode = 1; 
char showFinalSummary = 0;
char screen = 1;

/*Pill Box Array*/
char pillBox[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1};

void main(void) {
    
    // <editor-fold defaultstate="collapsed" desc="Machine Configuration">
    /********************************* PIN I/O ********************************/
    /* Write outputs to LATx, read inputs from PORTx. Here, all latches (LATx)
     * are being cleared (set low) to ensure a controlled start-up state. */  
    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;

    /* After the states of LATx are known, the data direction registers, TRISx
     * are configured. 0 --> output; 1 --> input. Default is  1. */
    TRISA = 0xFF; // All inputs (this is the default, but is explicated here for learning purposes)
    TRISB = 0xFF;
    TRISC = 0b10000000; /* RC3 is SCK/SCL (SPI/I2C),
                         * RC4 is SDA (I2C),
                         * RC5 is SDA (SPI),
                         * RC6 and RC7 are UART TX and RX, respectively. */
    TRISD = 0b00000001; /* RD0 is the GLCD chip select (tri-stated so that it's
                         * pulled up by default),
                         * RD1 is the GLCD register select,
                         * RD2 is the character LCD RS,
                         * RD3 is the character LCD enable (E),
                         * RD4-RD7 are character LCD data lines. */
    TRISE = 0b00000100; /* RE2 is the SD card chip select (tri-stated so that
                         * it's pulled up by default).
                         * Note that the upper 4 bits of TRISE are special 
                         * control registers. Do not write to them unless you 
                         * read §9.6 in the datasheet */
    
    /************************** A/D Converter Module **************************/
    ADCON0 = 0x00;  // Disable ADC
    ADCON1 = 0b00001111; // Set all A/D ports to digital (pg. 222)
    // </editor-fold>
    
    INT1IE = 1; // Enable RB1 (keypad data available) interrupt
    ei(); // Enable all interrupts
    
    /* Initialize LCD. */
    initLCD();
    __lcd_display_control(1, 0, 0); //turns cursor off
    
    /* Initialize GLCD. */
    initGLCD(); 
    glcd(0);
    
    /* Initialize UART. */
    UART_Init(9600);
    
    /*Initialize I2C Master with 100 kHz clock*/
    I2C_Master_Init(100000);

    /*Initialize RTC*/
    //RTC_setTime(); //comment out after time has been set on PIC 
    
    resetAllVariables();
    
    /*stand by mode*/ 
    __lcd_clear();
    standby();
    
    /* Receive User Input */
    while(UserInputMode){
        if (StandbyMode == 1){
            standby();
        }
        else if (chose_prescription == 0) lcd_prescription(codeNum); 
        else if (chose_pillNum == 0) lcdPillNum();
        else if (chose_ToD == 0) lcd_ToD(ToD);
        else if (chose_frequency == 0) lcd_freq(freq);
    }
        
    __lcd_clear();
    __lcd_home();
    printf("Machine in use");
    
    /*stores start time*/
    startTime();
    
    spiInit(4);
    glcd(1);
    
    /*EEPROM*/
    eepromAddData1();
    
    /*performs operation*/
    
    
    //1. scan box !!Mount over blue side!!
    if (!CorrectBoxOrientation()){
        //modify instructions
        modifyInstructions();
    }
    
    //convert instructions to array
    createPillBoxArray();
    
    //open lids
    openLids();
    
    glcd(2);
      
    //2. rotate and dispense
    RotateAndDispense();
    
    //3. close lids
    //closeLids();
    
    //4. Display Termination Msg
    __lcd_home();
    printf("Process Complete   ");
    I2C_Master_Init(100000);
    endTime();
    operationTime();
    
    //EEPROM
    eepromAddData2();

    spiInit(4);
    glcd(20);
    I2C_Master_Init(100000);
    __delay_ms(2000);
    
    //5. Enter standby mode
    __lcd_clear();
    StandbyMode = 1;
    standby();
    
    //6. on interrupt:
    showFinalSummary = 1;
    while(showFinalSummary == 1){
        displayFinalSummary(screen);
    }
    
    //stores last 4 runs
    // function to select run on LCD
    
    
    //showFinalSummary = 1;
    //char runSelected = 2;
    //eepromRetrieveData(runSelected); 
    
    resetAllVariables();
}

void interrupt interruptHandler(void){
    /* This function is mapped to the interrupt vector, and so is called when
     * any interrupt occurs to handle it. Note that if any interrupt is enabled,
     * and it does not have a handler to clear it, then unexpected behavior will
     * occur, perhaps even causing the PIC to reset.
     *
     * Arguments: none
     * 
     * Returns: none
     */
    
    if(INT1IF){
        /* Interrupt on change handler for RB1. */
        
        /*Standby Mode*/
        if (StandbyMode == 1){
            if (KBI0 && KBI1 && KBI2){
                unsigned char arr[1];
                arr[0] = 'Y';
                uartTransmitBlocking(arr, 1);
                
                 unsigned char *arr1;
            
                arr1[0] = 'A';
                while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                    uartReceiveBlocking(1);
                    arr1 = UART ->_dataRX;
                }
            }
            else {StandbyMode = 0;}
        }

        else if (UserInputMode && !KBI0 && !KBI1 && KBI2 && KBI3){
            resetAllVariables();
        }
        
        else if (showFinalSummary == 1){
             screen = screenISR(screen);
        }        
        
        else if (UserInputMode && KBI0 && KBI1 && KBI2){
            //reset
            pillNumR = 0;
            pillNumF = 0;
            pillNumL = 0;
            codeNum = 1;
            ToD = 1;
            freq = 1;
            
            choosingR = 1;
            choosingF = 1;
            choosingL = 1;
            
            chose_prescription = 0;
            chose_pillNum = 0;
            chose_ToD = 0;
            chose_frequency = 0;
            /*add other resets here later*/
        }
        
        /*Selecting prescription*/
        else if (chose_prescription == 0){
            // select prescription
            codeNum = prescriptionISR(codeNum);
        }
        
        else if (chose_pillNum == 0){
            // select number of each pill
            if (pillNumR != 0 && choosingR != 0) pillNumR = lcdPillRISR(pillNumR);
            else if (pillNumF != 0 && choosingF != 0) pillNumF = lcdPillFISR(pillNumF);
            else if (pillNumL != 0 && choosingL != 0) pillNumL = lcdPillLISR(pillNumL); 
        }

        else if(chose_ToD == 0){
            ToD = ToDISR(ToD);
        }
        
        else if(chose_frequency == 0){
            freq = FreqISR(freq);
        }
        
        INT1IF = 0;  // Clear interrupt flag bit to signify it's been handled
    }
}

void eepromReset(){
    EEPROM_write(0x00, 0);
}

unsigned char eepromHandler(){
    //address 0x00 contains items in EEPROM
    //address 0x00 + 1*__ contains earliest run
    //address 0x00 + 2*__ contains 2nd run
    //address 0x00 + 3*__ contains 3rd run
    //address 0x00 + 4*__ contains 4th run
    
    //returns the number of items in EEPROM
    
    
    unsigned char val = EEPROM_read(0x00);
    if (val == 1 || val == 2 || val == 3 || val == 4){
        return val;
    }
    else {
        EEPROM_write(0x00, 0);
        return 0;
    }   
}

void eepromAddData1(){
    
    unsigned char numRFL = pillNumR*100+pillNumF*10+pillNumL;
    unsigned char numTF = ToD*10 + freq;
    unsigned char numItems = eepromHandler();
    
    
    unsigned char tmp;
    
    if (numItems < 4){
        numItems += 1;
        EEPROM_write(0x00, numItems);
    }
    
    for (int i=0; i<3; i++){
        for (int j = 0; j<2; j++){
            tmp = EEPROM_read((2-i)*7+(j+1));
            EEPROM_write((3-i)*7+(j+1),tmp);
        }
    }
    
    EEPROM_write(1, numRFL);
    EEPROM_write(2, numTF);
    
}

void eepromAddData2(){
    
    unsigned char remainR = remainingR;
    unsigned char remainF = remainingF;
    unsigned char remainL = remainingL;
    unsigned char opMin = operationMin;
    unsigned char opSec = operationSec;
    unsigned char numItems = eepromHandler();
    
    unsigned char tmp;
    
    if (numItems < 4){
        numItems += 1;
        EEPROM_write(0x00, numItems);
    }
    
    for (int i=0; i<3; i++){
        for (int j = 2; j<7; j++){
            tmp = EEPROM_read((2-i)*7+(j+1));
            EEPROM_write((3-i)*7+(j+1),tmp);
        }
    }

    EEPROM_write(3, remainR);
    EEPROM_write(4, remainF);
    EEPROM_write(5, remainL);
    EEPROM_write(6, opMin);
    EEPROM_write(7, opSec);   
}

void eepromRetrieveData(char runNumber){
    //run number must be 1, 2, 3, 4
    
    char totalRuns = EEPROM_read(0x00);
    __lcd_home();
    printf("Total Runs: %d   ", totalRuns);
    __delay_ms(2000);
    showFinalSummary = 1;
    
    if (runNumber > totalRuns){
        while(showFinalSummary == 1){
            __lcd_home();
            printf("No Data         ");
            __lcd_newline();
            printf("Available       ");
        }
    }
    
    else{
        char data1 = EEPROM_read((runNumber-1)*7+1);
        char data2 = EEPROM_read((runNumber-1)*7+2);
        
        char runR = data1/100; data1 = data1-runR*100;
        char runF = data1/10; data1 = data1-runF*10;
        char runL = data1;
        char runToD = data2/10; data2 = data2-runToD*10;
        char runfreq = data2;
        
        while(showFinalSummary == 1){
            __lcd_home();
            printf("%dR %dF %dL          ", runR, runF, runL);
            __lcd_newline();
            printf("ToD: %d, Freq: %d   ", runToD, runfreq);
        } 
    }
    
}

/*GLCD*/
void glcd(int num){
    
    glcdSetOrigin(ORIGIN_TOP_RIGHT);
    
    if (num == 0){
        /*Display "HI"*/
        glcdDrawRectangle(0, 128, 0, 128, GREEN);
        glcdDrawRectangle(14, 32, 14, 114, WHITE);
        glcdDrawRectangle(55, 73, 14, 114, WHITE);
        glcdDrawRectangle(96, 114, 14, 114, WHITE);
        glcdDrawRectangle(32, 55, 55, 73, WHITE);
    }
    
    else if (num == 1){
        glcdDrawRectangle(0, 128, 0, 128, BLACK);
        glcdDrawRectangle(0, 128, 122, 128, GREEN);
    }
    
    else if (num <= 19){    
        glcdDrawRectangle(0, 128, 122-6*(num-1), 128-6*(num-1), GREEN);
    }
    
    else if (num <= 20){
        glcdDrawRectangle(0, 128, 0, 14, GREEN);
        glcdDrawRectangle(0,128,0,128,GREEN);
        //Check Mark
        for (int j = 0; j<12; j++){
            for (int i=0; i<32; i++){
                glcdDrawPixel(16+i+j, 80+i-j, WHITE);
            }
            for (int k = 0; k <70; k++){
                glcdDrawPixel(48+k-j, 112-k-j, WHITE);
            }
        }
    }

}

/*Standby Mode*/
void standby(){
    unsigned char time[7]; // Create a byte array to hold time read from RTC
    unsigned char i; // Loop counter
    while(StandbyMode == 1){
        //display real date and time
        
        /* Reset RTC memory pointer. */
        I2C_Master_Start(); // Start condition
        I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
        I2C_Master_Write(0x00); // Set memory pointer to seconds
        I2C_Master_Stop(); // Stop condition
    
        /* Read current time. */
        I2C_Master_Start(); // Start condition
        I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
        for(i = 0; i < 6; i++){
            time[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
        }
        time[6] = I2C_Master_Read(NACK); // Final Read with NACK
        I2C_Master_Stop(); // Stop condition

        /* Print received data to LCD. */
        __lcd_home();
        
        printf("%s %02x, 20%02x   ", convertMonth(time[5]), time[4], time[6]);//,time[5],time[4]); // Print date in YY/MM/DD
        __lcd_newline();
        printf("%02x:%02x:%02x        ", time[2],time[1],time[0]); // HH:MM:SS
        __delay_ms(1000);
    }
} /*NO BUGS*/

/*start and end time, times operation*/
char hexToDec(int hex){
    //converts hex to decimal
    char val1 = hex/16;
    char val2 = hex - val1*16;
    return val1*10+val2;
}

void startTime(){
    unsigned char time[7]; // Create a byte array to hold time read from RTC
    unsigned char i; // Loop counter
     
    /* Reset RTC memory pointer. */
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    I2C_Master_Stop(); // Stop condition

    /* Read current time. */
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
    for(i = 0; i < 6; i++){
        time[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
    }
    time[6] = I2C_Master_Read(NACK); // Final Read with NACK
    I2C_Master_Stop(); // Stop condition
    
    startMin = hexToDec(time[1]);
    startSec = hexToDec(time[0]);
}

void endTime(){
    unsigned char time[7]; // Create a byte array to hold time read from RTC
    unsigned char i; // Loop counter
    
    /* Reset RTC memory pointer. */
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010000); // 7 bit RTC address + Write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    I2C_Master_Stop(); // Stop condition

    /* Read current time. */
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010001); // 7 bit RTC address + Read
    for(i = 0; i < 6; i++){
        time[i] = I2C_Master_Read(ACK); // Read with ACK to continue reading
    }
    time[6] = I2C_Master_Read(NACK); // Final Read with NACK
    I2C_Master_Stop(); // Stop condition
    
    endMin = hexToDec(time[1]);
    endSec = hexToDec(time[0]);
}

void operationTime(){

    
    if (startMin > endMin){
        endMin += 60;
    }
    
    if (startSec <= endSec){
        operationSec = endSec - startSec;
        operationMin = endMin - startMin;
    }
    else {
        operationSec = endSec + 60 - startSec;
        operationMin = endMin - 1 - startMin;
    }
}

void displayOperationTime(){
    __lcd_home();
    printf("Operation Time:  ");//,time[5],time[4]); // Print date in YY/MM/DD
    __lcd_newline();
    printf("%d min(s) %ds     ", operationMin, operationSec); // "%01x mins %02xs" HH:MM:SS
}

/*extra functions*/
void convertCodeNumToRFL(char codeNum){
    
    switch(codeNum){
        
        case 1 :
            pillNumR = 1;
            pillNumF = 0;
            pillNumL = 0;
            break;
            
        case 2 :
            pillNumR = 0;
            pillNumF = 1;
            pillNumL = 0;
            break;
        
        case 3 :
            pillNumR = 0;
            pillNumF = 0;
            pillNumL = 1;
            break;
            
        case 4 :
            pillNumR = 1;
            pillNumF = 1;
            pillNumL = 0;
            break;
            
        case 5 :
            pillNumR = 1;
            pillNumF = 0;
            pillNumL = 1;
            break;
            
        case 6 :
            pillNumR = 0;
            pillNumF = 1;
            pillNumL = 1;
            break;
            
        case 7 :
            pillNumR = 1;
            pillNumF = 1;
            pillNumL = 1;
            break;
            
        default :
            chose_prescription = 0;
    }
    return;
} /*NO BUGS*/

char CorrectBoxOrientation(){
    //scans color of box
    //returns 1 if blue, return 0 if purple
    unsigned char *arr;
    arr[0] = 'A';
    uartTransmitBlocking(arr, 1);
    
    uartReceiveBlocking(1); //receives value from arduino
    arr = UART ->_dataRX; //stores received value into arr 
    while(arr[0] != '1'                          && arr[0] != '0'){ //'Z' indicates operation has completed
        uartReceiveBlocking(1);
        arr = UART ->_dataRX;
    }
    //if blue return 1, else return 0
    if (arr[0] == '1'){
        return 1;
    }
    return 0;
}
void modifyInstructions(){
    //modifies value of ToD and freq if box is inverted
    switch(ToD){
        case 4: ToD = 5; //alternating becomes inverted alternating
                break;
        case 1: ToD = 2; //morning -> afternoon
                break;
        case 2: ToD = 1; //afternoon -> morning
                break;
        default: ToD = 3; //default -> both morning and afternoon
                break;
    }   
}

void createPillBoxArray(){
    if (freq == 1){
        if (ToD == 1 || ToD == 3){ 
            char i;
            for (i = 0;i<7;i++){
                pillBox[i] = 1;
            }
        }
        if (ToD == 2 || ToD == 3){
            char j;
            for (j = 7;j<14;j++){
                pillBox[j] = 1;
            }
        }
        else if (ToD == 4){
            char k;
            for (k=0;k<7;k++){
                pillBox[2*k] = 1;//{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1};
            }
        }
        
        else if (ToD == 5){
            char l;
            for (l=0;l<7;l++){
                pillBox[2*l+1] = 1;//{0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1};
            }
        }
    }
    else if (freq == 2){
        if (ToD == 1 || ToD == 3){
            char i;
            for (i=0;i<4;i++){
                pillBox[2*i] = 1;
            }
        }
        if (ToD == 2 || ToD == 3){
            char j;
            for (j=0;j<4;j++){
                pillBox[2*j+7] = 1;
            }
        }
        else if (ToD == 4 || ToD == 5){
            pillBox[0] = 1;
            pillBox[4] = 1;
            pillBox[7] = 1;
            pillBox[11] = 1;
        }
    }
    else if (freq == 3){
        if (ToD == 1 || ToD == 3){
            char i;
            for (i=0; i<3; i++){
                pillBox[2*i+1] = 1;
            }
        }
        if (ToD == 2 || ToD == 3){
            char j;
            for (j=0;j<3;j++){
                pillBox[2*j+8] = 1;
            }
        }
        else if (ToD == 4){
            pillBox[1] = 1;
            pillBox[5] = 1;
            pillBox[10] = 1;
        }
        else if (ToD == 5){
            pillBox[3] = 1;
            pillBox[8] = 1;
            pillBox[12] = 1;
        }
    }
}

void RotateAndDispense(){
    //main rotate and dispense function
    char i;
    for (i=0; i<17; i++){
        RotateToPosition(i);
        glcd(i+3);      
        if (pillBox[i] == 1){
            DispenseR(i);
            DispenseF(i);
            DispenseL(i);
        }
    }
}

void RotateToPosition(char i){
    //rotate a specific number of degrees to reach position i;
    unsigned char arr[1];
    
    if (i==0){
        //send code to go to first position "J"
        arr[0] = 'J';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i==3 || i==10){
        //send code to go to first position "J"
        arr[0] = 'O';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i<7){
        //send code to rotate "n" degrees
        arr[0] = 'K';
        uartTransmitBlocking(arr, 1);
    }
    else if (i == 7) {
        //send code to arduino to rotate "O" degrees
        arr[0] = 'L';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i < 14){
        //send code to arduino to rotate "n" degrees
        arr[0] = 'K';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i == 14){
        // send code to arduino to rotate to first remaining pills position "P"
        arr[0] = 'M';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i == 15){
        //send code to arduino to rotate "p" degrees
        
        arr[0] = 'N';
        uartTransmitBlocking(arr, 1);
    }
    
    else if (i == 16){
        //send code to arduino to rotate "p" degrees
        arr[0] = 'P';
        uartTransmitBlocking(arr, 1);
    }
    
    //while waiting for arduino to complete
    uartReceiveBlocking(1); //receives value from arduino
    unsigned char *arr1;
    arr1 = UART ->_dataRX; //stores received value into arr 
    while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
        uartReceiveBlocking(1);
        arr1 = UART ->_dataRX;
    }
}

void DispenseR(char i){
       
    if (i == 14){
        closeLids();
        I2C_Master_Init(100000);
        endTime();
        operationTime();
        spiInit(4);
        
        if (operationMin < 2 || (operationMin < 3 && operationSec < 15)){
            unsigned char arr[1];
            //send code to arduino to dispense until the end and count
            arr[0] = 'C';
            uartTransmitBlocking(arr, 1);

            //how many remaining pills?
            unsigned char *arr1;
            arr1[0] = 'A';
            while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }

            while(arr1[0] =='Z'){
                uartReceiveBlocking(1);
                arr1 = UART -> _dataRX;
            }

             while(arr1[0] > 128){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }
            remainingR = arr1[0];   
        }    
    }
        
    if (pillNumR!= 0){    
        if (i < 14){
            //dispense pillNumR pills
            char pillsDispensed = 0;
            unsigned char arr[1];
            while (pillsDispensed < pillNumR){
                //send code to Arduino to dispense one pill
                arr[0] = 'B';
                uartTransmitBlocking(arr, 1);
                pillsDispensed ++;
                
                unsigned char *arr1;
                arr1[0] = 'A';
                while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                    uartReceiveBlocking(1);
                    arr1 = UART ->_dataRX;
                }
                
            }
        }
    }
}
void DispenseF(char i){
       
    if (i == 15){
        /*
        I2C_Master_Init(100000);
        endTime();
        operationTime();
        spiInit(4);
        
        if (operationMin < 2 || (operationMin < 3 && operationSec < 30)){
            unsigned char arr[1];
            //send code to arduino to dispense until the end and count
            arr[0] = 'E';
            uartTransmitBlocking(arr, 1);

            //how many remaining pills?
            unsigned char *arr1;
            arr1[0] = 'A';
            while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }

            while(arr1[0] =='Z'){
                uartReceiveBlocking(1);
                arr1 = UART -> _dataRX;
            }

            while(arr1[0] > 128){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }
            remainingF = arr1[0];
        }
        */
    }
        
    if (pillNumF!= 0){    
        if (i < 14){
            //dispense pillNumR pills
            char pillsDispensed = 0;
            unsigned char arr[1];
            while (pillsDispensed < pillNumF){
                //send code to Arduino to dispense one pill
                arr[0] = 'D';
                uartTransmitBlocking(arr, 1);
                pillsDispensed ++;
                
                unsigned char *arr1;
                arr1[0] = 'A';
                while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                    uartReceiveBlocking(1);
                    arr1 = UART ->_dataRX;
                }
                
            }
        }
    }
}
void DispenseL(char i){
       
    if (i == 16){
    /*
        I2C_Master_Init(100000);
        endTime();
        operationTime();
        spiInit(4);        
        
        if ((operationMin < 3 && operationSec < 30) || operationMin < 2){
            unsigned char arr[1];
            //send code to arduino to dispense until the end and count
            arr[0] = 'G';
            uartTransmitBlocking(arr, 1);

            //how many remaining pills?
            unsigned char *arr1;
            arr1[0] = 'A';
            while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }

            while(arr1[0] =='Z'){
                uartReceiveBlocking(1);
                arr1 = UART -> _dataRX;
            }

            while(arr1[0] > 128){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }
            remainingL = arr1[0];
        }
        */
    }
        
    if (pillNumL!= 0){    
        if (i < 14){
            //dispense pillNumR pills
            char pillsDispensed = 0;
            unsigned char arr[1];
            while (pillsDispensed < pillNumL){
                //send code to Arduino to dispense one pill
                arr[0] = 'F';
                uartTransmitBlocking(arr, 1);
                pillsDispensed ++;
                
                unsigned char *arr1;
                arr1[0] = 'A';
                while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                    uartReceiveBlocking(1);
                    arr1 = UART ->_dataRX;
                }
            }
        }
    }
}

void openLids(){
    unsigned char arr[1];
    arr[0] = 'H';
    uartTransmitBlocking(arr, 1);
    unsigned char *arr1;
    arr1[0] = 'A';
    while (arr1[0] != 'Z'){
        uartReceiveBlocking(1);
        arr1 = UART ->_dataRX;
    }
}

void closeLids(){
    unsigned char arr[1];
    arr[0] = 'I';
    uartTransmitBlocking(arr, 1);
    
    unsigned char *arr1;
    arr1[0] = 'A';
    while (arr1[0] != 'Z'){
        uartReceiveBlocking(1);
        arr1 = UART ->_dataRX;
    }
}

/****interrupt service routines****/
char prescriptionISR(char codeNum){
    /*for key press during "prescription: "
     * if key 4 is pressed, highlight item to left
     * if key 5 is pressed, item is selected
     * if key 6 is pressed, highlight item to right 
     * returns new codeNum based on keypad and current codeNum 
     */
    
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (codeNum == 7) codeNum = 0;
        return codeNum + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        chose_prescription = 1;
        convertCodeNumToRFL(codeNum);
        return 1;
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (codeNum == 1) codeNum = 8;
        return codeNum - 1;
    }
} 

char ToDISR(char ToD){
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (ToD == 4) ToD = 0;
        return ToD + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        chose_ToD = 1;
        return ToD;
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (ToD == 1) ToD = 5;
        return ToD - 1;
    }
}

char FreqISR(char freq){
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (freq == 3) freq = 0;
        return freq + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        chose_frequency = 1;
        UserInputMode = 0; // change later
        return freq;
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (freq == 1) freq = 3;
        return freq - 1;
    }
}

char lcdPillRISR(char pillNumR){
    //do something
    
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (pillNumR == 3) pillNumR = 0;
        return pillNumR + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        choosingR = 0;
        if (pillNumF == 0 && pillNumL == 0){ chose_pillNum = 1;}
        if (pillNumR < 3) return pillNumR;
        else {
            __lcd_home();
            printf("INVALID INPUT       ");
            __lcd_newline();
            printf("Please try again");
            __delay_ms(1500);
            choosingR = 1;
            chose_pillNum = 0;
            return 1;
        }
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (pillNumR == 1) pillNumR = 4;
        return pillNumR - 1;
    }
    
}

char lcdPillFISR(char pillNumF){
    //do something
    
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (pillNumF == 3) pillNumF = 0;
        return pillNumF + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        choosingF = 0;
        if (pillNumL == 0){ chose_pillNum = 1;}
        if (pillNumF < 3 && pillNumF + pillNumR + pillNumL <= 4) return pillNumF;
        else {
            __lcd_home();
            printf("INVALID INPUT       ");
            __lcd_newline();
            printf("Please try again");
            __delay_ms(1500);
            choosingF = 1;
            chose_pillNum = 0;
            return 1;
        }
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (pillNumF == 1) pillNumF = 4;
        return pillNumF - 1;
    }
    
}

char lcdPillLISR(char pillNumL){
    //do something
    
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (pillNumL == 3) pillNumL = 0;
        return pillNumL + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        // keep data about code and move onto next screen
        choosingL = 0;
        chose_pillNum = 1; 
        
        if (pillNumL + pillNumR + pillNumF <= 4) return pillNumL;
        else {
            __lcd_home();
            printf("INVALID INPUT       ");
            __lcd_newline();
            printf("Please try again");
            __delay_ms(1500);
            choosingL = 1;
            chose_pillNum = 0;
            return 1;
        }
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (pillNumL == 1) pillNumL = 4;
        return pillNumL - 1;
    }
    
}

char screenISR(char screen){
    
    if (KBI1 && KBI2){
        // if key 6 is pressed
        if (screen == 6) screen = 0;
        return screen + 1;
    }
    
    else if (KBI0 && KBI2) {
        // if key 5 is pressed
        showFinalSummary = 0;
        
        if (screen == 6){
            
            unsigned char arr[1];
            //send code to arduino to dispense until the end and count
            
            arr[0] = 'Q';
            uartTransmitBlocking(arr, 1);

            for (int i = 0; i<29; i++){
                arr[0] = EEPROM_read(i);
                uartTransmitBlocking(arr,1);
            }
            
            unsigned char *arr1;
            
            arr1[0] = 'A';
            while(arr1[0] != 'Z'){ //'Z' indicates operation has completed
                uartReceiveBlocking(1);
                arr1 = UART ->_dataRX;
            }
            
        }
        
        return 1;
    }
    
    else if (KBI2) { 
        // if key 4 is pressed
        if (screen == 1) screen = 7;
        return screen - 1;
    }
}

void RTC_setTime(void){
    /* Writes the happynewyear array to the RTC memory.
     *
     * Arguments: none
     *
     * Returns: none
     */
    
    I2C_Master_Start(); // Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); // Set memory pointer to seconds
    
    /* Write array. */
    for(char i=0; i<7; i++){
        I2C_Master_Write(happynewyear[i]);
    }
    
    I2C_Master_Stop(); //Stop condition
}

void resetAllVariables(){
    
    /*User Input Mode*/
    UserInputMode = 1; 
    showFinalSummary = 0;

    for (int i=0;i<14;i++){
        pillBox[i] = 0;
    }
    
    //reset
    pillNumR = 0;
    pillNumF = 0;
    pillNumL = 0;
    
    remainingR = 0;
    remainingF = 0;
    remainingL = 0;
    
    codeNum = 1;
    ToD = 1;
    freq = 1;

    choosingR = 1;
    choosingF = 1;
    choosingL = 1;

    chose_prescription = 0;
    chose_pillNum = 0;
    chose_ToD = 0;
    chose_frequency = 0;

    /*add other resets here later*/
    StandbyMode = 1;

    unsigned char arr[1];
    arr[0] = 'R';
    uartTransmitBlocking(arr, 1);
    unsigned char *arr1;
    arr1[0] = 'A';
    while (arr1[0] != 'Z'){
        uartReceiveBlocking(1);
        arr1 = UART ->_dataRX;
    }
    
}

void displayFinalSummary(char screen){
    if (screen == 1){
        displayOperationTime();
    }
     
    else if (screen == 2){
        __lcd_home();
        printf("Instructions     ");
        __lcd_newline();
        printf("%dR %dF %dL      ", pillNumR, pillNumF, pillNumL);
    }
    
    else if (screen == 3){
        __lcd_home();
        printf("Time of Day     ");
        __lcd_newline();
        switch(ToD){
            case 1:
                printf("Morning      ");
                break;
            case 2:
                printf("Afternoon    ");
                break;
            case 3:
                printf("Both         ");
                break;
            default:
                printf("Alternating  ");
                break; 
        }
    }
    
    else if (screen  == 4){
        __lcd_home();
        printf("Days of Week       ");
        __lcd_newline();
        switch(freq){
            case 1: 
                printf("Everyday        ");
                break;
            case 2:
                printf("SunTuesThursSat ");
                break;
            default :
                printf("MonWedFri       ");
                break;
        }
    }
    
    else if (screen == 5){
        __lcd_home();
        printf("Remaining Pills ");
        __lcd_newline();
        printf("%dR %dF %dL        ", remainingR, remainingF, remainingL);
    }
    
    else if (screen == 6){
        __lcd_home();
        printf("Get user history");
        __lcd_newline();
        printf("from computer");
    }
}