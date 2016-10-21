#include <SSD1306.h>
#include <DSPI.h>
#include <Icon8.h>
#include <Wire.h>
#include <LowPower.h>

/*
   Device definitions from DataStation Code
   Note: not matching LoRa example devices
*/

/*

   PIN definitions:
   Bluetooth
   PIN_RD6 38
   LoRa:
   PIN_RD13        36


*/

const int BOARD_TEMPERATURE  = 1;
const int BOARD_LIGHT_SENSOR = 2;
const int AIR_QUALITY_CLICK  = 3;
const int BAROMETER_CLICK    = 4;
const int CO_CLICK           = 5;
const int FLAME_CLICK        = 6;
const int LPG_CLICK          = 7;
const int METHANE_CLICK      = 8;
const int PROXIMITY_2_CLICK  = 9;
const int WEATHER_CLICK      = 10;

const int reset = 70;
const int dc = 77;
const int cs = 71;

DSPI0 spi;
CLICK_OLED_B oled(spi, cs, dc, reset);

bool ble = false;
bool lora = false;

void setup() {
  pinMode(PIN_STAT, INPUT);

  oled.initializeDevice();
  oled.fillScreen(Color::Black);
  oled.setFont(Fonts::Default);
  Wire.begin();
  enableLightSensor();

  pinMode(PIN_RD7, INPUT);
  pinMode(PIN_RF0, INPUT);
  pinMode(PIN_RF1, INPUT);
  pinMode(PIN_RG0, INPUT);

  pinMode(PIN_RD6, OPEN);
  digitalWrite(PIN_RD6, HIGH);

  pinMode(PIN_RD13, OPEN);
  digitalWrite(PIN_RD13, HIGH);

  pinMode(PIN_RG14, OUTPUT);
  digitalWrite(PIN_RG14, LOW);

  Serial.begin(115200);
  delay(1000);
  /*
  Serial.println("setup: s to start");
  while (1)
  {
    if (Serial.available() > 0)
    {
      char cc = Serial.read();
      if (cc == 'h' || cc == '?') {
        Serial.println("Press 's' to start.");
      }
      if (cc == 's') {
        break;
      }
    }
  }
  
  Serial.println("start");
  */
  // Only needs to be done once to configure it really.
  initBLE();
  Serial.println("Setup: Completed");

}

void loop() {
  //Are items devices?
  static int item = 0;
  static int i0 = HIGH;
  static int i1 = HIGH;
  static int i2 = HIGH;
  static int i3 = HIGH;

  oled.setFont(Fonts::Icon8);

  digitalWrite(PIN_RG14, HIGH);

  if (digitalRead(PIN_RD7) != i0) {
    Serial.println("PIN_RD7) != i0");
    i0 = digitalRead(PIN_RD7);
    if (i0 == LOW) {
      if (ble) {
        disableBLE();
      } else {
        enableBLE();
      }
    }
  }

  if (digitalRead(PIN_RG0) != i3) {
    i3 = digitalRead(PIN_RG0);
    if (i3 == LOW) {
      if (lora) {
        disableLoRa();
      } else {
        enableLoRa();
      }
    }
  }

  if (lora) {
    oled.setCursor(0, 0);
    oled.print(">");
  } else {
    oled.setCursor(0, 0);
    oled.print(" ");
  }

  if (ble) {
    oled.setCursor(32, 0);
    oled.print("&");
  } else {
    oled.setCursor(32, 0);
    oled.print(" ");
  }

  if (digitalRead(PIN_STAT) == LOW) {
    oled.setCursor(88, 0);
    if (U1OTGSTATbits.VBUSVD) {
      oled.print("1");
    } else {
      oled.print("2");
    }
  } else {
    oled.setCursor(88, 0);
    oled.print("-");
  }


  if (U1OTGSTATbits.SESVD) {
    oled.setCursor(16, 0);
    oled.print("'");
  } else {
    oled.setCursor(16, 0);
    oled.print(" ");
  }
  oled.setFont(Fonts::Default);
  oled.setCursor(0, 12);

  oled.fillRectangle(0, 8, 96, 31, Color::Black);

  //Just added the numbering scheme to the Serial1 output.
  switch (item) {
    case 0:
      oled.println("Battery:");
      oled.printf("%4.2fV", getBatteryVoltage());
      Serial1.printf("0:%4.2fV", getBatteryVoltage());
      break;
    case 1:
      oled.println("Temperature:");
      oled.printf("%5.2fC", getTemperature());
      Serial1.printf("1:% 5.2fC", getTemperature());
      break;
    case 2:
      oled.println("Light: ");
      oled.printf(" % d lux", getLightLevel());
      Serial1.printf("2: % d lux", getLightLevel());
      break;
    case 3:
      oled.println("IR: ");
      oled.printf(" % d lux", getIRLightLevel());
      Serial1.printf("3: % d lux", getIRLightLevel());
      break;
  }

  item++;
  if (item == 4) item = 0;

  digitalWrite(PIN_RG14, LOW);

  LowPower.snooze(2000);
}


