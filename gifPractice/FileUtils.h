#pragma once
#include <string>
#include <fstream>

inline uint16_t convertToUnsigned(uint8_t one, uint8_t two) {
  return one | (two << 8);
}

inline char getc(std::ifstream& file) {
  char result = 0;
  file.read(&result, 1);
  //assert(1 == file.gcount());

  return result;
}

inline uint8_t getByte(std::ifstream& file) {
  return (uint8_t)getc(file);
}

inline uint16_t getUnsigned(std::ifstream& file) {
  uint8_t one = getByte(file);
  uint8_t two = getByte(file);
  return one | (two << 8);
}

inline uint32_t getBlock(std::ifstream& file, char* result, uint32_t length) {
  file.read((char*)result, length);
  uint32_t bytesRead = file.gcount();
  //assert(length == file.gcount());

  return bytesRead;
}

inline uint32_t getSizeThenBlock(std::ifstream& file, char* result) {
  uint8_t length = getByte(file);
  getBlock(file, result, length);

  return length;
}