/*
 This is an example of how to drive a 7 segment LED display from an ATmega without the use of current limiting resistors.
 This technique is very common but requires some knowledge of electronics - you do run the risk of dumping too 
 much current through the segments and burning out parts of the display. If you use the stock code you should be ok, but 
 be careful editing the brightness values.
 
 This code should work with all colors (red, blue, yellow, green) but the brightness will vary from one color to the next
 because the forward voltage drop of each color is different. This code was written and calibrated for the red color.

 This code will work with most Arduinos but you may want to re-route some of the pins.

 7 segments
 4 digits
 1 colon
 =
 12 pins required for full control 
 
 */

#define SEGMENT_ON  LOW
#define SEGMENT_OFF HIGH

int digit1 = 11; //PWM Display pin 1
int digit2 = 10; //PWM Display pin 2
int digit3 = 9; //PWM Display pin 6
int digit4 = 6; //PWM Display pin 8

int segA = A1; //Display pin 14
int segB = 3; //Display pin 16
int segC = 4; //Display pin 13
int segD = 5; //Display pin 3
int segE = A0; //Display pin 5
int segF = 7; //Display pin 11
int segG = 8; //Display pin 15
int segDP = 2; //Display pin 7

int state;

volatile int res_value;
volatile int res_mult;

void setup() {                
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);
  pinMode(segDP, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  pinMode(digit4, OUTPUT);
  
  pinMode(13, OUTPUT);
  
  // Configure Timer0 to display digits every 10ms (100Hz)
  // more info here: http://www.instructables.com/id/Arduino-Timer-Interrupts/
  cli();//stop interrupts
  //set timer0 interrupt at 100Hz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 100hz increments
  OCR0A = 155;// = (16*10^6) / (100*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();//allow interrupts
  
  // states for the state machine
  // more info here: http://hackaday.io/post/2774
  #define CALCULATE    1
  #define DISPLAY      2
  state = 1;
  
}

ISR(TIMER0_COMPA_vect){//timer0 interrupt 100Hz 
  // toggle displayFlag
  state = DISPLAY;
}

void loop() {
  switch(state){
    case CALCULATE:
      //
      res_value = 277;
      res_mult = 2; // multiplier on the form 10^x
    break;
    case DISPLAY:
      //
      displayNumber(res_value, res_mult);
    break;  
    default:
    break;
  }
  
  
}

//Display brightness
//Each digit is on for a certain amount of microseconds
//Then it is off until we have reached a total of 10ms for the function call
//Let's assume each digit is on for 1000us
//If each digit is on for 1ms, there are 4 digits, so the display is off for 16ms.
//That's a ratio of 1ms to 16ms or 6.25% on time (PWM).
//Let's define a variable called brightness that varies from:
//5000 blindingly bright (15.7mA current draw per digit)
//2000 shockingly bright (11.4mA current draw per digit)
//1000 pretty bright (5.9mA)
//500 normal (3mA)
//200 dim but readable (1.4mA)
//50 dim but readable (0.56mA)
//5 dim but readable (0.31mA)
//1 dim but readable in dark (0.28mA)

void displayNumber(int res_value, int res_mult) {
  #define DISPLAY_BRIGHTNESS  500
  #define DIGIT_ON  HIGH
  #define DIGIT_OFF  LOW
  long beginTime = millis();
  
  //Correct the value to be displayed (those will have a decimal)
  switch(res_mult) {
    case 1:
      res_value *= 10;
      break;
    case 4:
      res_value *= 10;
      break;
  }

  //Turn off every digit, then turn on with assigned value
  for(int digit = 4 ; digit > 0 ; digit--) {
    lightNothing();
    //Turn on a digit for a short amount of time
    switch(digit) {
    case 1:
      digitalWrite(digit1, DIGIT_ON);
      break;
    case 2:
      digitalWrite(digit2, DIGIT_ON);
      break;
    case 3:
      digitalWrite(digit3, DIGIT_ON);
      break;
    case 4:
      digitalWrite(digit4, DIGIT_ON);
      break;
    }

    //Display the measure (Ohm, KOhm, or MOhm), and the DecimalPoint
    switch(res_mult) {
    case 0:
      if (digit == 4) lightOhm();
      break;
    case 1:
      if (digit == 4) lightOhm();
      break;
    case 2:
      if (digit == 4) lightK();
      if (digit == 2) lightDP();
      break;
    case 3: 
      if (digit == 4) lightK();
      break;
    case 4:
      if (digit == 4) lightK();
      break;
    case 5:
      if (digit == 4) lightM();
      if (digit == 2) lightDP();
      break;
    }
    
    //Display every digit (3 digits) of the res_value  
    if (digit != 4){
      if (digit == 1 && res_value % 10 == 0) {
        lightNothing();
      } else {
        lightNumber(res_value % 10);
      }
      res_value /= 10;
    }    

    delayMicroseconds(DISPLAY_BRIGHTNESS); //Display this digit for a fraction of a second (between 1us and 5000us, 500 is pretty good)

    //Turn off all segments
    lightNothing(); 

    //Turn off all digits
    digitalWrite(digit1, DIGIT_OFF);
    digitalWrite(digit2, DIGIT_OFF);
    digitalWrite(digit3, DIGIT_OFF);
    digitalWrite(digit4, DIGIT_OFF);
  }
  state = CALCULATE;
}


