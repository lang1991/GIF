#include "LZW.h"
#include "FileUtils.h"
#include <iostream>

LZWState::LZWState(std::ifstream& gifFile): 
  file(gifFile) {}

bool LZWState::init(int codeSizeFromStream) {
  if (codeSizeFromStream < 1 || codeSizeFromStream >= LZW_MAXBITS)
    return false;
  /* read buffer */
  byteBuffer = 0;
  numBitsInByte = 0;
  bytesRemaining = 0;

  /* decoder */
  codeSize = codeSizeFromStream;
  currSize = codeSize + 1;
  currMask = mask[currSize];
  dictLastSlot = 1 << currSize;
  clearCode = 1 << codeSize;
  endCode = clearCode + 1;
  dictCurrSlot = newCodeStart = clearCode + 2;
  oldCode = firstChar = -1;
  sp = stack;

  return true;
}

int LZWState::getCode() {
  int codeBuffer;

  while (numBitsInByte < currSize) { // if not enough bits in current byte buffer for a code
    if (!bytesRemaining) { // if the current sub block has been depleted (or upon start up), read a byte from the data block
      bytesRemaining = getByte(file);
    }
    // Add the newly read byte into the byte buffer
    byteBuffer |= getByte(file) << numBitsInByte;
    // We read a new byte, therefore, we read 8 more bits
    numBitsInByte += 8;
    // We just consumed a byte from the file; therefore we decrease the bytes remaining in the stream by 1
    bytesRemaining--;
  }
  // The byte buffer has the code we need, but it is mingled within the same byte with the next code
  codeBuffer = byteBuffer;
  // We can update the byte buffer now since "codeBuffer" already has the information we need to extract the code
  byteBuffer >>= currSize;

  // The code is extracted by "byteBuffer >>= currSize", so we chop off the bits for that code
  numBitsInByte -= currSize;
  // using the mask on "codeBuffer", we get our code!
  return codeBuffer & currMask;
}

int LZWState::decode(std::vector<uint8_t>& buffer, int length) {
  int currLength = length; 
  int c, code;


  while(true) {
    if (sp > stack) {
      buffer.push_back(*(--sp));
      if ((--currLength) == 0) {
        break;
      }
      continue;
    }
    c = getCode();
    if (c == endCode) {
      break;
    }
    else if (c == clearCode) {
      currSize = codeSize + 1;
      currMask = mask[currSize];
      dictCurrSlot = newCodeStart;
      dictLastSlot = 1 << currSize;
      firstChar = oldCode = -1;
    }
    else {
      code = c;
      if (code == dictCurrSlot && firstChar >= 0) {
        *sp++ = firstChar;
        code = oldCode;
      }
      else if (code >= dictCurrSlot)
        break;
      while (code >= newCodeStart) {
        *sp++ = suffix[code];
        code = prefix[code];
      }
      *sp++ = code;
      if (dictCurrSlot < dictLastSlot && oldCode >= 0) {
        suffix[dictCurrSlot] = code;
        prefix[dictCurrSlot++] = oldCode;
      }
      firstChar = code;
      oldCode = c;
      if (dictCurrSlot >= dictLastSlot) {
        if (currSize < LZW_MAXBITS) {
          dictLastSlot <<= 1;
          currMask = mask[++currSize];
        }
      }
    }
  }
  return length - currLength;
}

void LZWState::skipBadData() {
  char buffer[256];
  while (bytesRemaining > 0) {
    std::cout << "Skipping bad LZW data\n";
    uint32_t count = getBlock(file, buffer, bytesRemaining);
    if (count != bytesRemaining) {
      break;
    }
    bytesRemaining = getByte(file);
  }
}