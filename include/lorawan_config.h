#include "Arduino.h"
#include "LoRaWan_APP.h"

uint16_t userChannelsMask[6] = { 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool keepNet = false;
bool isTxConfirmed = false;

uint8_t confirmedNbTrials = 4;