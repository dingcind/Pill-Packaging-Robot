/*Pin Assignments*/
//D2 - Lid Inserter Servo 1 //OPENCLOSE LIDS
//D3 - Lid Closer Servo //OPENCLOSE LIDS
//D4 - Stepper Rotator //ROTATOR
//D5 - Stepper Rotator //ROTATOR
//D6 - Stepper Rotator //ROTATOR
//D7 - Stepper Rotator //ROTATOR
//D8 - Enable Dispenser L //DISPENSE ONE PILL
//D9 - Enable Dispenser F //DISPENSE ONE PILL
//D10 - Stepper Dispenser //DISPENSE ONE PILL
//D11 - Stepper Dispenser //DISPENSE ONE PILL
//D12 - Stepper Dispenser //DISPENSE ONE PILL
//D13 - Stepper Dispenser //DISPENSE ONE PILL
//A0 - Color Sensor //COLOR SENSOR
//A1 - Break Beam Sensor //DISPENSE ONE PILL
//A2 - 
//A3 - 
//A4 - Enable Dispenser R //DISPENSE ONE PILL'
//A5 - Lid Inserter Servo 2 //OPENCLOSE LIDS

/*UART codes*/
//'A' - Scan color of box (Color Sensor)
//'B' - Dispense one R pill (Break Beam and Stepper)
//'C' - Dispense and count all R pills (Break Beam and Stepper)
//'D' - Dispense one F pill (Break Beam and Stepper)
//'E' - Dispense and count all F pills (Break Beam and Stepper)
//'F' - Dispense one L pill (Break Beam and Stepper)
//'G' - Dispense and count all L pills (Break Beam and Stepper)
//'H' - Insert Lids (Servo)
//'I' - Remove and close lids (Servo x2)
//'J' - Rotator - rotate to first ramp AND reset initial position of ramp
//'K' - Rotator - rotate from ramp 1 to 7 and 8 to 14
//'L' - Rotator - rotate from ramp 7 to 8
//'M' - Rotator - rotate from ramp 14 to 15
//'N' - Rotator - rotate from 15 to 16
//'O' - Rotator - rotatoe from 3 to 4 or 10 to 11
//'P' - Rotator - rotate ramp from 16 to 17
//'Q' - PC interface
//'R' - Reset position of all
//'Z' - Operation Complete

/* Includes. */
#include <SoftwareSerial.h>
#include <CheapStepper.h>
#include <Servo.h>

/*Functions*/
void dispenseR(); //dispenses one R pill
void dispenseF(); //dispenses one F pill
void dispenseL(); //dispenses one L pill
int dispenseAndCountR(); //dispenses and counts all R pills and return number of R remaining
int dispenseAndCountF(); //dispenses and counts all F pills and return number of F remaining
int dispenseAndCountL(); //dispenses and counts all l pills and return number of L remaining
void resetRotator(); //resets position of rotator to zero
void resetMicroServo(); //resets the position of the microServo/Lids
void insertLids(); //insert ramps into lids
void removeLids(); //remove ramps from lids
void closeLids(); //push down lids to close

/* Read-only variables. */
static char receivedData[2]; // To hold received characters
static char sentData[1];
const byte rxPin = 1;
const byte txPin = 0;

/* Set up objects. */
SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);

/*Setup Pins*/
char servoLidInserter = 2;
char servoLidCloser = 3;
char rotator1 = 4;
char rotator2 = 5;
char rotator3 = 6;
char rotator4 = 7;
char enableF = 8;
char enableL = 9;
char pillDispenser1 = 10;
char pillDispenser2 = 11;
char pillDispenser3 = 12;
char pillDispenser4 = 13;
char photoResistorPin = A0;
char breakBeamPinR = A1; 
char breakBeamPinF = A1; 
char breakBeamPinL = A1; 
char enableR = A4;
char debugColor = A3;
char servoLidInserter2 = A5;

int blueValue = 0;

