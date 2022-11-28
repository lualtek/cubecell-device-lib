

#include "LualtekCubecellLib.h"
// #include <Seeed_BME280.h>
#include <Wire.h>

/* OTAA */
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define DEBUG_SERIAL_ENABLED 0
/* Data transmission duty cycle.  value in [ms].*/
#define DEFAULT_DUTY_CYCLE_MINUTES MINUTES_20_IN_MILLISECONDS
/* Application port (BME280) */
uint8_t appPort = 3;

#define debugSerial Serial

LualtekCubecell ll(DEFAULT_DUTY_CYCLE_MINUTES, CLASS_A, LORAMAC_REGION_EU868, debugSerial);
// BME280 bme280;

int temperature, humidity, batteryVoltage, batteryLevel;
long pressure;

void downLinkDataHandle(McpsIndication_t *mcpsIndication) {
  ll.onDownlinkReceived(mcpsIndication);
  deviceState = DEVICE_STATE_SEND;
}

/* Prepares the payload of the frame */
void onSendUplink(uint8_t port) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(500);

  // while (!bme280.init()) {
  //   if (DEBUG_SERIAL_ENABLED) {
  //     debugSerial.println("Waiting for device...");
  //   }
  // }

  // temperature = bme280.getTemperature();
  // humidity = bme280.getHumidity();
  // pressure = bme280.getPressure();

  // Turn the power to the sensor off again
  Wire.end();
  digitalWrite(Vext, HIGH);

  batteryVoltage = getBatteryVoltage();
  batteryLevel = (BoardGetBatteryLevel() / 254) * 100;

  appDataSize = 12;
  appData[0] = highByte(temperature);
  appData[1] = lowByte(temperature);

  appData[2] = highByte(humidity);
  appData[3] = lowByte(humidity);

  appData[4] = (byte) ((pressure & 0xFF000000) >> 24 );
  appData[5] = (byte) ((pressure & 0x00FF0000) >> 16 );
  appData[6] = (byte) ((pressure & 0x0000FF00) >> 8  );
  appData[7] = (byte) ((pressure & 0X000000FF)       );

  appData[8] = highByte(batteryVoltage);
  appData[9] = lowByte(batteryVoltage);

  appData[10] = highByte(batteryLevel);
  appData[11] = lowByte(batteryLevel);

  if (DEBUG_SERIAL_ENABLED) {
    debugSerial.print("Temperature: ");
    debugSerial.print(temperature);
    debugSerial.print("C, Humidity: ");
    debugSerial.print(humidity);
    debugSerial.print("%, Pressure: ");
    debugSerial.print(pressure / 100);
    debugSerial.print(" mbar, Battery Voltage: ");
    debugSerial.print(batteryVoltage);
    debugSerial.print(" mV, Battery Level: ");
    debugSerial.print(batteryLevel);
    debugSerial.println(" %");
  }
}

void setup() {
  if (DEBUG_SERIAL_ENABLED) {
    debugSerial.begin(115200);
  }

  boardInitMcu();

  ll.setup();
  ll.join();
}

void loop() {
  ll.loop();
}
