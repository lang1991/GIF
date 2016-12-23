#include "GIFReader.h"

char GIFReader::getc() {
  char result = 0;
  file.read(&result, 1);
  assert(1 == file.gcount());

  return result;
}

uint8_t GIFReader::getByte() {
  return (uint8_t)getc();
}

uint16_t GIFReader::getUnsigned() {
  uint8_t one = getByte();
  uint8_t two = getByte();
  return one | (two << 8);
}

uint32_t GIFReader::getBlock(char* result, uint32_t length) {
  file.read((char*) result, length);
  uint32_t bytesRead = file.gcount();
  assert(length == file.gcount());

  return bytesRead;
}

bool GIFReader::parseGIF(GIFFile& result) {
  std::string signature(6, ' ');
  getBlock(&signature[0], 6);
  if (signature != "GIF87a" && signature != "GIF89a") {
    std::cout << "Bad image signature, aborting.\n";
    return false;
  }
  result.version = signature;

  if (!readLogicalScreenDesc(result)) {
    return false;
  }
}

bool GIFReader::readLogicalScreenDesc(GIFFile& result) {
  result.screenWidth = getUnsigned();
  result.screenHeight = getUnsigned();
  uint8_t packed = getByte();
  result.backgroundColorIndex = getByte();
  // we don't care about aspect ratio
  getByte();

  // Read global color table
  return true;
}