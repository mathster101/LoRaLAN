#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include "heltec.h"

#include "reliableLora.h"
#include <vector>

#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
reliableConnection rc;

void setup() {
  Serial.begin(115200);
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, false /*Serial Enable*/);

  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);

  int state = radio.begin();
  radio.setFrequency(915);
  radio.setOutputPower(22);
  radio.setBandwidth(500.0);
  radio.setSpreadingFactor(8);
  radio.setCodingRate(5);
  radio.setPreambleLength(8);
  radio.setCRC(true);
  radio.setCurrentLimit(140);
  rc.radio = &radio;

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
}

int counter = 0;
void loop() {
  byte *data;
  int size = rc.receiveData(&data);
  String snr = String(rc.radio->getSNR());
  ++counter;
  Heltec.display->drawString(0, counter%45, (char*)data);
  Heltec.display->drawString(0, counter%45+10, snr);
  Heltec.display->display();
  for (int c = 0; c < size; c++) { Serial.print((char)*(data + c)); }
  Serial.println("");
  Heltec.display->clear();
}
