#include "Arduino.h"
#include "LoRaWan_APP.h"

uint16_t userChannelsMask[6] = { 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool keepNet = false;
bool isTxConfirmed = false;

uint8_t confirmedNbTrials = 4;

/* ABP (not used) here as placeholder as required for Cubecell lib */
uint8_t nwkSKey[] = { 0x00 };
uint8_t appSKey[] = { 0x00 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;
