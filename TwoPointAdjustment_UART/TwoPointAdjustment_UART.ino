#include "AdjustmentUART.h"

extern I2C_eeprom ee;  //0x50, I2C_DEVICESIZE_24LC04
// Poti:
struct Poti {
  String name;        // Name of Poti
  const int PotiPin;  // Pin assigned to Poti
  float gain;         // gain of measurment characteristics
  float offset;       // offset of measurment characteristics
};

// left Poti:
Poti poti_left = {
  "left Poti",
  PotiLeftPin,
  0,
  0
};

// left Poti:
Poti poti_right = {
  "right Poti",
  PotiRightPin,
  0,
  0
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }

  ee.begin();
  if (!ee.isConnected()) {
    Serial.println("ERROR: Can't find eeprom\nstopped...");
    //while (1)
    // ;
  }
  Serial.println("ready");

  // Poti Left
  // y = mx + b
  poti_left.gain = ((float)(readFromEEPROM(AddPLP2Y) - readFromEEPROM(AddPLP1Y))) / ((float)(readFromEEPROM(AddPLP2X) - readFromEEPROM(AddPLP1X)));
  poti_left.offset = (float)readFromEEPROM(AddPLP1Y) - poti_left.gain * (float)readFromEEPROM(AddPLP1X);

  // Poti Right
  // y = mx + b
  poti_right.gain = ((float)(readFromEEPROM(AddPRP2Y) - readFromEEPROM(AddPRP1Y))) / ((float)(readFromEEPROM(AddPRP2X) - readFromEEPROM(AddPRP1X)));
  poti_right.offset = (float)readFromEEPROM(AddPRP1Y) - poti_right.gain * (float)readFromEEPROM(AddPRP1X);
}

void loop() {
  // put your main code here, to run repeatedly:
  new_message = false;

  readFromSerial();
  stateSelect();
  delay(3000);
  readPoti(poti_right);
  readPoti(poti_left);
}

void readPoti(Poti poti) {
  int NumberOfSamples = 100;
  // y = mx + b

  float z;
  z = poti.gain * getMeanAnalogVal(poti.PotiPin, NumberOfSamples) + poti.offset;
  Serial.print(poti.name);
  Serial.print("\t Höhe [mm]: \t");
  Serial.println(z);
}