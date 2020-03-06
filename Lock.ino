#include <SPI.h>
#include <Adafruit_BLE_UART.h>

// Connect CLK/MISO/MOSI to hardware SPI
// e.g. On UNO & compatible: CLK = 13, MISO = 12, MOSI = 11
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9

Adafruit_BLE_UART bluetooth = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

char ON[] = {'O','N'};
char OFF[] = {'O','F','F'};
//const char ON[5] = {'A','L','_','U','P'};
//const char OFF[5] = {'A','L','S','T','P'};

void setup() {
  Serial.begin(9600);
  Serial.println("Bluetooth door lock is starting...");
  
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

  bluetooth.setDeviceName("B16LOCK");
  bluetooth.begin();
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop() {
  bluetooth.pollACI();

  aci_evt_opcode_t blueStatus = bluetooth.getState();
  if (blueStatus != laststatus) {
    if (blueStatus == ACI_EVT_DEVICE_STARTED) Serial.println(F("* Advertising started"));
    if (blueStatus == ACI_EVT_CONNECTED) Serial.println(F("* Connected!"));
    if (blueStatus == ACI_EVT_DISCONNECTED) Serial.println(F("* Disconnected or advertising timed out"));
    laststatus = blueStatus;
  }

  if (blueStatus == ACI_EVT_CONNECTED) {
    boolean hasData = false;
    int dataSize = 0;
    if (bluetooth.available()) {
      Serial.print("* ");
      dataSize = bluetooth.available();
      Serial.print(dataSize);
      Serial.println(F(" bytes available from BTLE"));
      hasData = true;
    }

    if (hasData) {
      char data[dataSize] = {0, 0, 0, 0, 0};
      for (int i = 0; i < dataSize; i++) {
        char c = bluetooth.read();
        Serial.print(c);
        data[i] = c;
      }
      Serial.print("\n");
      Serial.print(sizeof(data));
      Serial.println(F(" are stored in data"));
      processData(data, sizeof(data));
    }
  }
}

boolean compareCode(char code1[], int code1Len, char code2[], int code2Len) {
  if (code1Len != code2Len) return false;
  for (int i = 0; i < code1Len; i++) {
    if (code1[i] != code2[i]) return false;
  }
  return true;
}

void processData(char data[], int dataLen) {
  Serial.print(F("Processing data, sizeof(data) = "));
  Serial.println(sizeof(data));
  for (int i = 0; i < dataLen; i++) {
    Serial.print(data[i]);
  }
  Serial.print("\n");
  if (compareCode(data, dataLen, ON, sizeof(ON))) {
    Serial.println("ON");
    digitalWrite(3, HIGH);
  }
  if (compareCode(data, dataLen, OFF, sizeof(OFF))) {
    Serial.println("OFF");
    digitalWrite(3, LOW);
  }
}