/*Stepper*/
CheapStepper rotator (rotator1, rotator2, rotator3, rotator4);
CheapStepper pillDispenser (pillDispenser1, pillDispenser2, pillDispenser3, pillDispenser4);

/*servo*/
Servo LidInserter;
Servo LidInserter2;
Servo LidCloser;

void setup() {

  /* Configure pin modes for tx and rx. */
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  /* Open software serial port with baud rate = 9600. */
  mySerial.begin(9600);//mySerial.begin(9600);

  receivedData[1] = '\0'; // Terminate string so that it will print properly on the LCD

  /*Rotator Motor Setup*/
  rotator.setRpm(10);
  rotator.setTotalSteps(4076);
  rotator.begin();

  /*Pill Dispenser*/
  pillDispenser.setRpm(24);
  pillDispenser.setTotalSteps(4076);
  pillDispenser.begin();
  pinMode(enableR, OUTPUT);
  pinMode(enableF, OUTPUT);
  pinMode(enableL, OUTPUT);
  digitalWrite(enableR, HIGH); //active low
  digitalWrite(enableF, HIGH); //active low
  digitalWrite(enableL, HIGH); //active low

  /*Lid inserter*/
  LidInserter.attach(servoLidInserter);
  LidInserter2.attach(servoLidInserter2);
  LidCloser.attach(servoLidCloser);

  /*Color Sensor*/
  pinMode(photoResistorPin, INPUT);   

  /*BreakBeam Sensor*/
  pinMode(breakBeamPinR, INPUT);
  pinMode(breakBeamPinF, INPUT);
  pinMode(breakBeamPinL, INPUT);  

  /*debug color sensor*/
  pinMode(debugColor, OUTPUT);
}