float getBatteryVoltage() {
  pinMode(PIN_SENSEL, OUTPUT);
  digitalWrite(PIN_SENSEL, LOW);
  delay(50);
  float vs = analogRead(PIN_VSENSE);
  float volts = (vs / 1023.0) * 3.24;
  digitalWrite(PIN_SENSEL, HIGH);
  pinMode(PIN_SENSEL, INPUT);
  return volts * 3.0;
}

void enableLightSensor() {
  delay(100);
  Wire.beginTransmission(0x29);
  Wire.write(0x80);
  Wire.write(0x01);
  Wire.endTransmission();
}

void disableLightSensor() {
  delay(100);
  Wire.beginTransmission(0x29);
  Wire.write(0x80);
  Wire.write(0x00);
  Wire.endTransmission();
}

uint16_t getLightLevel() {
  Wire.beginTransmission(0x29);
  Wire.write(0x88);
  Wire.endTransmission(false);
  Wire.requestFrom(0x29, 4);
  uint16_t ch1 = Wire.read();
  ch1 |= (Wire.read() << 8);
  uint16_t ch0 = Wire.read();
  ch0 |= (Wire.read() << 8);
  return ch0;
}

uint16_t getIRLightLevel() {
  Wire.beginTransmission(0x29);
  Wire.write(0x88);
  Wire.endTransmission(false);
  Wire.requestFrom(0x29, 4);
  uint16_t ch1 = Wire.read();
  ch1 |= (Wire.read() << 8);
  uint16_t ch0 = Wire.read();
  ch0 |= (Wire.read() << 8);
  return ch1;
}

float getTemperature() {
  return ((analogRead(9) / 1023.0 * 3.3) - 0.5) * 100.0;
}

void initBLE() {
  enableBLE();
  delay(110);
  Serial1.print("$$$"); //get command prompt
  delay(110);
  report(Serial1);
  /*
  Serial.println("D\r"); //Dump
  Serial1.print("D\r"); //Dump
  delay(500);
  report(Serial1);
*/

  Serial.println("SF,1\r"); //factory reset
  Serial1.print("SF,1\r"); //factory reset
  delay(1000);
  report(Serial1);

  Serial1.print("$$$"); //get command prompt
  delay(500);
  report(Serial1);

  Serial.println("SS,C0\r");
  Serial1.print("SS,C0\r");
  delay(500);
  report(Serial1);
/*
  Serial.println("SN,DNLoRaShield\r"); //set bluetooth name
  Serial1.print("SN,DNLoRaShield\r"); //set bluetooth name

  delay(500);
  report(Serial1);
*/

  Serial.println("S-,LoRa\r"); //set bluetooth name
  Serial1.print("S-,LoRa\r"); //set bluetooth name
  delay(500);
  report(Serial1);


  Serial.println("SR,00A0\r"); //A0 0x2000 + 0x8000
  Serial1.print("SR,00A0\r"); //A0 0x2000 + 0x8000
  delay(500);
  report(Serial1);
/*
  Serial.println("D\r"); //Dump
  Serial1.print("D\r"); //Dump
  delay(500);
  report(Serial1);
*/

  Serial.println("R,1\r");
  Serial1.print("R,1\r");
  report(Serial1);
  delay(500);

  //disableBLE();
}

void enableLoRa() {
  digitalWrite(PIN_RD13, LOW);
  Serial0.begin(57600);
  lora = true;
}

void disableLoRa() {
  Serial0.end();
  digitalWrite(PIN_RD13, HIGH);
  lora = false;
}

void enableBLE() {
  digitalWrite(PIN_RD6, LOW);
  Serial1.begin(115200);
  ble = true;
}

void disableBLE() {
  Serial1.end();
  digitalWrite(PIN_RD6, HIGH);
  ble = false;
}

void report(Stream &d) {
  while (d.available()) Serial.write(d.read());
}
