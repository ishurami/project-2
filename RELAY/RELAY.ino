const int RELAY1_PIN = 4;
const int RELAY2_PIN = 5;

unsigned long start_time = 0;

void setup() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  start_time = millis();
  digitalWrite(RELAY1_PIN, LOW);  
  digitalWrite(RELAY2_PIN, LOW);     
  delay(5000);                      
  digitalWrite(RELAY1_PIN, HIGH); 
  digitalWrite(RELAY2_PIN, HIGH);  
  delay(3000);                      
}