void lightDP() {
  digitalWrite(segDP, SEGMENT_ON);
}

void lightNothing() {
  digitalWrite(segA, SEGMENT_OFF);
  digitalWrite(segB, SEGMENT_OFF);
  digitalWrite(segC, SEGMENT_OFF);
  digitalWrite(segD, SEGMENT_OFF);
  digitalWrite(segE, SEGMENT_OFF);
  digitalWrite(segF, SEGMENT_OFF);
  digitalWrite(segG, SEGMENT_OFF);
  digitalWrite(segDP, SEGMENT_OFF);
}

void lightIdle() {
  digitalWrite(segA, SEGMENT_OFF);
  digitalWrite(segB, SEGMENT_OFF);
  digitalWrite(segC, SEGMENT_OFF);
  digitalWrite(segD, SEGMENT_OFF);
  digitalWrite(segE, SEGMENT_OFF);
  digitalWrite(segF, SEGMENT_OFF);
  digitalWrite(segG, SEGMENT_ON);
  digitalWrite(segDP, SEGMENT_OFF);
}

void lightOhm() {
  digitalWrite(segA, SEGMENT_OFF);
  digitalWrite(segB, SEGMENT_OFF);
  digitalWrite(segC, SEGMENT_ON);
  digitalWrite(segD, SEGMENT_OFF);
  digitalWrite(segE, SEGMENT_ON);
  digitalWrite(segF, SEGMENT_OFF);
  digitalWrite(segG, SEGMENT_ON);
  digitalWrite(segDP, SEGMENT_OFF);
}

void lightK() {
  digitalWrite(segA, SEGMENT_OFF);
  digitalWrite(segB, SEGMENT_ON);
  digitalWrite(segC, SEGMENT_ON);
  digitalWrite(segD, SEGMENT_OFF);
  digitalWrite(segE, SEGMENT_ON);
  digitalWrite(segF, SEGMENT_ON);
  digitalWrite(segG, SEGMENT_ON);
  digitalWrite(segDP, SEGMENT_OFF);
}

void lightM() {
  digitalWrite(segA, SEGMENT_ON);
  digitalWrite(segB, SEGMENT_ON);
  digitalWrite(segC, SEGMENT_ON);
  digitalWrite(segD, SEGMENT_OFF);
  digitalWrite(segE, SEGMENT_ON);
  digitalWrite(segF, SEGMENT_ON);
  digitalWrite(segG, SEGMENT_OFF);
  digitalWrite(segDP, SEGMENT_OFF);
}

//Given a number, turns on those segments
void lightNumber(int numberToDisplay) {

  switch (numberToDisplay){

  case 0:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 1:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 2:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_OFF);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 3:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 4:
    digitalWrite(segA, SEGMENT_OFF);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 5:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 6:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_OFF);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 7:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_OFF);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_OFF);
    digitalWrite(segG, SEGMENT_OFF);
    break;

  case 8:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_ON);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;

  case 9:
    digitalWrite(segA, SEGMENT_ON);
    digitalWrite(segB, SEGMENT_ON);
    digitalWrite(segC, SEGMENT_ON);
    digitalWrite(segD, SEGMENT_ON);
    digitalWrite(segE, SEGMENT_OFF);
    digitalWrite(segF, SEGMENT_ON);
    digitalWrite(segG, SEGMENT_ON);
    break;
  }

}
