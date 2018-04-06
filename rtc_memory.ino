bool readRtcMemory(RtcData* data) {
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) data, sizeof(RtcData))) {
    uint32_t crcOfData = calculateCRC32(((uint8_t*) data) + 4, sizeof(RtcData) - 4);
    
    if (crcOfData != data->crc32) {
      return false;
    }
    else {
      return true;
    }
  }
}

bool writeRtcMemory(RtcData* data) {
  data->crc32 = calculateCRC32(((uint8_t*) data) + 4, sizeof(RtcData) - 4);
  return ESP.rtcUserMemoryWrite(0, (uint32_t*) data, sizeof(RtcData));
}

uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}
