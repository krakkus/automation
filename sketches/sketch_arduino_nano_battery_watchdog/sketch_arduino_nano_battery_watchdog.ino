void setup() {
  pinMode(A1, INPUT);
  pinMode(A3, INPUT);

  Serial.begin(9600);
  Serial.println("Booted!");
}

int sample_1_x = 0;
int sample_1_y = 0;

int sample_2_x = 687;
int sample_2_y = 25000;

bool enable = false;

void loop() {
  int sensorValue = analogRead(A3);
  int esp = digitalRead(A1);

  int voltage = map(sensorValue,
                    sample_1_x,
                    sample_2_x,
                    sample_1_y,
                    sample_2_y);

  Serial.print("milivolts ");
  Serial.print(voltage);
  Serial.print(" enable ");
  Serial.print(enable);
  Serial.print(" esp ");
  Serial.println(esp);

  if (voltage > 26000) enable = true;
  if (voltage < 24000) enable = false;

  if (esp == 1 && enable == true) {
    //digitalWrite(13, 1);
  } else {
    //digitalWrite(13, 0);
  }

  delay(1000);
}
