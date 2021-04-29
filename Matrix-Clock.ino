#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <TM1640.h>
#include <TM16xxMatrixGFX.h>
#include <ezTime.h>

//--My libraries
#include <Charconvert.h>
#include "MatrixFonts.h"
#include "Credentials.h"

TM1640 module(23, 22);    // For ESP8266/WeMos D1-mini: DIN=D7/13/MOSI, CLK=D5/14/SCK
TM1640 module2(23, 21);
TM16xx * modules[]={&module,&module2};  

#define MYTIMEZONE "Australia/Brisbane"

#define MATRIX_NUMCOLUMNS 16
#define MATRIX_NUMROWS 8
TM16xxMatrixGFX matrix(modules, MATRIX_NUMCOLUMNS, MATRIX_NUMROWS, 2, 1); 

//--Wifi credentials from Credentials.h
const char *ssid = SID;
const char *password = PW;

Timezone myTZ;

String lastDisplayedTime = "";
String lastDisplayedAmPm = "";

const int OFFSET_M0 = 26;
const int OFFSET_M1 = 19;
const int OFFSET_H0 = 9;
const int OFFSET_H1 = 2;

bool transitionFlag = false;
bool isDots = true; 

bool fattyFont = false;

int m0 = 0;
int m1 = 0;
int h0 = 0;
int h1 = 0;
int secs;
int millisecs;
int myMinutes;
long interval = 1000;  
long previousMillis = 0;


void setup() {
  Serial.begin(115200);

  //--Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  //--Set WiFi to station mode and disconnect from an AP if it was Previously
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //--setup matrix panels
  matrix.setIntensity(7); // Use a value between 0 and 7 for brightness
  matrix.setRotation(1);
  matrix.setMirror(true);   // set X-mirror true when using the WeMOS D1 mini Matrix LED Shield (X0=Seg1/R1, Y0=GRD1/C1)
  
  matrix.fillScreen(LOW);
  matrix.write();

  //--Setup EZ Time
  setDebug(INFO);
  waitForSync();

  Serial.println();
  Serial.println("UTC:             " + UTC.dateTime());

  myTZ.setLocation(F(MYTIMEZONE));
  Serial.print(F("Time in your set timezone:         "));
  Serial.println(myTZ.dateTime());
}


void clearDigit(int index) {
  int xOffset;
  //--Get which digit to clear
  switch (index) {
    case 0: 
      xOffset = OFFSET_M0;
      break;
    case 1: 
      xOffset = OFFSET_M1;
      break;
    case 2: 
      xOffset = OFFSET_H0;
      break;
    case 3: 
      xOffset = OFFSET_H1;
      break;    
  }
  //--clear the digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+xOffset, y, LOW);
    }
  }
}

void writeDigit(int index, int digit) {
  int xOffset;
  //--Get which digit to clear
  switch (index) {
    case 0: 
      xOffset = OFFSET_M0;
      break;
    case 1: 
      xOffset = OFFSET_M1;
      break;
    case 2: 
      xOffset = OFFSET_H0;
      break;
    case 3: 
      xOffset = OFFSET_H1;
      break;    
  }
  //write first mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if (fattyFont) {
        if(fatFont[digit][y][x]) {
          matrix.drawPixel(x+xOffset, y, HIGH);
        }
      }
      else {
        if(skinnyFont[digit][y][x]) {
          matrix.drawPixel(x+xOffset, y, HIGH);     
        }
      }
    }
  }
}