void loop() {
  /* Wait to receive the message from the main PIC. */
  while(mySerial.available() < 1){ continue; }
  receivedData[0] = mySerial.read(); //mySerial.read();

  //color sensor
  if (receivedData[0] == 'A'){
    char isBlue = myColorSensor();
    if (isBlue == 1) mySerial.write('1');
    else mySerial.write('0');   
  }

  //dispenser code
  if (receivedData[0] == 'B'){
    //dispense one R
    dispenseR();
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'C'){
    // dispense and count
    unsigned char val = dispenseAndCountR();
    mySerial.write('Z');
    mySerial.write(val);
  }

  else if (receivedData[0] == 'D'){
    //dispense one F
    dispenseF();
    mySerial.write('Z');  
  }

  else if (receivedData[0] == 'E'){
    // dispense and count
    unsigned char val = dispenseAndCountF();
    mySerial.write('Z');
    mySerial.write(val);
  }

  else if (receivedData[0] == 'F'){
    //dispense one L
    dispenseL();
    mySerial.write('Z');  
  }

  else if (receivedData[0] == 'G'){
    // dispense and count
    unsigned char val = dispenseAndCountL();
    mySerial.write('Z');
    mySerial.write(val);
  }

  else if (receivedData[0] == 'H'){
    resetMicroServo();
    insertLids();
    mySerial.write('Z');
    }

  else if (receivedData[0] == 'I'){
    /*
    for (char pos = 40; pos <= 90; pos += 1){
      LidCloser.write(pos);
      delay(15);
    }
    */
    removeLids();
    closeLids();
    mySerial.write('Z');
   }
    
  // centre rotator
  else if (receivedData[0] == 'J'){
    lids1to7();
    int rotPos = 30; //degrees
    resetRotator();
    myStepperMotor(rotPos);
    mySerial.write('Z'); 
  }

  else if (receivedData[0] == 'K'){
    int rotPos = 20; //degrees
    myStepperMotor(rotPos);
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'L'){
    for (int pos = 150; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      LidInserter.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    delay(1000); 
    lids7to14();
    
    for (int pos = 0; pos <= 160; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
      LidInserter2.write(pos);
      delay(10);                       // waits 15ms for the servo to reach the position
    }
    
    int rotPos = 298; //degrees
    rotator.moveDegreesCW(rotPos);
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'M'){
    int rotPos = 15; //degrees
    myStepperMotor(rotPos);
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'N'){
    //int rotPos = 28;
    //myStepperMotor(rotPos);
    mySerial.write('Z');
  }
 
  else if (receivedData[0] == 'O'){
    int rotPos = 19;
    myStepperMotor(rotPos);
    
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'P'){
    //int rotPos = 186;
    //myStepperMotor(rotPos);
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'Q'){
    unsigned char eeprom[29];
    for (int i=0; i<29; i++){
      while(mySerial.available() < 1){ continue; }
      eeprom[i] = mySerial.read();
      //mySerial.println(eeprom[i]);
      }
    mySerial.end();

    Serial.begin(9600);
    //Serial.println("SS off HS on");
    char valFromPC = Serial.read();
    
    while(Serial.available()<1){
      continue;
      }

    if (valFromPC == 'd'){
      for (int i=0; i<29; i++){
        //Serial.println(eeprom[i]);
        Serial.write(eeprom[i]);
      }
    }
    Serial.end();

    mySerial.begin(9600);
    mySerial.write('Z');
  }

  else if (receivedData[0] == 'Y'){
    blueValue = analogRead(photoResistorPin);
    mySerial.write('Z');
  }  

  else if (receivedData[0] == 'R'){
    resetAll();
    mySerial.write('Z');
  }  
}

void resetAll(){
  LidInserter.write(0);
  LidInserter2.write(0);
  LidCloser.write(90);  
  //add code for rotator;
  }

void insertLids(){  
  int pos;
  for (pos = 0; pos <= 150; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    LidInserter.write(pos);              // tell servo to go to position in variable 'pos'
    //LidInserter2.write(pos);
    delay(10);                       // waits 15ms for the servo to reach the position
  }
}

void resetMicroServo(){
  LidInserter.write(0);  
  LidCloser.write(90);  
}

void removeLids(){
  int pos;
  for (pos = 150; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    LidInserter2.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  delay(1000); 
}

void closeLids(){
  
  //presses lids to fully closed state 
  int pos;
  /*
  for (pos = 90; pos >= 40; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    LidCloser.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  delay(1000);
  */
  
  for (pos = 45; pos <=140; pos += 1) { // goes from 180 degrees to 0 degrees
    LidCloser.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }  
  delay(1000);
  
    for (pos = 140; pos >= 90; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    LidCloser.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
 }

void lids1to7(){
  int pos;
  delay(200);
  for (pos = 90; pos <= 110; pos += 1){
    LidCloser.write(pos);
    delay(15);
    }
  delay(200);
  }


void lids7to14(){
  int pos;
  delay(200);
  for (pos = 110; pos >= 45; pos -= 1){
    LidCloser.write(pos);
    delay(15);
    }
  delay(200);
  }

/*stepper motor*/
void myStepperMotor(int x){
    rotator.moveDegreesCCW(x);
}

void resetRotator(){
    rotator.moveToCCW(0);
}


char myColorSensor(){
  
  // returns 1 if blue, 0 if not blue
  char val = analogRead(photoResistorPin);
  if  (val < (blueValue + 2)){ 
    //debug blue
    digitalWrite(debugColor, HIGH);
    return 1; //CHANGE THIS!!!
  }
  digitalWrite(debugColor, LOW);
  return 0;
  }

/*Dispenser*/
void dispenseR(){
  digitalWrite(enableR, LOW);
  digitalWrite(enableF, HIGH);
  digitalWrite(enableL, HIGH);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCW(359); //move 90 degrees
    timer = 0;
    while (timer < 20000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        timer++;
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        dispensed = 1;  // stops dispensing mechanism
        changeState = 0;
        Serial.println("One Pill Fell Through");
        pillDispenser.stop();
        delay(250);
        break;
      }
      timer++;
      lastState = currState;
      }
      
    }
  }

void dispenseF(){
  digitalWrite(enableR, HIGH);
  digitalWrite(enableF, LOW);
  digitalWrite(enableL, HIGH);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCCW(359); //move 90 degrees
    timer = 0;
    while (timer < 20000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        timer++;
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        dispensed = 1;  // stops dispensing mechanism
        changeState = 0;
        Serial.println("One Pill Fell Through");
        pillDispenser.stop();
        delay(250);
        break;
      }
      timer++;
      lastState = currState;
      }
      
    }   
  }
  
  void dispenseL(){
  digitalWrite(enableR, HIGH);
  digitalWrite(enableF, HIGH);
  digitalWrite(enableL, LOW);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCCW(359); //move 90 degrees
    timer = 0;
    while (timer < 20000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        timer++;
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        dispensed = 1;  // stops dispensing mechanism
        changeState = 0;
        Serial.println("One Pill Fell Through");
        pillDispenser.stop();
        delay(250);
        break;
      }
      timer++;
      lastState = currState;
      }
      
    }
  }
  
