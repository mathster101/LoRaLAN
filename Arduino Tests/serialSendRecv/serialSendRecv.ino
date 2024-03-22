#include "heltec.h"
#include <cstring>

char recvBuffer[2001];

void setup() {
  Serial.begin(115200);
  Heltec.begin(1, 0, 0);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void loop() {

  if (Serial.available() > 0) {
    Heltec.display->clear();
    int numBytes = Serial.readBytesUntil('\n', recvBuffer, 1501);
    recvBuffer[numBytes + 1] = '\0';
    Heltec.display->drawString(0, 10, recvBuffer);
    Serial.println(recvBuffer);
    Heltec.display->display();
  }
}
