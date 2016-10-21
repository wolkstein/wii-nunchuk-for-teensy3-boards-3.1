#include <Wire.h>
#include "Nunchuk.h"

// Nunchuck for input
IntervalTimer NuchukTimer;
Nunchuk nc = Nunchuk();

// vars for left right beatdetection 
unsigned long XACC_LEFT_TIME = 0;
unsigned long XACC_RIGHT_TIME = 0;
unsigned int LEFTCOUNT = 0;
unsigned int RIGHTCOUNT = 0;

/*
 *  Nunchuk Data.
 *  Can not use a struct here because volatile vars.
 *  nunchuk read runns in a Timer all 10 ms.
 *  all datatyps are the same than in Nunchuk.h
 */

volatile float NcAccel;
volatile int NcAccelX;
volatile int NcAccelY;
volatile int NcAccelZ;
volatile bool NcBtnZ;
volatile bool NcBtnC;
volatile int NcJoyX;
volatile int NcJoyY;
volatile float NcTiltX;
volatile float NcTiltY;
volatile float NcTiltZ;

int NcJoyX_offset = 0; // offset for joystick x achsis
int NcJoyY_offset = 0; // offset for joystick yS achsis


int mouseDead = 5; // set between min of 0 and max of 10
int mylookupZeroOffset= 126 + mouseDead +1;
int mouseLookupX[300];
int mouseLookupY[300];


/// activate nuchuk emulation
bool IS_MOUSE = true;
bool IS_KEY = true;
bool IS_JOY = false;

float SLOW_MOUSE_MOVE_X = 0.0;
float SLOW_MOUSE_MOVE_Y = 0.0;


/// --------------SETUP-------------
void setup()
{  
  // Initialize the serial port
  //Serial.begin(115200);
  Serial.begin(57600);
  
  // Initialize the Nunchuk.
  nc.begin();
  NuchukTimer.begin(readNuchuk, 10000);// all 10ms

  // Calibrate Joy X and Joy Y
  elapsedMillis waiting;     // "waiting" starts at zero
  while (waiting < 2000) {
    NcJoyX_offset = runningAverageX(NcJoyX);
    NcJoyY_offset = runningAverageY(NcJoyY);
  }
  if(NcBtnZ){
    IS_MOUSE = false;
    IS_KEY = false;
    IS_JOY = true;
    }
   
  NcJoyX_offset *= -1;
  NcJoyY_offset *= -1;
  Serial.print("offset X: ");
  Serial.print(NcJoyX_offset);
  Serial.print("offset Y: ");
  Serial.println(NcJoyY_offset);

  //create lookuptables
  // negative -127 -> -1
  
  for( int i = 0; i <= 299; i++){
     mouseLookupX[i] = 128;
     mouseLookupY[i] = 128;   
  }
  
  int val = -127;
  for( int i = 0; i <= 126; i++){
     mouseLookupX[i] = val;
     mouseLookupY[i] = val;
     //Serial.println( mouseLookupX[i] );
     val +=1;
  }
  // add deadPoint XY
  for( int i = 127; i <= 126 + (mouseDead*2) +1; i++){
     mouseLookupX[i] = 0;
     mouseLookupY[i] = 0;
     //Serial.println( mouseLookupX[i] );
  }
  // add positive
  int posval = 1;
  for( int i = 127 + (mouseDead*2) +1; i <= 255 + (mouseDead*2); i++){
     mouseLookupX[i] = posval;
     mouseLookupY[i] = posval;
     //Serial.println( mouseLookupX[i] );
     posval +=1;
  }

 /*
  for( int i = 0; i <= 280; i++){
    Serial.print("i: ");
    Serial.print(i);
    Serial.print(", val: ");
    Serial.println( mouseLookupX[i]); 
  }
  Serial.print("my zero in lookup table:");
  Serial.println(mylookupZeroOffset);
  delay(300*1000);
  */
}


/// -------Nunchuk Timer Function all 10ms---
void readNuchuk(){
  
  nc.read();
  
  if( nc.isOk() ) {
    NcAccel = nc.getAccel();
    NcAccelX = nc.getAccelX();
    NcAccelY = nc.getAccelY();
    NcAccelZ = nc.getAccelZ();
    NcBtnZ = nc.getButtonZ();
    NcBtnC = nc.getButtonC();
    NcJoyX = nc.getJoyX();
    NcJoyY = nc.getJoyY();
    NcTiltX = nc.getTiltX();
    NcTiltY = nc.getTiltY();
    NcTiltZ = nc.getTiltZ();       
  }
}


