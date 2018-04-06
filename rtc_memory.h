// CRC function used to ensure data validity
uint32_t calculateCRC32(const uint8_t *data, size_t length);

struct RtcData {
  uint32_t crc32;
  byte data[508];
};
