#include <Arduino.h>
#include "Wire.h"
#include "I2C_eeprom.h"
#include "AdjustmentUART.h"

I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC04);
const uint16_t dataLength = 2;  // Datenlänge 2 byte

// Point
struct Point {
  String name;
  uint16_t X;
  uint16_t Y;
};

const int NumberOfSamples = 100;  // Samples of Analog Read values

// Parameter für Serielle Kommunikation
const unsigned int MAX_MESSAGE_LENGTH = 15;
static char message[MAX_MESSAGE_LENGTH];
boolean new_message;
static unsigned int message_pos = 0;

enum cmd_list {
  cmd_1,
  cmd_2,
  cmd_3,
  last_cmd
};

enum cmd_list number_of_cmds = last_cmd;

struct mapping {
  enum cmd_list cmd_entry;
  char cmd_name[MAX_MESSAGE_LENGTH];
};

// Nummerierung der Kommandos
mapping map_cmd[last_cmd + 1] = {
  { cmd_1, "APL" },  // Adjust Poti Left
  { cmd_2, "APR" },  // Adjust Poti Right
  { cmd_3, "GAI" },  // Get Adjustment Info: P1(X1/Y1), P2(X2/Y2), slope m, offset b
  { last_cmd, "Last_Command" }
};

//------------------------------------------------------------------------
//                              Funktionen
//------------------------------------------------------------------------
int findCmdIndex(char message[], int number_of_cmds);
int eingabePunkt(String Koordinate);
int readFromEEPROM(int memoryAddress);
int getMeanAnalogVal(int pin, int NumberOfSamples);
void write2EEPROM(int memoryAddress, uint16_t wData, uint16_t dataLength);
void readPoti();

void readFromSerial() {
  new_message = false;
  while ((Serial.available() > 0)) {
    // Read the next available byte in the serial receive buffer
    byte inByte = Serial.read();
    // wenn LineFeed LF (= 10 in Dezimal) ODER CarriageReturn CR (= 13 in Dezimal) ODER max Nachrichtenlänge erreicht
    // --> Nachricht beendet
    if (inByte == 10 || inByte == 13 || message_pos >= MAX_MESSAGE_LENGTH - 1) {
      // Add null character to string to characterize it as string
      message[message_pos] = '\0';
      message_pos = 0;
      new_message = true;
      //Serial.println(message);
    } else {
      // Nachricht aus Bytes zusammenbauen und jedes Byte (Dezimalwert von 0 -255) in char casten (siehe Ascii Tabelle)
      message[message_pos] = (char)inByte;
      message_pos++;
    }
  }
}

