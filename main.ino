/*
** Author: Emirhan Gocturk
** Description: Arduino UNO based digital safe project.
** Version: V1.0
*/

#include <Servo.h>

//Global constants for pin numbers
#define outputB 11
#define outputA 12
#define rotaryButton 13

const int ledRed = A0;
const int ledGreen = A1;
int latchPin = 2;//Pin connected to ST_CP of 74HC595
int clockPin = 3;//Pin connected to SH_CP of 74HC595 
int dataPin = 4; //Pin connected to DS of 74HC595 


int counter = 0; 
int number;
int currentState;
int previousState;  

const char common = 'a';    // common anode

int btn = 1;
int count = 0;
int savedCode[3] = {1, 1, 1};
int inputCode[3] = {0};
Servo servo1;
int servo_position = 100;
const int buzzer = 9;

const int pushButton = A5;
int buttonState = 0;

 
 /*
 ** Author: Emirhan Gocturk
 ** Arguments: NONE
 ** Description: As you spin the rotary it will get values dependingon its output
 **              however, this is better than directly shifting the bits since it
 **              allows you to smooth it as you want. I do every 2 clicks.
 **              Will return a final integer value named "val.
 */

 int rotarySpinner()
 {
  //this function makes spin smoother than just bit-shifting
  int val;
   currentState = digitalRead(outputA); 
   //If current state and previous state don't match
   if (currentState != previousState){ 
        
     // If the current state is different than output B knob is being rotated clockwise
     if (digitalRead(outputB) != currentState) 
     { 
       counter ++;
     } else 
     {
       counter --;
     }

     Serial.print("Position: ");
     
     //every 2 spiner click will increment number by one
     //also the other way around!
     if(counter >= 50)
     {
      val = 9;
      counter = 50;
     }
     else if(counter >= 18)
     {
      val = 9; 
     }
     else if(counter >= 16)
     {
      val = 8;
     }
     else if(counter >= 14)
     {
      val = 7;
     }
     else if(counter >= 12)
     {
      val = 6;
     }
     else if(counter >= 10)
     {
      val = 5;
     }
     else if(counter >= 8)
     {
      val = 4;
     }
     else if(counter >= 6)
     {
      val = 3;
     }
     else if(counter >= 4)
     {
      val = 2;
     }
     else if(counter >= 2)
     {
      val = 1;
     }
     else if(counter >= 0)
     {
      val = 0;
     }
     else if(counter < 0)
     {
      val = 0;
      counter = 0;
     }
     
     Serial.println(val);
   } 
   
   //Update previous state with current state value
   previousState = currentState;
   
   return val;
 }

 /*
 ** Author: Emirhan Gocturk
 ** Arguments: int someNumber
 ** Description: Instead of bitshifting, after smoothing the rotary
 **              encoder, variable "val" form rotarySpinner function
 **              is assigned to "someNumber" and a case is chosen to
 **              get the bits in order to light-up certain LEDs.
 */
