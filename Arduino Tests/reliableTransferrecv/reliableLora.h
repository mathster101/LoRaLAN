#include <Arduino.h>

#include <stdlib.h>
#include <sys/types.h>
#include <cstring>
#include <vector>
#include <memory>

#include <SPI.h>
#include <Wire.h>
#include <RadioLib.h>
#include "heltec.h"

#define PDATA_SIZE 70
#define RETRY_THRESH 20

typedef uint8_t byte;

class dataPacket
{
public:
  int16_t packetId;
  int16_t bytesToTake;
  byte rawData[PDATA_SIZE];

  dataPacket()
  {
    packetId = 0;
    memset(rawData, 0, sizeof(char) * PDATA_SIZE);
  }
};

class ackPacket
{
public:
  byte highestContinuous;
  ackPacket()
  {
    highestContinuous = 0;
  }
};

class reliableConnection
{
public:
  int sendData(void *data, int size);
  int receiveData(byte **data);
  SX1262 *radio;

private:
  int mySocket;
  int windowSize;
  int numPackets(int dataSize);
  std::vector<std::unique_ptr<byte>> splitIntoChunks(const void *data, int size, int &sizeOfLastChunk);
  std::vector<dataPacket> generatePackets(const void *data, int size);
  byte *reassembleChunks(std::vector<dataPacket> rcvdPackets, int &totalNumberofBytes);
  int readFromTimeout(void *buffer, int sizeOfBuffer);
};