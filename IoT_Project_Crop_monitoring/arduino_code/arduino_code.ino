#include <SoftwareSerial.h>

// Pin Configurations

// Arduino Part
#define tempSensor A0
#define moistureSensor A2
#define lightSensor A1

SoftwareSerial espSerial(2, 3);

String str;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  espSerial.begin(9600);

  // Pin INPUT/OUTPUT configurations
  pinMode(tempSensor, INPUT);
  pinMode(moistureSensor, INPUT);
  pinMode(lightSensor, INPUT);

}

float temperatureReading() {
  float voltage = analogRead(tempSensor);
  float temp = voltage * (5000 / 1024.0);
  return temp / 10;
}

float drynessReading() {
  float dryness = analogRead(moistureSensor);
  dryness = map(dryness, 0, 1024, 0, 100);
  return dryness;
}

float lightReading() {
  float light = analogRead(lightSensor);
  light = map(light, 0, 1024, 0, 100);
  return light;
}

void loop() {
  // put your main code here, to run repeatedly:
  float tempVal = temperatureReading();

  float drynessVal = drynessReading();

  float lightVal = lightReading();

  str = str + "Temp: " + String(tempVal) + " \xC2\xB0 C |" + " Dryness: " + String(drynessVal) + " % | Darkness: " + String(lightVal) + " %";

  Serial.println(str);

  espSerial.println(str);

  delay(10000);

  str = "";
}
