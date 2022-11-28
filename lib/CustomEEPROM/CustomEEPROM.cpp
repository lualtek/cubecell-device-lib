#include <CustomEEPROM.h>

// EEPROM address init (step value is stored here)
#define EEPROM_ADDRESS_ACTION_VALUE 0


void writeEEPROM (int address, int value) {
  EEPROM.begin(512);
  delay(800);
  EEPROM.write(address, value);
  EEPROM.commit();
  EEPROM.end();
}

int readEEPROM (int address) {
  EEPROM.begin(512);
  delay(800);
  int value = (int) EEPROM.read(address);
  EEPROM.end();
  return value;
}
