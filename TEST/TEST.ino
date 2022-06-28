int pHValue;
float voltage;

void setup() {
//  Serial.begin(115200);
  Serial.begin(9600);
}

void loop() {
  pHValue = analogRead(A0);
  voltage = pHValue * (3.3 / 1023.0);
  Serial.print(pHValue);
  Serial.print(" - ");
  Serial.println(voltage);
  delay(500);
}