void stateSelect() {
  int cmd_index;
  if (new_message) {
    cmd_index = findCmdIndex(message, number_of_cmds);

    switch (cmd_index) {
      case 0:  // write Values to EEPROM
        {
          Serial.println("State: \t Adjust Poti left");

          Point PL1 = {
            "PL1",  // name
            0,      // X
            0       // Y
          };

          Point PL2 = {
            "PL2",  // name
            0,      // X
            0       // Y
          };

          PL1.Y = eingabePunkt("Y1");
          PL1.X = getMeanAnalogVal(PotiLeftPin, NumberOfSamples);

          Serial.print("P1 (X1/Y1): \t");
          Serial.print(PL1.X);
          Serial.print("/");
          Serial.println(PL1.Y);

          // Write X1 to EEPROM
          write2EEPROM(AddPLP1X, PL1.X, dataLength);
          // Write Y1 to EEPROM
          write2EEPROM(AddPLP1Y, PL1.Y, dataLength);

          PL2.Y = eingabePunkt("Y2");
          PL2.X = getMeanAnalogVal(PotiLeftPin, NumberOfSamples);

          Serial.print("P2 (X2/Y2): \t");
          Serial.print(PL2.X);
          Serial.print("/");
          Serial.println(PL2.Y);

          // Write X2 to EEPROM
          write2EEPROM(AddPLP2X, PL2.X, dataLength);
          // Write Y2 to EEPROM
          write2EEPROM(AddPLP2Y, PL2.Y, dataLength);
          break;
        }
      case 1:  // write Values to EEPROM
        {
          Serial.println("State: \t Adjust Poti right");

          Point PR1 = {
            "PR1",  // name
            0,      // X
            0       // Y
          };

          Point PR2 = {
            "PR2",  // name
            0,      // X
            0       // Y
          };

          PR1.Y = eingabePunkt("Y1");
          PR1.X = getMeanAnalogVal(PotiRightPin, NumberOfSamples);

          Serial.print("P1 (X1/Y1): \t");
          Serial.print(PR1.X);
          Serial.print("/");
          Serial.println(PR1.Y);

          // Write X1 to EEPROM
          write2EEPROM(AddPRP1X, PR1.X, dataLength);
          // Write Y1 to EEPROM
          write2EEPROM(AddPRP1Y, PR1.Y, dataLength);

          PR2.Y = eingabePunkt("Y2");
          PR2.X = getMeanAnalogVal(PotiRightPin, NumberOfSamples);

          Serial.print("P2 (X2/Y2): \t");
          Serial.print(PR2.X);
          Serial.print("/");
          Serial.println(PR2.Y);

          // Write X2 to EEPROM
          write2EEPROM(AddPRP2X, PR2.X, dataLength);
          // Write Y2 to EEPROM
          write2EEPROM(AddPRP2Y, PR2.Y, dataLength);
          break;
        }
      case 2:  // read Values from EEPROM
        {
          Serial.println();
          Serial.println("State: \t Get Adjustment Info");
          Serial.println();
          Serial.println("\t\t P1(X1/Y1)\t P1(X2/Y2)");

          Serial.println("Left Poti");

          Serial.print("Address: \t  (  ");
          Serial.print(AddPLP1X);
          Serial.print("/ ");
          Serial.print(AddPLP1Y);
          Serial.print(")\t");
          Serial.print("  (  ");
          Serial.print(AddPLP2X);
          Serial.print("/ ");
          Serial.print(AddPLP2Y);
          Serial.println(")");

          Serial.print("Values: \t  (");
          Serial.print(readFromEEPROM(AddPLP1X));
          Serial.print("/");
          Serial.print(readFromEEPROM(AddPLP1Y));
          Serial.print(") \t");
          Serial.print("  (");
          Serial.print(readFromEEPROM(AddPLP2X));
          Serial.print("/");
          Serial.print(readFromEEPROM(AddPLP2Y));
          Serial.println(")");

          // y = m*x+b
          float mL = ((float)(readFromEEPROM(AddPLP2Y) - readFromEEPROM(AddPLP1Y)) / (float)(readFromEEPROM(AddPLP2X) - readFromEEPROM(AddPLP1X)));  // m = (Y2-Y1)/(X2-X1)
          float bL = (float)readFromEEPROM(AddPLP1Y) - mL * (float)readFromEEPROM(AddPLP1X);                                                         // b = Y1-m*X1
          Serial.print("slope m: \t  ");
          Serial.println(mL);
          Serial.print("offset b: \t  ");
          Serial.println(bL);

          Serial.println("Right Poti");

          Serial.print("Address: \t  ( ");
          Serial.print(AddPRP1X);
          Serial.print("/ ");
          Serial.print(AddPRP1Y);
          Serial.print(")\t");
          Serial.print("  ( ");
          Serial.print(AddPRP2X);
          Serial.print("/");
          Serial.print(AddPRP2Y);
          Serial.println(")");

          Serial.print("Values: \t  (");
          Serial.print(readFromEEPROM(AddPRP1X));
          Serial.print("/");
          Serial.print(readFromEEPROM(AddPRP1Y));
          Serial.print(") \t");
          Serial.print("  (");
          Serial.print(readFromEEPROM(AddPRP2X));
          Serial.print("/");
          Serial.print(readFromEEPROM(AddPRP2Y));
          Serial.println(")");

          // y = m*x+b
          float mR = ((float)(readFromEEPROM(AddPRP2Y) - readFromEEPROM(AddPRP1Y)) / (float)(readFromEEPROM(AddPRP2X) - readFromEEPROM(AddPRP1X)));  // m = (Y2-Y1)/(X2-X1)
          float bR = (float)readFromEEPROM(AddPRP1Y) - mR * (float)readFromEEPROM(AddPRP1X);                                                         // b = Y1-m*X1
          Serial.print("slope m: \t  ");
          Serial.println(mR);
          Serial.print("offset b: \t  ");
          Serial.println(bR);

          Serial.println("Enter any key to exit");
          while (!Serial.available()) {
            delay(100);
          }
          while (Serial.available()) {
            Serial.read(); // read and clear Input
          }
          break;
        }
      default:
        // Statement(s)
        Serial.println("unkown cmd");
        break;  // Wird nicht benötigt, wenn Statement(s) vorhanden sind
    }
  }
}

int findCmdIndex(char message[], int number_of_cmds) {
  for (int i = 0; i <= number_of_cmds; i++) {
    if (strcmp(message, map_cmd[i].cmd_name) == 0) {
      return i;
      break;
    }
  }
  return -1;
}

int eingabePunkt(String Koordinate) {
  int X = 0;
  Serial.print("Eingabe ");
  Serial.print(Koordinate);
  Serial.println(":");
  while (Serial.available() == 0)
    ;
  while (Serial.available() > 0) {
    X = Serial.parseInt();
    Serial.read();
  }
  return X;
}

// Funktion zum Schreiben eines Wertes in den EEPROM
void write2EEPROM(int memoryAddress, uint16_t wData, uint16_t dataLength) {
  byte data[2];
  data[0] = (byte)(wData & 0x00FF);
  data[1] = (byte)((wData & 0xFF00) >> 8);
  // &data[0] zeigt auf byte data[0] im Speicher des Arduinos
  ee.writeBlock(memoryAddress, &data[0], dataLength);
}

int readFromEEPROM(int memoryAddress) {
  uint16_t j;
  byte rData[2];
  ee.readBlock(memoryAddress, &rData[0], 2);
  j = rData[0] + (rData[1] << 8);
  //Serial.print("read Data from Address ");
  //Serial.print(memoryAddress);
  //Serial.print(": ");
  //Serial.println(j);
  return j;
}

int getMeanAnalogVal(int pin, int NumberOfSamples) {
  int Y = 0;
  for (int i = 0; i < NumberOfSamples; i++) {
    Y = Y + analogRead(pin);
  }
  return Y = Y / 100;
}