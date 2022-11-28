#include <LualtekCubecellLib.h>
#include <CustomEEPROM.h>

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

LualtekCubecell::LualtekCubecell(unsigned long dutyCycleMs, DeviceClass_t deviceClass, LoRaMacRegion_t deviceRegion, Stream &debugStream) {
  this->previousMillis = 0;
  this->dutyCycleMs = {
    MINUTES_60_IN_MILLISECONDS,
    MINUTES_40_IN_MILLISECONDS,
    MINUTES_30_IN_MILLISECONDS,
    MINUTES_20_IN_MILLISECONDS,
    MINUTES_15_IN_MILLISECONDS,
    MINUTES_10_IN_MILLISECONDS,
    MINUTES_5_IN_MILLISECONDS,
    dutyCycleMs
  };

  this->deviceClass = deviceClass;
  this->deviceRegion = deviceRegion;
  this->uplinkInterval = dutyCycleMs;
  this->debugStream = &debugStream;
}

void LualtekCubecell::debugPrint(const char *message) {
  if (this->debugStream != NULL && DEBUG_SERIAL_ENABLED) {
    this->debugStream->print(message);
  }
}

void LualtekCubecell::debugPrintln(const char *message) {
  if (this->debugStream != NULL && DEBUG_SERIAL_ENABLED) {
    this->debugStream->println(message);
  }
}

void LualtekCubecell::debugPrintln(int message) {
  if (this->debugStream != NULL && DEBUG_SERIAL_ENABLED) {
    this->debugStream->println(message);
  }
}

void LualtekCubecell::delayMillis(unsigned long millisToWait) {
  unsigned long currentMillis = millis();
  while (millis() < currentMillis + millisToWait);
}

void LualtekCubecell::handleChangeDutyCycle(int commandIndex) {
  int dutyCycleIndexAssinged = -1;

  switch (commandIndex) {
    case MINUTES_60_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes60;
      dutyCycleIndexAssinged = MINUTES_60_COMMAND_INDEX;
      break;
    case MINUTES_40_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes40;
      dutyCycleIndexAssinged = MINUTES_40_COMMAND_INDEX;
      break;
    case MINUTES_30_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes30;
      dutyCycleIndexAssinged = MINUTES_30_COMMAND_INDEX;
      break;
    case MINUTES_20_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes20;
      dutyCycleIndexAssinged = MINUTES_20_COMMAND_INDEX;
      break;
    case MINUTES_15_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes15;
      dutyCycleIndexAssinged = MINUTES_15_COMMAND_INDEX;
      break;
    case MINUTES_10_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes10;
      dutyCycleIndexAssinged = MINUTES_10_COMMAND_INDEX;
      break;
    case MINUTES_5_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutes5;
      dutyCycleIndexAssinged = MINUTES_5_COMMAND_INDEX;
      break;
    case MINUTES_DEFAULT_COMMAND_INDEX:
      this->uplinkInterval = this->dutyCycleMs.minutesDefault;
      dutyCycleIndexAssinged = MINUTES_DEFAULT_COMMAND_INDEX;
      break;
    default:
      break;
  }

  if (dutyCycleIndexAssinged != -1) {
    this->debugPrintln("Duty cycle changed");
    this->debugPrint("Duty cycle: ");
    this->debugPrintln(this->uplinkInterval);

    appTxDutyCycle = this->uplinkInterval;
    writeEEPROM(EEPROM_ADDRESS_DUTY_CYCLE_INDEX, dutyCycleIndexAssinged);
  }
}

void LualtekCubecell::onSendUplink(void (*callback)(int appPort)) {
  this->onSendUplinkCallback = callback;
}

void LualtekCubecell::onDownlinkReceived(McpsIndication_t *mcpsIndication) {
  if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK) {
    this->debugPrint("LoRaWAN downlink data handle failed with status: ");
    this->debugPrintln(mcpsIndication->Status);
    return;
  }

  if (mcpsIndication->RxData != true) {
    this->debugPrintln("LoRaWAN downlink data handle failed: no data received");
    return;
  }

  switch(mcpsIndication->Port) {
    case DOWNLINK_ACTION_CHANGE_INTERVAL_PORT:
      this->debugPrintln("Received downlink for changing duty cycle");
      this->handleChangeDutyCycle(mcpsIndication->Buffer[0]);
      break;
    case DOWNLINK_ACTION_REJOIN_PORT:
      this->debugPrintln("Received downlink for rejoin. Rejoining...");
      this->join();
      break;
    default:
      break;
  }
}

void LualtekCubecell::setup() {
  // Setup duty cycle from EEPROM if available or use default
  int currentDutyCycleIndex = readEEPROM(EEPROM_ADDRESS_DUTY_CYCLE_INDEX);
  if (currentDutyCycleIndex >= MINUTES_60_COMMAND_INDEX && currentDutyCycleIndex <= MINUTES_DEFAULT_COMMAND_INDEX) {
    this->handleChangeDutyCycle(currentDutyCycleIndex);
  } else {
    this->handleChangeDutyCycle(MINUTES_DEFAULT_COMMAND_INDEX);
  }

  // Setup LoRaWAN
  deviceClass = this->deviceClass;
  deviceRegion = this->deviceRegion;
}

void LualtekCubecell::join() {
  this->delayMillis(5000 + randr(0, APP_TX_DUTYCYCLE_RND));
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
}

void LualtekCubecell::loop() {
  switch(deviceState) {
    case DEVICE_STATE_INIT: {
      LoRaWAN.generateDeveuiByChipID();
      printDevParam();
      LoRaWAN.init(deviceClass, deviceRegion);
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
