#include <Wire.h>
#include <UnoWiFiDevEd.h>
#include "rgb_lcd.h"
#include <SoftwareSerial.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Servo.h>

#define LEDgreen 7
#define LEDred 6
#define LEDblue 5
#define FAN 4

Servo window;
Servo shade;

//const values
const int closedWindow = 10;
const int openWindow = 80;
const int closedShade = 5;
const int openShade = 175;

const char* connector = "rest";
const char* server  = "172.16.2.172";
const char* method = "GET";

int id;
rgb_lcd lcd;
struct PairString;
//PN532_I2C pn532_i2c(Wire);
//NfcAdapter nfc = NfcAdapter(pn532_i2c);
struct PairString {
  String first;
  String second;
};

PairString getNextString(const String DELIM, String s) {
  int idx = s.indexOf(DELIM);
  Serial.println(s);
  String rtn = s.substring(0, idx);
  PairString ps;
  ps.first = rtn;
  ps.second = s.substring(idx + 1);
  return ps;
}

void setup() {

  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDred, OUTPUT);
  pinMode(LEDblue, OUTPUT);
  pinMode(FAN, OUTPUT);
  Serial.begin(9600);
  Ciao.begin();
  pinMode(2, INPUT);
  lcd.begin(16, 2);
  //nfc.begin();
  window.attach(9);
  shade.attach(8);
 
}

/*
void readRFID(const char* conn, const char* server, const char* method) {
  Serial.println("\nScan a NFC tag\n");
  if (nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    String tagID = tag.getUidString();
    //tag.print();
    Serial.print("UID: "); Serial.println(tagID);
    if (tagID == "EA 14 54 62") {
      //if(tag.getUidString() == "EA 14"){
      Serial.print("WELCOME SHAUN");
    } else if (tagID == "0A B5 54 62") {
      Serial.print("WELCOME KEN");
    }
  }
}
*/

void loop() {
  //shade.write(openShade);
  //window.write(openWindow);
  delay(100);
  //readRFID(connector, server, method);
  delay(1000);
  pollFan(connector, server, "/api/aircon/1", method);
  pollBrightness(connector, server, "/api/lights/1", method, LEDred);
  pollBrightness(connector, server, "/api/lights/2", method, LEDgreen);
  pollBrightness(connector, server, "/api/lights/3", method, LEDblue);
  pollLCD(connector, server, "/api/lcd", method);
  shade.write(closedShade);
  window.write(closedWindow);
}

void pollFan(const char* conn, const char* server, const char* command, const char* method) {

  Serial.println("Setting Air-con");
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    String currStatus = String(data.get(2));
    Serial.print("Fan status: " + currStatus + " ");
    if (currStatus == "OFF") {
      digitalWrite(FAN, LOW);
    } else {
      digitalWrite(FAN, HIGH);
    }
  }  else {
    Ciao.println ("Write Error");
    Serial.println ("Write Error");
  }
}

void pollLCD(const char* conn, const char* server, const char* command, const char* method) {
  delay(1000);
  Serial.println("Setting LCD");
  String DELIM = "$";
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    String lcdData = String(data.get(2));
    Serial.println("LCD:" + lcdData);

    PairString lcdText = getNextString("$", lcdData);
    PairString lcdR = getNextString("$", lcdText.second);
    PairString lcdG = getNextString("$", lcdR.second);
    PairString lcdB = getNextString("$", lcdG.second);
    lcdData = lcdText.first;
    Serial.println("text: " + lcdText.first);
     Serial.println("R: " + String(lcdR.first) + " G: " + String(lcdG.first) + " B: " + String(lcdB.first));
     Serial.println(lcdR.first.toInt());
    lcd.setRGB(lcdR.first.toInt(), lcdG.first.toInt(), lcdB.first.toInt());
//    lcd.setRGB(255, 255, 255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(lcdData);
    if (lcdData.length() > 16) {
      lcd.setCursor(0, 1);
      String secondHalf = lcdData.substring(16, lcdData.length());
      lcd.print(secondHalf);
    }
  }
  else {
    Ciao.println ("Write Error");
    Serial.println ("Write Error");
  }
  delay(100);
}

void pollBrightness(const char* conn, const char* server, const char* command, const char* method, int LED) {
  Serial.println("Polling light brightness status");
  String ledType;
  if (LED == LEDblue) {
    ledType = "BLUE";
  } else if (LED == LEDred) {
    ledType = "RED";
  } else {
    ledType = "GREEN";
  }
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    int brightness = String( data.get(2) ).toInt();
    Serial.println("Brightness of " + String(ledType) + " : " + String(brightness));
    analogWrite(LED, brightness);
  }  else {
    Ciao.println ("Write Error");
    Serial.println ("Write Error");
  }
}


