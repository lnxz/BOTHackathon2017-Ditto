#include <Wire.h>
#include <UnoWiFiDevEd.h>
#include "rgb_lcd.h"
#include <SoftwareSerial.h>
#include <Servo.h>


#define LEDgreen 7
#define LEDred 6
#define LEDblue 5
#define FAN 4
Servo window;
Servo shade;

//int servoState = 0;

rgb_lcd lcd;
struct PairString;

struct PairString {
  String first;
  String second;
};

PairString getNextString(const String DELIM, String s) {
  int idx = s.indexOf(DELIM);
  //Serial.println(s);
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
  window.attach(9);
  shade.attach(8);

}

void loop() {
  //readRFID("rest", "172.16.2.172", "GET");
  pollFan("rest", "172.16.2.172", "/api/aircon/1", "GET");
  pollBrightness("rest", "172.16.2.172", "/api/lights/1", "GET");
  pollServo("rest", "172.16.2.172", "/api/servo", "GET");
  pollLCD("rest", "172.16.2.172", "/api/lcd", "GET");
}

void pollServo(const char* conn, const char* server, const char* command, const char* method) {
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    String currStatus = String(data.get(2));
    //Serial.println("Servo status: " + currStatus + " ");
    if (currStatus == "OFF") {
    // if (servoState) {
        shade.write(5);
        window.write(80);
       // servoState = 0;
     // }
    } else {
     // if (!servoState) {
        shade.write(175);
        window.write(10);
       // servoState = 1;
      //}
    }
  }
}
void pollFan(const char* conn, const char* server, const char* command, const char* method) {

  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    String currStatus = String(data.get(2));

    if (currStatus == "OFF") {
      digitalWrite(FAN, LOW);
    } else {
      digitalWrite(FAN, HIGH);
    }
  }
}

void pollLCD(const char* conn, const char* server, const char* command, const char* method) {
  //delay(1000);
  String DELIM = "$";
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    String lcdData = String(data.get(2));

    PairString lcdText = getNextString("$", lcdData);
    PairString lcdR = getNextString("$", lcdText.second);
    PairString lcdG = getNextString("$", lcdR.second);
    PairString lcdB = getNextString("$", lcdG.second);
    lcdData = lcdText.first;

    lcd.setRGB(lcdR.first.toInt(), lcdG.first.toInt(), lcdB.first.toInt());

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
 // delay(100);
}

void pollBrightness(const char* conn, const char* server, const char* command, const char* method) {

  String ledType;
  CiaoData data = Ciao.write(conn, server, command, method);
  if (!data.isEmpty()) {
    int brightness = String( data.get(2) ).toInt();
    analogWrite(LEDgreen, brightness);
    analogWrite(LEDred, brightness);
    analogWrite(LEDblue, brightness);
  }
}