void transition(int index, int digit) {
  int offScreen = -8;
  int xOffset;
  int nextDig = 0;
  //--Get which digit to clear
  switch (index) {
  case 0: 
    xOffset = OFFSET_M0;
    if (digit == 9) { nextDig = 0; }
    else nextDig = digit + 1;
    break;
  case 1: 
    xOffset = OFFSET_M1;
    if (digit == 5) { nextDig = 0; }
    else nextDig = digit + 1;   
    break;
  case 2: 
    xOffset = OFFSET_H0;
    if (digit == 5) { nextDig = 0; }
    else nextDig = digit + 1;   
    break;
  case 3: 
    xOffset = OFFSET_H1;
    if (digit == 2) { nextDig = 0; }
    else nextDig = digit + 1;   
    break;    
  }
  
  for (int i=0; i < 16; i++) {
    //--Clear the screen for the next write
    clearDigit(index);

    //--Start displaying the digit offscreen
    //--Increment by one until the last write
    //--This will sample falling numbers
    if (offScreen<0) { offScreen++; }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        if(fattyFont) {
          if(fatFont[nextDig][y][x]) {
            matrix.drawPixel(x+xOffset, y+offScreen, HIGH);
          }
        }
        else {
          if(skinnyFont[nextDig][y][x]) {
            matrix.drawPixel(x+xOffset, y+offScreen, HIGH);
          }  
        }
      }
    }
    delay(30);
    matrix.write();
  }
}


void updateTime() {
  String timeString = "";
  String AmPmString = "";
  bool PM = isPM();
  timeString = myTZ.dateTime("hisv");
  m0 = charToInt(timeString[3]);
  m1 = charToInt(timeString[2]);
  h0 = charToInt(timeString[1]);
  h1 = charToInt(timeString[0]);
  secs = myTZ.dateTime("s").toInt();
  millisecs = myTZ.dateTime("v").toInt();
  myMinutes = myTZ.dateTime("i").toInt();

  if (h0 == 6 && myTZ.isAM()) { matrix.setIntensity(5); }
  if (h0 == 8 && myTZ.isPM()) { matrix.setIntensity(0); }  
   
  /*// Only update Time if its different
  if (lastDisplayedTime != timeString) {
    //Serial.println(m0);
    //Serial.println(m1);
    //Serial.println(timeString);
    //ial.println(myMinutes);
       
    lastDisplayedTime = timeString;
  }*/
  
  //--clear then write first mins digit
  clearDigit(0);  
  writeDigit(0, m0);

  //--clear then write clear second min digit
  clearDigit(1);  
  writeDigit(1, m1);

  //--clear then write hr digit
  clearDigit(2);
  writeDigit(2, h0);

  //--clear then write 2nd hr digit
  clearDigit(3);
  writeDigit(3, h1);
}

void skinnydotdots() {
  //clear dot dots
  if (!isDots) {  
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 1; x++) {      
        matrix.drawPixel(x+16, y, LOW);
      }
    }
  }

  if (isDots) {
    //--dots
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 1; x++) {      
        if(dots[y][x]) { 
          matrix.drawPixel(x+16, y, HIGH);
        }
      }
    }
  }
}


void dotdots() {
  //clear dot dots
  if (!isDots) {  
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 2; x++) {      
        matrix.drawPixel(x+16, y, LOW);
      }
    }
  }

  if (isDots) {
    //--dots
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 2; x++) {      
        if(dots[y][x]) { 
          matrix.drawPixel(x+16, y, HIGH);
        }
      }
    }
  }
}


void loop() {
  //--update the time 
  //--check and use the correct fontdots
  updateTime(); 

  if (fattyFont) { dotdots(); }
  else skinnydotdots();
  
  //Serial.println(s);
  //--set the animation transition flag before the time changes
  if (secs==59 && millisecs==999) { transitionFlag=true; transition(0, m0); }
  if (m0==9 && secs==59 && millisecs==999) { transitionFlag=true; transition(1, m1); }
  if (myMinutes==59 && secs==59 && millisecs==999) { transitionFlag=true; transition(2, h0); }
  if (h0==9 && myMinutes==59 && secs==59 && millisecs==999) { transitionFlag=true; transition(3, h1); }
  
  unsigned long currentMillis = millis();
  
  //--change dot flag
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;
    if (isDots) { isDots = false; }
    else isDots = true;
  }

  //--finally write to the matrix
  matrix.write();
}
