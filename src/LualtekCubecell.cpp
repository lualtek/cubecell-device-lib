#include <LualtekCubecell.h>

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

uint32_t appTxDutyCycle;
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_EU868;
DeviceClass_t  loraWanClass = CLASS_A;

bool isDutyCycleIndex(int commandIndex) {
  return commandIndex >= 0 && commandIndex <= sizeof(dutyCycleCommandTable) - 1;
}

LualtekCubecell::LualtekCubecell(DeviceClass_t deviceClass, LoRaMacRegion_t deviceRegion, lualtek_dowlink_command_dutycycle_index_t dutyCycleIndex) {
  this->previousMillis = 0;
  this->defaultDutyCycleIndex = dutyCycleIndex;
  this->deviceClass = deviceClass;
  this->deviceRegion = deviceRegion;
}

void LualtekCubecell::writeEEPROM(int address, int value) {
  EEPROM.begin(512);
  this->delayMillis(1000);
  EEPROM.write(address, value);
  EEPROM.commit();
  EEPROM.end();
}

int LualtekCubecell::readEEPROM(int address) {
  EEPROM.begin(512);
  this->delayMillis(1000);
  int value = (int) EEPROM.read(address);
  EEPROM.end();
  return value;
}

void LualtekCubecell::delayMillis(unsigned long millisToWait) {
  unsigned long currentMillis = millis();
  while (millis() < currentMillis + millisToWait);
}

void LualtekCubecell::handleChangeDutyCycle(int commandIndex) {
  if (!isDutyCycleIndex(commandIndex)) {
    return;
  }

  appTxDutyCycle = dutyCycleCommandTable[commandIndex];
  this->writeEEPROM(EEPROM_ADDRESS_DUTY_CYCLE_INDEX, commandIndex);
}

void LualtekCubecell::onSendUplink(void (*callback)(int appPort)) {
  this->onSendUplinkCallback = callback;
}

void LualtekCubecell::onDownlinkReceived(McpsIndication_t *mcpsIndication) {
  if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK) {
    return;
  }

  if (mcpsIndication->RxData != true) {
    return;
  }

  switch(mcpsIndication->Port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      this->handleChangeDutyCycle(mcpsIndication->Buffer[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      this->join();
      break;
    default:
      break;
  }
}

void LualtekCubecell::setup() {
  // Setup duty cycle from EEPROM if available or use default
  int currentDutyCycleIndex = this->readEEPROM(EEPROM_ADDRESS_DUTY_CYCLE_INDEX);
  this->handleChangeDutyCycle(isDutyCycleIndex(currentDutyCycleIndex) ? currentDutyCycleIndex : this->defaultDutyCycleIndex);

  loraWanClass = this->deviceClass;
  loraWanRegion = this->deviceRegion;
}

void LualtekCubecell::join() {
  this->delayMillis(1000 + randr(1000, 5000));
  deviceState = DEVICE_STATE_INIT;
}

void LualtekCubecell::loop() {
  switch(deviceState) {
    case DEVICE_STATE_INIT: {
      #if (!CUSTOM_DEVEUI)
        LoRaWAN.generateDeveuiByChipID();
        printDevParam();
      #endif

      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;
    }

    case DEVICE_STATE_JOIN: {
      LoRaWAN.join();
      break;
    }

    case DEVICE_STATE_SEND: {
      this->onSendUplinkCallback(appPort);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE: {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr(0, APP_TX_DUTYCYCLE_RND);
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP: {
      LoRaWAN.sleep();
      break;
    }
    default: {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}
