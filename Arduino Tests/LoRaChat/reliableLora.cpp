#include "reliableLora.h"

// splits input data into packet friendly chunks
std::vector<std::unique_ptr<byte>> reliableConnection::splitIntoChunks(const void *data, int size, int &sizeOfLastChunk)
{

  std::vector<std::unique_ptr<byte>> chunks;
  int numChunks = numPackets(size);

  // allocating the memory required
  for (int chunkNum = 0; chunkNum < numChunks; chunkNum++)
  {
    std::unique_ptr<byte> temp(new byte[PDATA_SIZE]);
    chunks.push_back(std::move(temp));
  }

  byte *now = (byte *)data;
  for (int chunkNum = 0; chunkNum < numChunks; chunkNum++)
  {
    // last packet
    if ((size - (now - (byte *)data)) < PDATA_SIZE)
    {
      sizeOfLastChunk = size - (now - (byte *)data);
      memcpy(chunks[chunkNum].get(), now, sizeOfLastChunk);
    }
    else
    {
      memcpy(chunks[chunkNum].get(), now, PDATA_SIZE);
    }

    now += PDATA_SIZE;
  }

  return chunks;
}

// split tcp packet into smaller radio packets
std::vector<dataPacket> reliableConnection::generatePackets(const void *data, int size)
{
  int sizeOfLastChunk = PDATA_SIZE;
  int numPack = numPackets(size);
  std::vector<std::unique_ptr<byte>> chunks = splitIntoChunks(data, size, sizeOfLastChunk);
  std::vector<dataPacket> packets;
  for (int pack = 0; pack < numPack; pack++)
  {
    dataPacket newPacket;
    newPacket.packetId = pack;
    if (pack == numPack - 1)
      newPacket.bytesToTake = sizeOfLastChunk;
    else
      newPacket.bytesToTake = PDATA_SIZE;
    memcpy(newPacket.rawData, chunks[pack].get(), newPacket.bytesToTake);
    packets.push_back(newPacket);
  }
  return packets;
}

// timeout read function
int reliableConnection::readFromTimeout(void *buffer, int sizeOfBuffer)
{
  int state = radio->receive((uint8_t *)buffer, sizeOfBuffer);
  if (state == RADIOLIB_ERR_RX_TIMEOUT || state == RADIOLIB_ERR_CRC_MISMATCH)
    return 0;
  return sizeOfBuffer;
}

// reliably send one packet at a time
int reliableConnection::sendData(void *data, int size)
{
  int numberOfPackets = numPackets(size);
  int failureCounter = 0;
  if (numberOfPackets >= 30000)
  {
    Serial.println("data too big!");
    return -1;
  }
  std::vector<dataPacket> packets = generatePackets(data, size);
  ackPacket ack;
  for (int i = 0; i < numberOfPackets;)
  {
    //Serial.print("Packet sent:");
    //Serial.println(i);
    // int bytesSent = sendto(mySocket, (void *)&packets[i], sizeof(dataPacket), MSG_CONFIRM,
    //                        (sockaddr *)&targetAddress, sizeof(targetAddress));

    int state = radio->transmit((uint8_t *)&packets[i], sizeof(dataPacket));

    int numBytes = readFromTimeout(&ack, sizeof(ack));
    switch (numBytes)
    {
    case -1:
      // ERROR CONDITION
      break;
    case 0: // timeout, so resend the same packet
    {
      Serial.println("timeout no ack");
      ++failureCounter;
      break;
    }
    default:
    {
      //Serial.print("Got ack:");
      Serial.println(i);
      if (ack.highestContinuous == i)
      {
        i++;
        failureCounter = 0;
      }
      else
        ++failureCounter;
    }
    }
    if (failureCounter > RETRY_THRESH)
    {
      // printf("Total timeout...exiting\n");
      return -1;
    }
  }
  dataPacket finPacket;
  finPacket.packetId = numberOfPackets;
  finPacket.bytesToTake = -1 * numberOfPackets;

  memcpy(finPacket.rawData, "<END OF TRANSMISSION>\0", 23);
  for (int i = 0; i < 5; i++) // hail mary
    radio->transmit((uint8_t *)&finPacket, sizeof(dataPacket));
  //Serial.println("Sent FINs!");
  return 0;
}

// put chunks back together from the recvd packets
byte *reliableConnection::reassembleChunks(std::vector<dataPacket> rcvdPackets, int &totalNumberofBytes)
{
  totalNumberofBytes = 0;
  for (int packetCount = 0; packetCount < rcvdPackets.size(); packetCount++)
  {
    totalNumberofBytes += rcvdPackets[packetCount].bytesToTake;
  }

  byte *merged = new byte[totalNumberofBytes];
  for (int packetCount = 0; packetCount < rcvdPackets.size(); packetCount++)
  {
    memcpy(merged + packetCount * PDATA_SIZE, rcvdPackets[packetCount].rawData, rcvdPackets[packetCount].bytesToTake);
  }
  return merged;
}

// reliably receive one packet at a time
int reliableConnection::receiveData(byte **data)
{
  int latestPacketId = 0;
  std::vector<dataPacket> rcvdPackets;
  int failureCounter = 0;
  dataPacket packet;
  ackPacket ack;
  while (true)
  {
    dataPacket packet;
    ackPacket ack;

    int numBytes = readFromTimeout(&packet, sizeof(packet));
    switch (numBytes)
    {
    case -1:
    case 0: // timeout condition
    {
      failureCounter += 1;
      if (failureCounter > RETRY_THRESH && latestPacketId >= 0)
      {
        *data = nullptr;
        return 0;
      }
      break;
    }
    default: // received a valid packet
    {
      if (packet.packetId == latestPacketId)
      {
        failureCounter = 0;
        if (packet.bytesToTake == -1 * latestPacketId) // fin packet
        {
          int totalPacketSize;
          *data = reassembleChunks(rcvdPackets, totalPacketSize);
          return totalPacketSize;
        }
        // printf("got new packet %d\n", latestPacketId);
        //Serial.print("got a new packet");
        //Serial.println(latestPacketId);
        ++latestPacketId;
        rcvdPackets.push_back(packet);
      }

      else // received a repeated or random in-between packet
      {
        failureCounter += 1;
        if (failureCounter > RETRY_THRESH) // quit if many retries
        {
          // printf("Total timeout...exiting\n");
          *data = nullptr;
          return -1;
        }
      }
      // resend ack if nothing new, else send ack with new val
      ack.highestContinuous = latestPacketId - 1;
      //Serial.print("sending ack ");
      //Serial.println(ack.highestContinuous);
      radio->transmit((uint8_t *)&ack, sizeof(ack));
    }
    }
  }
  return 0;
}

// utility
int reliableConnection::numPackets(int dataSize)
{
  if (dataSize % PDATA_SIZE != 0)
    return dataSize / PDATA_SIZE + 1;
  return dataSize / PDATA_SIZE;
}