byte myfnNumToBits(int someNumber) 
{
  //cases represent numbers, self-explanatory
  switch (someNumber) {
    case 0:
      return B11111100;
      break;
    case 1:
      return B01100000;
      break;
    case 2:
      return B11011010;
      break;
    case 3:
      return B11110010;
      break;
    case 4:
      return B01100110;
      break;
    case 5:
      return B10110110;
      break;
    case 6:
      return B10111110;
      break;
    case 7:
      return B11100000;
      break;
    case 8:
      return B11111110;
      break;
    case 9:
      return B11110110;
      break;
    case 10:
      return B11101110; // Hexidecimal A
      break;
    case 11:
      return B00111110; // Hexidecimal B
      break;
    case 12:
      return B10011100; // Hexidecimal C or use for Centigrade
      break;
    case 13:
      return B01111010; // Hexidecimal D
      break;
    case 14:
      return B10011110; // Hexidecimal E
      break;
    case 15:
      return B10001110; // Hexidecimal F or use for Fahrenheit
      break;  
    default:
      return B10010010; // Error condition, displays three vertical bars
      break;   
  }
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments: byte eightBits
 ** Description: Updates the display.
 */
 void myfnUpdateDisplay(byte eightBits) 
 {
    if (common == 'a') 
    {                  // using a common anonde display?
      eightBits = eightBits ^ B11111111;  // then flip all bits using XOR 
    }
    digitalWrite(latchPin, LOW);  // prepare shift register for data
    shiftOut(dataPin, clockPin, LSBFIRST, eightBits); // send data
    digitalWrite(latchPin, HIGH); // update display
 }

 /*
 ** Author: Emirhan Gocturk
 ** Arguments: NONE
 ** Description: All three functions packed into one single function
 **              in order to make the code readable.
 */
 void updateDisplay()
 {
    //get the rotary position as long as the button isn't pressed down
    number = rotarySpinner();
    byte bits = myfnNumToBits(number) ;
    myfnUpdateDisplay(bits);    // display alphanumeric digit
 }

 /*
 ** Author: Emirhan Gocturk
 ** Arguments: NONE
 ** Description: Checks given passcode to saved admin passcode.
 */
void checkPassCode()
{
 for(int index = 0; index < 3; index++)
  {
    //compare each number to saved passcode
    if(inputCode[index] == savedCode[index])
    {
      //if each step is correct count will be 3
      count++;
    }
  } 
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments: NONE
 ** Description: Rotates the servo motor to open the lock.
 */
void openLock()
{
  //if count is three open the safe
    btn = 1;
    digitalWrite(ledGreen, HIGH);
    
    for(servo_position = 100; servo_position >= 0; servo_position-=5)
    {
      servo1.write(servo_position);
      delay(10);
    }
    
    tone(buzzer,1000,100);
    delay(2000);
    digitalWrite(ledGreen, LOW);
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Rotates servo motor back to its initial position
 **              to lock the safe.
 */
void closeLock()
{

    //when the button is pressed safe will lock back
    for(servo_position = 0; servo_position <= 100; servo_position+=5)
    {
      servo1.write(servo_position);
      delay(10);
    }
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Waits until user input and selects if user wants to
 **              assign a new passcode or just lock the safe.
 */
void selectOrContinue()
{
  count = 0;
  while(btn != 0)
  {
    //get the button state
    buttonState = digitalRead(pushButton);
    
    //if button is pressed and count is 0
    if((buttonState == 1) && (count == 0))
    {
      tone(buzzer,660,100);
      delay(200);
      //reset to initial 7 segment display
      latchPin = 2;//Pin connected to ST_CP of 74HC595
      clockPin = 3;//Pin connected to SH_CP of 74HC595 
      dataPin = 4; //Pin connected to DS of 74HC595 
      
      //get the input into input array
      getInput();

      //transfer input code to saved code array
      for(count = 0; count < 3; count++)
      {
        savedCode[count] = inputCode[count];
      }
      count = 3;
    }
    
    //while button is not pressed wait for an input
    btn = digitalRead(rotaryButton);
  }
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Plays Mario soundtrack (short) as an alarm.
 */
void playMario()
{
   //if the passcode is false, mario alarm will play. Red LED will also blink until alarm ends
    digitalWrite(ledRed, HIGH);
      tone(buzzer,660,100);
      delay(150);
      tone(buzzer,660,100);
      delay(300);
      tone(buzzer,660,100);
      delay(300);
      tone(buzzer,510,100);
      delay(100);
      tone(buzzer,660,100);
      delay(300);
      tone(buzzer,770,100);
      delay(550);
      tone(buzzer,380,100);
      delay(575);
    delay(5000);
    digitalWrite(ledRed, LOW);
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Reset all 7-Segment displays.
 */
void reset()
{
  count = 0;
  latchPin = 2;//Pin connected to ST_CP of 74HC595
  clockPin = 3;//Pin connected to SH_CP of 74HC595 
  dataPin = 4; //Pin connected to DS of 74HC595 
  byte bits = myfnNumToBits(0);
  myfnUpdateDisplay(bits);
  tone(buzzer,1000,100);
  delay(200);
  latchPin = 5;//Pin connected to ST_CP of 74HC595
  clockPin = 6;//Pin connected to SH_CP of 74HC595 
  dataPin = 7; //Pin connected to DS of 74HC595 
  myfnUpdateDisplay(bits);
  tone(buzzer,800,100);
  delay(200);
  latchPin = A2;//Pin connected to ST_CP of 74HC595
  clockPin = A3;//Pin connected to SH_CP of 74HC595 
  dataPin = A4; //Pin connected to DS of 74HC595 
  myfnUpdateDisplay(bits);
  tone(buzzer,660,100);
  delay(200);
  latchPin = 2;//Pin connected to ST_CP of 74HC595
  clockPin = 3;//Pin connected to SH_CP of 74HC595 
  dataPin = 4; //Pin connected to DS of 74HC595 
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Setup second display to manipulate it.
 */
void setupSecondDisplay()
{
  inputCode[0] = number;
  latchPin = 5;//Pin connected to ST_CP of 74HC595
  clockPin = 6;//Pin connected to SH_CP of 74HC595 
  dataPin = 7; //Pin connected to DS of 74HC595 
  count = 1;
  delay(200);
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Setup third display to manipulate it.
 */
void setupThirdDisplay()
{
  inputCode[1] = number;
  latchPin = A2;//Pin connected to ST_CP of 74HC595
  clockPin = A3;//Pin connected to SH_CP of 74HC595 
  dataPin = A4; //Pin connected to DS of 74HC595 
  count = 2;
  delay(200);
}

 /*
 ** Author: Emirhan Gocturk
 ** Arguments:
 ** Description: Read the rotary encoder values and its button's
 */
void getInput()
{
   while(count != 3)
  {
    //get the button state
    btn = digitalRead(rotaryButton);
    
    //if the button is NOT pressed
   if(btn == 1)
   {
    //updates current 7 segment display
    updateDisplay();
   }
   else if(btn == 0)
   {
    //play an indicator sound when rotart button is pressed
    tone(buzzer,100,10);
    if(count == 0)
    {
      //if it is first press switch to the second 7 segment
      setupSecondDisplay();
      continue;
    }
    else if(count == 1)
    {
      //if it is the second press switch to the third 7 segment
      setupThirdDisplay();
      continue;
    }
    else if(count == 2)
    {
      //if it is the third press end the loop
      inputCode[2] = number;
      count = 3;
    }
   }
  }
}

 /*
 ** Author: Emirhan Gocturk
 ** Description: Arduino setup function
 */
void setup() 
{ 
  //Setting pins as INPUT_PULLUP since we want to read 1 only (high) instead of INPUT
   pinMode (outputA,INPUT_PULLUP);
   pinMode (outputB,INPUT_PULLUP);

   pinMode(latchPin,OUTPUT);
   pinMode(clockPin,OUTPUT);
   pinMode(dataPin,OUTPUT);
   pinMode(rotaryButton, INPUT_PULLUP);
   pinMode(ledRed, OUTPUT);
   pinMode(ledGreen, OUTPUT);

   //attach servo and setup initial position
   servo1.attach(8);
   servo1.write(servo_position);

  //setup buzzer
   pinMode(buzzer, OUTPUT);

  //setup pins for 2nd 7 segment display
   pinMode(5, OUTPUT);
   pinMode(6, OUTPUT);
   pinMode(7, OUTPUT);
  //setup pins for 3rd 7 segment display
   pinMode(A2, OUTPUT);
   pinMode(A3, OUTPUT);
   pinMode(A4, OUTPUT);

   //setup push button
   pinMode(pushButton, INPUT);
   //Inititate the serial console for maintenance
   Serial.begin (9600);
   
   ///Get the initial state of output A from the rotary encoder, assign it to previousState
   previousState = digitalRead(outputA);  
} 

 /*
 ** Author: Emirhan Gocturk
 ** Description: Arduino loop function
 */
void loop() 
{ 
  //reset everything else to default state
  reset();
  //Get the current state of output pin A and assign the value to currentState
  getInput();
  
  count = 0;
  //check if passcode is correct
  checkPassCode();

  if(count == 3)
  {
    //open the lock if pass code was correct
    openLock();
    
    //wait until rotary button is pressed unless push button is pressed
    selectOrContinue();

    //close the lock if the rotary button is pressed
    closeLock();
  }
  else
  {
    //if the passcode is false, mario alarm will play. Red LED will also blink until alarm ends
    playMario();
  }
}
