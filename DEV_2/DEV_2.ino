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
const int MIXER_MOTOR_PIN = 3;
const int ASAM_MOTOR_PIN = 4;
const int BASA_MOTOR_PIN = 5;

const float pHSet = 7;
const float pHTolerance = 0.5;
const float referenceVoltage = 3.3; // 3.3 for ESP8266 and ESP32 | 5 for Arduino Uno
const int onTimeMotor = 500;  // Time for motor to supply liquid in Milisecond

// Calibration variable
// The value of PH4 and PH7 must be calibrated before use to get the precise value
const float PH4 = 3.29;
const float PH7 = 2.69;
const float PH10 = 2.20;  // Not really used
const float pHStep = (PH4 - PH7) / 3;

float pHVal = 0;
float pHVoltage = 0;
int encodedVal;
unsigned long int avgValue;
int buf[100], temp;

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

unsigned long int getAveragePHValue() {
  for (int i = 0; i < 100; i++) {
    buf[i] = analogRead(PH_METER_PIN);
    delay(10);
  }
  
  for (int i = 0; i < 100-1; i++) {
    for (int j = i + 1; j < 100; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  
  avgValue = 0;
  
  for (int i = 24; i < 24+50; i++) {
    avgValue += buf[i];
  }
  
  return avgValue /= 50;
}

void getPHValue() {
  encodedVal = getAveragePHValue();
  pHVoltage = encodedVal * (referenceVoltage / 1024);
  pHVal = 7 + ((PH7 - pHVoltage) / pHStep);
  Serial.print("ADC Val: ");
  Serial.print(encodedVal);
  Serial.print(" | PH Vol: ");
  Serial.print(pHVoltage);
  Serial.print(" | PH: ");
  Serial.print(pHVal);
  pHStatus = String("PH in tolerance range.");
  prevGetPHValue = millis();
}

bool isPHBelowTolerance() {
  if (pHVal < (pHSet - pHTolerance)) {
    Serial.print(" - (");
    Serial.print(pHSet - pHTolerance);
    Serial.print(") Below");
    return true;
  }
  
  return false;
}

bool isPHAboveTolerance() {
  if (pHVal > (pHSet + pHTolerance)) {
    Serial.print(" - (");
    Serial.print(pHSet + pHTolerance);
    Serial.print(") Above");
    return true;
  }
  
  return false;
}

void mixLiquid() {
  digitalWrite(MIXER_MOTOR_PIN, LOW);
  Serial.print(" - Mixing liquid");
  delay(5000);
  digitalWrite(MIXER_MOTOR_PIN, HIGH);
}

void addAsamLiquid() {
  Serial.print(" - Asam Motor ON");
  pHStatus = String("Adding acid liquid...");
  digitalWrite(ASAM_MOTOR_PIN, LOW);
  delay(onTimeMotor);
  digitalWrite(ASAM_MOTOR_PIN, HIGH);
  mixLiquid();
}

void addBasaLiquid() {
  Serial.print(" - Basa Motor ON");
  pHStatus = String("Adding base liquid...");
  digitalWrite(BASA_MOTOR_PIN, LOW);
  delay(onTimeMotor);
  digitalWrite(BASA_MOTOR_PIN, HIGH);
  mixLiquid();
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

  pHStatus = String("Adding base liquid...");  
  prevSentDataTime = millis();
}

// Setup
void setup() {
  Serial.begin(9600);
  
  pinMode(MIXER_MOTOR_PIN, OUTPUT);
  pinMode(ASAM_MOTOR_PIN, OUTPUT);
  pinMode(BASA_MOTOR_PIN, OUTPUT);
  digitalWrite(MIXER_MOTOR_PIN, HIGH); 
  digitalWrite(ASAM_MOTOR_PIN, HIGH); 
  digitalWrite(BASA_MOTOR_PIN, HIGH);
  
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

// Loop
void loop() {
  startTime = millis();

  setupWifi();

  if ((startTime - prevSentDataTime) >= 15000) {
    getPHValue(); 
    
    if (isPHBelowTolerance()) {
      // Liquid is Acid
      addBasaLiquid();
    } else if (isPHAboveTolerance()) {
      // Liquid is Base
      addAsamLiquid();
    }
    
    Serial.println();
    
    sendDataToThingspeak();
  }
}
