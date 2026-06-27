#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RF_RX_PIN     PA3
#define RELAY_PIN     PB0

#define RF_BAUD       2000
#define PACKET_HEADER 0xAA

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool lastHelmet   = false;
bool lastAlcohol  = true;
bool lastRelay    = false;

uint8_t rfReadByte() {
  uint8_t data = 0;
  for (int i = 7; i >= 0; i--) {
    delayMicroseconds(1000000 / RF_BAUD);
    if (digitalRead(RF_RX_PIN) == HIGH) {
      data |= (1 << i);
    }
  }
  return data;
}

bool waitForHeader(unsigned long timeoutMs) {
  unsigned long start = millis();
  while ((millis() - start) < timeoutMs) {
    if (digitalRead(RF_RX_PIN) == HIGH) {
      uint8_t received = rfReadByte();
      if (received == PACKET_HEADER) return true;
    }
  }
  return false;
}

void updateLCD(bool helmet, bool alcohol, bool ignition) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Helmet:");
  lcd.print(helmet ? "YES" : "NO ");
  lcd.print(" Alc:");
  lcd.print(alcohol ? "Y" : "N");

  lcd.setCursor(0, 1);
  lcd.print("Ignition: ");
  lcd.print(ignition ? "ON " : "OFF");
}

void setRelay(bool state) {
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(RF_RX_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Smart Helmet");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");
  delay(1500);
  lcd.clear();

  Serial.println("Smart Helmet Receiver Ready");
}

void loop() {
  if (!waitForHeader(300)) return;

  delayMicroseconds(500);
  uint8_t payload    = rfReadByte();
  delayMicroseconds(500);
  uint8_t payloadInv = rfReadByte();

  if ((payload ^ payloadInv) != 0xFF) {
    Serial.println("Packet error — checksum mismatch, ignoring");
    return;
  }

  bool helmetWorn      = (payload >> 1) & 0x01;
  bool alcoholDetected = (payload >> 0) & 0x01;
  bool ignitionON      = helmetWorn && !alcoholDetected;

  if (helmetWorn != lastHelmet || alcoholDetected != lastAlcohol || ignitionON != lastRelay) {
    setRelay(ignitionON);
    updateLCD(helmetWorn, alcoholDetected, ignitionON);

    lastHelmet  = helmetWorn;
    lastAlcohol = alcoholDetected;
    lastRelay   = ignitionON;

    Serial.print("Helmet: ");   Serial.print(helmetWorn ? "YES" : "NO");
    Serial.print(" | Alcohol: "); Serial.print(alcoholDetected ? "YES" : "NO");
    Serial.print(" | Ignition: "); Serial.println(ignitionON ? "ON" : "OFF");
  }
}
