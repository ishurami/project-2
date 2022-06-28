#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

#define SECRET_SSID "skematology"    // replace MySSID with your WiFi network name
#define SECRET_PASS "skematology"  // replace MyPassword with your WiFi password

#define SECRET_CH_ID 1762213     // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "V23I9QGHDSD7BKN3"   // replace XYZ with your channel write API Key

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

WiFiClient  client;

#define PH_METER_PIN A0
const int ASAM_MOTOR_PIN = 4;
const int BASA_MOTOR_PIN = 5;

const float pHSet = 7;
const float pHTolerance = 0.5;
const float referenceVoltage = 3.3;
const int onTimeMotor = 500;  // Milisecond

// Calibration variable
const float PH4 = 3.1;
const float PH7 = 2.6;
const float pHStep = (PH4 - PH7) / (7 - 4);

float pHVal = 0;
float pHVoltage = 0;
int a;

unsigned long startTime = 0;
unsigned long prevGetPHValue = 0;
unsigned long prevSentDataTime = 0;

String pHStatus = "";

void setupWifi() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
}

void getPHValue() {
  a = analogRead(PH_METER_PIN);
  pHVoltage = a * (referenceVoltage / 1023.0);
  pHVal = 7 + ((PH7 - pHVoltage) / pHStep);
  Serial.print(a);
  Serial.print(" - ");
  Serial.print(pHVoltage);
  Serial.print(" - ");
  Serial.print(pHVal);
//  pHStatus = String("PH in tolerance range: " + pHVal);
  pHStatus = String("PH in tolerance range.");
  prevGetPHValue = millis();
}

bool isPHBelowTolerance() {
//  if (pHVal < (pHSet - pHTolerance)) return true;
  if (pHVoltage < 2.70) {
    Serial.print(" - Below");
    return true;
  }
  return false;
}

bool isPHAboveTolerance() {
//  if (pHVal > (pHSet - pHTolerance)) return true;
  if (pHVoltage > 2.90) {
    Serial.print(" - Above");
    return true;
  }
  return false;
}

void addAsamLiquid() {
  Serial.print(" - Asam Motor ON");
  pHStatus = String("Adding acid liquid...");
  digitalWrite(ASAM_MOTOR_PIN, LOW);
  delay(onTimeMotor);
  digitalWrite(ASAM_MOTOR_PIN, HIGH);
}

void addBasaLiquid() {
  Serial.print(" - Basa Motor ON");
  pHStatus = String("Adding base liquid...");
  digitalWrite(BASA_MOTOR_PIN, LOW);
  delay(onTimeMotor);
  digitalWrite(BASA_MOTOR_PIN, HIGH);
}


void sendDataToThingspeak() {
  ThingSpeak.setField(1, pHVal);
  ThingSpeak.setStatus(pHStatus);

  // write to the ThingSpeak channel
  int statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(statusCode == 200){
    Serial.println("Channel update successful.");
  } else{
    Serial.println("Problem updating channel. HTTP error code " + String(statusCode));
  }
  
  prevSentDataTime = millis();
}

// Setup
void setup() {
  Serial.begin(9600);
  
  pinMode(ASAM_MOTOR_PIN, OUTPUT);
  pinMode(BASA_MOTOR_PIN, OUTPUT);
  digitalWrite(ASAM_MOTOR_PIN, HIGH); 
  digitalWrite(BASA_MOTOR_PIN, HIGH);
  
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

// Loop
void loop() {
  startTime = millis();

  setupWifi();

  if ((startTime - prevGetPHValue) >= 5000) {
    getPHValue(); 

    if (isPHBelowTolerance()) {
      addAsamLiquid();
    } else if (isPHAboveTolerance()) {
      addBasaLiquid();
    }
    
    Serial.println();
  }
//
  if ((startTime - prevSentDataTime) >= 15000) {
    sendDataToThingspeak();
  }
//  
//  Serial.println();
}
