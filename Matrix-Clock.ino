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

bool transitionFlag1 = false;
bool transitionFlag2 = false;
bool transitionFlag3 = false;
bool transitionFlag4 = false;
bool isDots = true; 


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
      if(skinnyFont[digit][y][x]) {
        matrix.drawPixel(x+xOffset, y, HIGH);
      }
    }
  }
}

void setFatFontTime() {
  String timeString = "";
  String AmPmString = "";
  bool PM = isPM();
  timeString = myTZ.dateTime("hi");
  int m0 = charToInt(timeString[3]);
  int m1 = charToInt(timeString[2]);
  int h0 = charToInt(timeString[1]);
  int h1 = charToInt(timeString[0]);

  if (h0 == 6 && myTZ.isAM()) { matrix.setIntensity(5); }
  if (h0 == 8 && myTZ.isPM()) { matrix.setIntensity(0); }  
 
  // Only update Time if its different
  if (lastDisplayedTime != timeString) {
    Serial.println(m0);
    Serial.println(m1);
    Serial.println(h0);
    Serial.println(h1);
        
    lastDisplayedTime = timeString;
  }

  //clear first mins digit
  clearDigit(0);
  
  //write first mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(fatFont[m0][y][x]) {
        matrix.drawPixel(x+OFFSET_M0, y, HIGH);
      }
    }
  }

  //clear second min digit
  clearDigit(1);
  //write second mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(fatFont[m1][y][x]) {
        matrix.drawPixel(x+OFFSET_M1, y, HIGH);
      }
    }
  }

  //clear first hr digit
  clearDigit(2);
  //write first hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if (h1 != 0) {
        if(fatFont[h1][y][x]) {
          matrix.drawPixel(x+OFFSET_H1, y, HIGH);
        }
      }
    }
  }

  //clear 2nd hr digit
  clearDigit(3);
  //write 2nd hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(fatFont[h0][y][x]) {
        matrix.drawPixel(x+OFFSET_H0, y, HIGH);
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

void transitionOne() {
  for (int t = 0; t < 4; t++) {
    Serial.println("animating1");
    //Serial.println(m0);

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        switch (m0){
          case 0: 
            if(trans01[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 1: 
            if(trans02[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 2: 
            if(trans03[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 3: 
            if(trans04[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 4: 
            if(trans05[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 5: 
            if(trans06[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
           case 6: 
            if(trans07[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 7: 
            if(trans08[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
            break;
          case 8: 
            if(trans09[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            }
          case 9: 
            if(trans10[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M0, y, HIGH);
            } 
            break;                 
        }
      }
    }
    if (t==3) { transitionFlag1 = false; }
    matrix.write(); 
    if (t<3) { delay(150); }
  }
}

void transitionTwo() {
  for (int t = 0; t < 4; t++) {
    Serial.println("animating2");
    //Serial.println(m0);

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        switch (m1){
          case 0: 
            if(trans01[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 1: 
            if(trans02[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 2: 
            if(trans03[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 3: 
            if(trans04[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 4: 
            if(trans05[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 5: 
            if(trans06[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
           case 6: 
            if(trans07[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 7: 
            if(trans08[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
            break;
          case 8: 
            if(trans09[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            }
          case 9: 
            if(trans10[t][y][x]) {
              matrix.drawPixel(x+OFFSET_M1, y, HIGH);
            } 
            break;                 
        }
      }
    }
    if (t==3) { transitionFlag2 = false; }
    matrix.write(); 
    if (t<3) { delay(150); }
  }
}

void transitionThree() {
  for (int t = 0; t < 4; t++) {
    Serial.println("animating3");
    //Serial.println(m0);

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        switch (h0){
          case 0: 
            if(trans01[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 1: 
            if(trans02[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 2: 
            if(trans03[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 3: 
            if(trans04[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 4: 
            if(trans05[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 5: 
            if(trans06[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
           case 6: 
            if(trans07[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 7: 
            if(trans08[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
            break;
          case 8: 
            if(trans09[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            }
          case 9: 
            if(trans10[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H0, y, HIGH);
            } 
            break;                 
        }
      }
    }
    if (t==3) { transitionFlag3 = false; }
    matrix.write(); 
    if (t<3) { delay(150); }
  }
}

void transitionFour() {
  for (int t = 0; t < 4; t++) {
    Serial.println("animating4");
    //Serial.println(m0);

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        switch (h1){
          case 0: 
            if(trans01[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 1: 
            if(trans02[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 2: 
            if(trans03[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 3: 
            if(trans04[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 4: 
            if(trans05[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 5: 
            if(trans06[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
           case 6: 
            if(trans07[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 7: 
            if(trans08[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
            break;
          case 8: 
            if(trans09[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            }
          case 9: 
            if(trans10[t][y][x]) {
              matrix.drawPixel(x+OFFSET_H1, y, HIGH);
            } 
            break;                 
        }
      }
    }
    if (t==3) { transitionFlag4 = false; }
    matrix.write(); 
    if (t<3) { delay(150); }
  }
}

void setSkinnyFontTime() {
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
   
  // Only update Time if its different
  if (lastDisplayedTime != timeString) {
    //Serial.println(m0);
    //Serial.println(m1);
    //Serial.println(timeString);
    //ial.println(myMinutes);
       
    lastDisplayedTime = timeString;
  }
  
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


void loop() {
  //setFatFontTime();
  setSkinnyFontTime(); 
  skinnydotdots();

  //Serial.println(s);
  //--set the animation transition flag before the time changes
  if (secs==59 && millisecs==999) { transitionFlag1=true; transitionOne(); Serial.println("f1"); }
  if (m0==9 && secs==59 && millisecs==999) { transitionFlag2=true; transitionTwo(); Serial.println("f2"); }
  if (myMinutes==59 && secs==59 && millisecs==999) { transitionFlag3=true; transitionThree(); Serial.println("f3"); }
  if (h0==9 && myMinutes==59 && secs==59 && millisecs==999) { transitionFlag4=true; transitionFour(); Serial.println("f4"); }
  
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
