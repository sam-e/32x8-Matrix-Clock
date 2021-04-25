#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <TM1640.h>
#include <TM16xxMatrixGFX.h>
#include <ezTime.h>

//--My libraries
#include <Charconvert.h>
#include <MatrixFonts.h>

TM1640 module(23, 22);    // For ESP8266/WeMos D1-mini: DIN=D7/13/MOSI, CLK=D5/14/SCK
TM1640 module2(23, 21);
TM16xx * modules[]={&module,&module2};  

#define MYTIMEZONE "Australia/Brisbane"

#define MATRIX_NUMCOLUMNS 16
#define MATRIX_NUMROWS 8
TM16xxMatrixGFX matrix(modules, MATRIX_NUMCOLUMNS, MATRIX_NUMROWS, 2, 1); 
const char* ssid = "";
const char* password = "";

Timezone myTZ;

String lastDisplayedTime = "";
String lastDisplayedAmPm = "";

const int OFFSET_M0 = 26;
const int OFFSET_M1 = 19;
const int OFFSET_H0 = 9;
const int OFFSET_H1 = 2;

bool isDots = true; 


/*//test code
  int m0 = 0;
  int m1 = 0;
  int h0 = 0;
  int h1 = 0;
//end test code
*/



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
  clearMONE();
  
  //write first mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(fatFont[m0][y][x]) {
        matrix.drawPixel(x+OFFSET_M0, y, HIGH);
      }
    }
  }

  //clear second min digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_M1,y, LOW);
    }
  }
  //write second mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(fatFont[m1][y][x]) {
        matrix.drawPixel(x+OFFSET_M1, y, HIGH);
      }
    }
  }

  //clear first hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_H1,y, LOW);
    }
  }
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
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_H0,y, LOW);
    }
  }
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

void clearMONE() {
    //clear first mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_M0,y, LOW);
    }
  }
}

void transONE() {
  //write first mins digit
  for (int t = 0; t < 4; t++) {
    clearMONE();
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        if(trans01[t][y][x]) {
          matrix.drawPixel(x+OFFSET_M0, y, HIGH);
        }
      }
    }
    matrix.write(); 
    delay(1000);
  }
}

void transTWO() {
  //write first mins digit
  for (int t = 0; t < 4; t++) {
    clearMONE();
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 6; x++) {
        if(trans02[t][y][x]) {
          matrix.drawPixel(x+OFFSET_M0, y, HIGH);
        }
      }
    }
    matrix.write(); 
    delay(1000);
  }
}

void setSkinnyFontTime() {
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
  clearMONE();
  //if (m0==0) { transONE(); }
  //if (m0==1) { transTWO(); }
  
  //write first mins digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(skinnyFont[m0][y][x]) {
        matrix.drawPixel(x+OFFSET_M0, y, HIGH);
      }
    }
  }

  //clear second min digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_M1,y, LOW);
    }
  }
  //write second mins digit
   for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(skinnyFont[m1][y][x]) {
        matrix.drawPixel(x+OFFSET_M1, y, HIGH);
      }
    }
  }

  //clear first hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_H1,y, LOW);
    }
  }
  //write first hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if (h1 != 0) {
        if(skinnyFont[h1][y][x]) {
          matrix.drawPixel(x+OFFSET_H1, y, HIGH);
        }
      }
    }
  }

  //clear 2nd hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      matrix.drawPixel(x+OFFSET_H0,y, LOW);
    }
  }
  //write 2nd hr digit
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 6; x++) {
      if(skinnyFont[h0][y][x]) {
        matrix.drawPixel(x+OFFSET_H0, y, HIGH);
      }
    }
  }
}

void skinnydotdots() {
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
  //setFatFontTime();
  setSkinnyFontTime();
  dotdots();

  if (isDots) { isDots = false; }
  else isDots = true;
  matrix.write();
  delay(1000);
  
  //if (m0==0) { transONE(); }
  //if (m0==1) { transTWO(); }
  //if (m0==9) { m0=0; }
  //else; m0++;
}
