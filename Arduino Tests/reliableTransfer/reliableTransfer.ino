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

void setup()
{
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

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
}

int counter = 0;
// tx loop
void loop()
{
  Heltec.display->clear();
  ++counter;
  // char text[100] = "my name is mathew";
  char beagleText[] = "Not only is the Beagle an excellent hunting dog and loyal companion, it is also happy-go-lucky, funny, and thanks to its pleading expression cute. They were bred to hunt in packs, so they enjoy company and are generally easygoing. There are two Beagle varieties: those standing under 13 inches at the shoulder, and those between 13 and 15 inches. Both varieties are sturdy, solid, and big for their inches, as dog folks say. They come in such pleasing colors as lemon, red and white, and tricolor. The Beagle's fortune is in his adorable face, with its big brown or hazel eyes set off by long, houndy ears set low on a broad head. A breed described as 'merry' by its fanciers, Beagles are loving and lovable, happy, and companionable'all qualities that make them excellent family dogs. No wonder that for years the Beagle has been the most popular hound dog among American pet owners. These are curious, clever, and energetic hounds who require plenty of playtime. I personally own a beagle named Hugo baby. He is so sweet and adorable. I am lucky to have him as a part of my life.";
  char sendText[1200];
  sprintf(sendText, "%d %s", counter, beagleText);
  long start = millis();
  rc.sendData(sendText, strlen(sendText));
  long end = millis();
  Serial.print(1000 * strlen(sendText) / (end - start));
  Serial.println("Bytes/Second");

  char outputBuffer[100];
  sprintf(outputBuffer, "%f Bits/sec", 8*1000 * strlen(sendText) / (float)(end - start));
  Heltec.display->drawString(0, counter%55, (char *)outputBuffer);
  Heltec.display->display();
}

// rx loop
//  void loop() {
//    byte *data;
//    int size = rc.receiveData(&data);
//    for (int c = 0; c < size; c++) { Serial.print((char)*(data + c)); }
//    Serial.println("");
//  }