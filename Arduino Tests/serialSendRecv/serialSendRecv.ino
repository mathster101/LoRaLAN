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

  // if (Serial.available() > 0) {
  //   Heltec.display->clear();
  //   int numBytes = Serial.readBytesUntil('\n', recvBuffer, 1501);
  //   recvBuffer[numBytes + 1] = '\0';
  //   Heltec.display->drawString(0, 10, recvBuffer);
  //   Serial.println(recvBuffer);
  //   Heltec.display->display();
  // }
  Serial.println("Not only is the Beagle an excellent hunting dog and loyal companion, it is also happy-go-lucky, funny, and thanks to its pleading expression cute. They were bred to hunt in packs, so they enjoy company and are generally easygoing. There are two Beagle varieties: those standing under 13 inches at the shoulder, and those between 13 and 15 inches. Both varieties are sturdy, solid, and big for their inches, as dog folks say. They come in such pleasing colors as lemon, red and white, and tricolor. The Beagle's fortune is in his adorable face, with its big brown or hazel eyes set off by long, houndy ears set low on a broad head. A breed described as 'merry' by its fanciers, Beagles are loving and lovable, happy, and companionable'all qualities that make them excellent family dogs. No wonder that for years the Beagle has been the most popular hound dog among American pet owners. These are curious, clever, and energetic hounds who require plenty of playtime. I personally own a beagle named Hugo baby. He is so sweet and adorable. I am lucky to have him as a part of my life.");
  sleep(1);
}