/// ----------Main Loop------------
void loop()
{
/// uncomment next line to see all nunchuck inputs on serial console
  //serial_print_all_nunchuk_inputs();



/// EXAMPLES for KEY, JOYSTICK and MOUSE emulation

/// KEY example using beatdetection to move for example a slideshow with KEY_LEFT or KEY_RIGT

  if(IS_KEY){
    // detect left right hits
    int hitdirection = simpleLeftRightHitDetection();
    // we fire some keyboard keys
    if(hitdirection == -1){
      doKeyboardRightKey(); // reversed, but better feeling on image slideshow
    }
    if(hitdirection == 1){
      doKeyboardLeftKey(); // reversed, but better feeling on image slideshow
    }
  }

/// JOY Example for 4 achsis and two btn joystick
  if(IS_JOY){
    Joystick.X(map(NcJoyX + NcJoyX_offset,-105,105,0,1023));
    Joystick.Y(map(NcJoyY + NcJoyY_offset,-105,105,1023,0));
    Joystick.Z(map(NcTiltX,-90,90,0,1023));
    Joystick.Zrotate(map(NcTiltY,-90,90,0,1023));
    Joystick.button(1, NcBtnZ);
    Joystick.button(2, NcBtnC);
  }


/// MOUSE Example for two a button Mouse while Nunchuck Joystick is used for Mouse Mouvements
  if(IS_MOUSE){

    // add mouse deadzone
    int deadX = mouseLookupX[NcJoyX + NcJoyX_offset + mylookupZeroOffset];
    int deadY = mouseLookupY[NcJoyY + NcJoyY_offset + mylookupZeroOffset];   
    // map the mouse to the maximus
    float mouseX = map(deadX,-105,105,-100,100);
    float mouseY = map(deadY,-105,105,100,-100);   
    // devide mouse speed
    float parsed_mouseX = mouseX / 5.0; // 25.0 on lenovo laptop, 5 on raspberry pi
    float parsed_mouseY = mouseY / 5.0; // 25.0 on lenovo laptop, 5 on raspberry pi

    // create slow move for x if x <1
    if((SLOW_MOUSE_MOVE_X >= -1.0 && SLOW_MOUSE_MOVE_X <= 1.0) && (parsed_mouseX > -1.0 && parsed_mouseX < 1.0)){
      SLOW_MOUSE_MOVE_X += parsed_mouseX;
      if (SLOW_MOUSE_MOVE_X >= 1.00){
        Mouse.move(SLOW_MOUSE_MOVE_X, parsed_mouseY);
      }
      if (SLOW_MOUSE_MOVE_X <= -1.00){
         Mouse.move(SLOW_MOUSE_MOVE_X, parsed_mouseY);
      }
    }
    else{
      SLOW_MOUSE_MOVE_X = 0.0;
    }

    if(parsed_mouseX == 0.0){
      SLOW_MOUSE_MOVE_X = 0.0;
    }

    // create slow move for y if y < 1
    if((SLOW_MOUSE_MOVE_Y >= -1.0 && SLOW_MOUSE_MOVE_Y <= 1.0) && (parsed_mouseY > -1.0 && parsed_mouseY < 1.0)){
      SLOW_MOUSE_MOVE_Y += parsed_mouseY;
      if (SLOW_MOUSE_MOVE_Y >= 1.00){
        Mouse.move(int(parsed_mouseX), int(SLOW_MOUSE_MOVE_Y));
      }
      if (SLOW_MOUSE_MOVE_Y <= -1.00){
         Mouse.move(int(parsed_mouseX), int(SLOW_MOUSE_MOVE_Y));
      }
    }
    else{
      SLOW_MOUSE_MOVE_Y = 0.0;
    }

    if(parsed_mouseY == 0.0){
      SLOW_MOUSE_MOVE_Y = 0.0;
    } 

/*
    Serial.print("dead X: ");    
    Serial.print(deadX);
    Serial.print(" dead Y: ");    
    Serial.print(deadY);
      
    Serial.print("X: ");    
    Serial.print(mouseX);
    Serial.print(" X parsed: ");
    Serial.print(int(parsed_mouseX));
    Serial.print(" Y: ");    
    Serial.print(mouseY);
    Serial.print(" Y parsed: ");
    Serial.print(parsed_mouseY);   
    Serial.print(" X slow: ");
    Serial.print(SLOW_MOUSE_MOVE_X);
    Serial.print(" Y slow: ");
    Serial.println(SLOW_MOUSE_MOVE_Y);
*/ 
    // create move for x y if bigger than 1      
    Mouse.move(int(parsed_mouseX), int(parsed_mouseY));
    Mouse.set_buttons(NcBtnZ, 0, NcBtnC);
  }
/// Delay everything a little bit.
  delay(5);
}

