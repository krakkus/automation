void setup() {
  pinMode(A1, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(9600);
  Serial.println("Booted!");
}

bool enable = false;

void loop() {
  int sensorValue = analogRead(A3);
  int esp = digitalRead(A1);

  float voltage = sensorValue * 25 / 687;
  Serial.print("Voltage: ");
  Serial.println(voltage);

  if (voltage > 26) enable = true;
  if (voltage < 25.6) enable = false;

  if (esp == 1 && enable == true) {
    digitalWrite(13, 1);
  } else {
    digitalWrite(13, 0);
  }

  delay(1000);
}