int dispenseAndCountR(){
  digitalWrite(enableR, LOW);
  digitalWrite(enableF, HIGH);
  digitalWrite(enableL, HIGH);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned char count = 0;
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state

  unsigned long latestTime = millis();
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCW(359); //move 90 degrees
    timer = 0;
    
    while (timer < 5000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        changeState = 0;
        count++;
        Serial.println(count);
        currState = 0;
        latestTime = millis();
      }
      
      timer++;
      lastState = currState;
      }
  
      if (millis() - latestTime > 3500){
        dispensed = 1;
        pillDispenser.stop();
       }          
    }
    return count;
  }  
  
int dispenseAndCountF(){
  digitalWrite(enableF, LOW);
  digitalWrite(enableR, HIGH);
  digitalWrite(enableL, HIGH);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned char count = 0;
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state

  unsigned long latestTime = millis();
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCCW(359); //move 90 degrees
    timer = 0;
    
    while (timer < 5000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        changeState = 0;
        count++;
        Serial.println(count);
        currState = 0;
        latestTime = millis();
      }
      
      timer++;
      lastState = currState;
      }
  
      if (millis() - latestTime > 4000){
        dispensed = 1;
        pillDispenser.stop();
       }          
    }
    return count; // because it double counts
  } 

  int dispenseAndCountL(){
  digitalWrite(enableL, LOW);
  digitalWrite(enableF, HIGH);
  digitalWrite(enableR, HIGH);
  
  unsigned char dispensed = 0; //has pill been dispensed
  unsigned char count = 0;
  unsigned int timer = 0; //timer
  char changeState = 0; //has state changed (unbroken->broken += 1; broken->unbroken += 1)
  
  char currState = 0; //curent state
  char lastState = 0; //previous state

  unsigned long latestTime = millis();
  
  while (!dispensed){
    pillDispenser.newMoveDegreesCCW(359); //move 90 degrees
    timer = 0;
    
    while (timer < 5000){ // set this to however long it takes for the thing to turn 90 degrees
      //read from sensor
      pillDispenser.run();
      int sensorRead = analogRead(breakBeamPinR); //read sensor state

      //changes sensor state based on read
      if (sensorRead > 100) currState = 1; 
      else currState = 0; 

      //see if sensor is currently broken or unbroken
      if (currState && !lastState) {
        //broken -> state changed (+= 1).
        timer=0; 
        changeState++;
        } 
    
      if (!currState && lastState) {
        //unbroken
        changeState ++;
        }

      //if state changes twice -> one pill fell through
      if (changeState >= 2){
        changeState = 0;
        count++;
        Serial.println(count);
        currState = 0;
        latestTime = millis();
      }
      
      timer++;
      lastState = currState;
      }
  
      if (millis() - latestTime > 5000){
        dispensed = 1;
        pillDispenser.stop();
       }          
    }
    rotator.moveToCW(3500);
    return count;
  } 