// -----------Functions----------

void doKeyboardLeftKey(){
  Keyboard.set_key1(KEY_LEFT);
  Keyboard.send_now();
  Keyboard.set_key1(0);
  Keyboard.send_now(); 
}

void doKeyboardRightKey(){
  Keyboard.set_key1(KEY_RIGHT);
  Keyboard.send_now();
  Keyboard.set_key1(0);
  Keyboard.send_now();  
}


/// simple Average filter
long runningAverageX(int M) {
  #define LM_SIZE 20
  static int LM[LM_SIZE];      // LastMeasurements
  static byte index = 0;
  static long sum = 0;
  static byte count = 0;

  // keep sum updated to improve speed.
  sum -= LM[index];
  LM[index] = M;
  sum += LM[index];
  index++;
  index = index % LM_SIZE;
  if (count < LM_SIZE) count++;

  return sum / count;
}

long runningAverageY(int M) {
  #define LM_SIZE 20
  static int LM[LM_SIZE];      // LastMeasurements
  static byte index = 0;
  static long sum = 0;
  static byte count = 0;

  // keep sum updated to improve speed.
  sum -= LM[index];
  LM[index] = M;
  sum += LM[index];
  index++;
  index = index % LM_SIZE;
  if (count < LM_SIZE) count++;

  return sum / count;
}


// simple detection of x axis left and right beats
// return -1 = left, 0 = nothing, 1 = right
int simpleLeftRightHitDetection(){
  int hit = 0;
  int averangeaccx = runningAverageX(NcAccelX);

  if ( NcAccelX - averangeaccx > 300) {
    RIGHTCOUNT +=1;
    if( XACC_RIGHT_TIME == 0 ){
      XACC_RIGHT_TIME = millis();
    }
  }
  
  if (NcAccelX - averangeaccx < -300) {
    LEFTCOUNT += 1;
    if( XACC_LEFT_TIME == 0 ){
      XACC_LEFT_TIME = millis();
    }
  }

  if (XACC_RIGHT_TIME != 0 && millis() - XACC_RIGHT_TIME > 100 ){
    XACC_RIGHT_TIME = 0; 
    //Serial.print("RIGHTCOUNT: ");
    //Serial.println(RIGHTCOUNT);
    if(RIGHTCOUNT > LEFTCOUNT){
      hit = 1;
      Serial.println("beat to right side  -->");
    }    
    RIGHTCOUNT = 0;
  }

  if (XACC_LEFT_TIME != 0 && millis() - XACC_LEFT_TIME > 100 ){
    //Serial.println("setze XACC_LEFT_TIME auf 0");
    XACC_LEFT_TIME = 0;
    //Serial.print("LEFTCOUNT: ");
    //Serial.println(LEFTCOUNT);
    if(LEFTCOUNT > RIGHTCOUNT){
      hit = -1;
      Serial.println("beat to left side  <--");
    }
    LEFTCOUNT = 0;
  }
  return hit; 
}

void serial_print_all_nunchuk_inputs(){
  
  Serial.print( NcAccel, DEC );
  Serial.print( "\t" );
  Serial.print( NcAccelX, DEC );
  Serial.print( "\t" );
  Serial.print( NcAccelY, DEC );
  Serial.print( "\t" );
  Serial.print( NcAccelZ, DEC );
  Serial.print( "\t" );

  Serial.print( NcBtnZ, DEC );
  Serial.print( "\t" );
  Serial.print( NcBtnC, DEC );
  Serial.print( "\t" );
  Serial.print( NcJoyX + NcJoyX_offset, DEC );
  Serial.print( "\t" );
  Serial.print( NcJoyY + NcJoyY_offset, DEC );
  Serial.print( "\t" );
  
  Serial.print( NcTiltX, DEC );
  Serial.print( "\t" );
  Serial.print( NcTiltY, DEC );
  Serial.print( "\t" );
  Serial.print( NcTiltZ, DEC );
  Serial.print( "\n" );
  }
