#include "Arduino.h"
#include "Wire.h"
#include "I2C_eeprom.h"

#ifndef ADJUSTMENTUART_H_
#define ADJUSTMENTUART_H_

// memoryAddress: 0,      2,      4,      6,      8,      10,     12,     14
// Koordinate:    PLP1_X, PLP1_Y, PLP2_X, PLP2_Y, PRP1_X, PRP1_Y, PRP2_X, PRP2_Y,

#define PotiLeftPin A0 // left Poti
#define PotiRightPin A1 // right Poti

#define AddPLP1X 0  // Memory Address of Poti Left P1 X
#define AddPLP1Y 2  // Memory Address of Poti Left P1 Y
#define AddPLP2X 4  // Memory Address of Poti Left P2 X
#define AddPLP2Y 6  // Memory Address of Poti Left P1 Y

#define AddPRP1X 8  // Memory Address of Poti Right P1 X
#define AddPRP1Y 10 // Memory Address of Poti Right P1 Y
#define AddPRP2X 12 // Memory Address of Poti Right P2 X
#define AddPRP2Y 14 // Memory Address of Poti Right P2 Y

//------------------------------------------------------------------------
//                              Parameter
//------------------------------------------------------------------------
extern I2C_eeprom ee;
extern boolean new_message;

//------------------------------------------------------------------------
//                              Funktionen
//------------------------------------------------------------------------
void readFromSerial();
void stateSelect();
//void readPoti();
//int findCmdIndex(char message[], int number_of_cmds);
//int eingabePunkt(String Koordinate);
// Funktion zum Schreiben eines Wertes in den EEPROM
//void write2EEPROM(int memoryAddress, uint16_t wData, uint16_t dataLength);
int readFromEEPROM(int memoryAddress);
int getMeanAnalogVal(int pin, int NumberOfSamples);

#endif /* ADJUSTMENTUART_H_ */