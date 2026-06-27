#include <Arduino.h>

#define IR_PIN        PA0
#define MQ3_PIN       PA1
#define RF_TX_PIN     PA2

#define MQ3_THRESHOLD    400
#define SAMPLE_INTERVAL  200
#define RF_BAUD          2000

#define PACKET_HEADER    0xAA

unsigned long lastSampleTime = 0;

void rfSendByte(uint8_t data) {
  for (int i = 7; i >= 0; i--) {
    bool bit = (data >> i) & 0x01;
    digitalWrite(RF_TX_PIN, bit ? HIGH : LOW);
    delayMicroseconds(1000000 / RF_BAUD);
  }
  digitalWrite(RF_TX_PIN, LOW);
}

void sendPacket(bool helmetWorn, bool alcoholDetected) {
  uint8_t payload = 0x00;
  if (helmetWorn)       payload |= (1 << 1);
  if (alcoholDetected)  payload |= (1 << 0);

  rfSendByte(PACKET_HEADER);
  delayMicroseconds(500);
  rfSendByte(payload);
  delayMicroseconds(500);
  rfSendByte(~payload);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(IR_PIN,    INPUT);
  pinMode(MQ3_PIN,   INPUT_ANALOG);
  pinMode(RF_TX_PIN, OUTPUT);
  digitalWrite(RF_TX_PIN, LOW);

  delay(2000);
  Serial.println("Smart Helmet Transmitter Ready");
  Serial.println("MQ3 warm-up complete");
}

void loop() {
  unsigned long now = millis();
  if ((now - lastSampleTime) < SAMPLE_INTERVAL) return;
  lastSampleTime = now;

  bool helmetWorn      = (digitalRead(IR_PIN) == LOW);
  int  mq3Value        = analogRead(MQ3_PIN);
  bool alcoholDetected = (mq3Value > MQ3_THRESHOLD);

  Serial.print("IR Raw: ");        Serial.print(digitalRead(IR_PIN));
  Serial.print(" | Helmet: ");     Serial.print(helmetWorn ? "YES" : "NO");
  Serial.print(" | MQ3 ADC: ");    Serial.print(mq3Value);
  Serial.print(" | Alcohol: ");    Serial.println(alcoholDetected ? "YES" : "NO");

  sendPacket(helmetWorn, alcoholDetected);
}
