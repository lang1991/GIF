#include "LZW.h"
#include "FileUtils.h"
#include <iostream>

LZWState::LZWState(std::ifstream& gifFile): 
  file(gifFile) {}

bool LZWState::init(int codeSizeFromStream) {
  if (codeSizeFromStream < 1 || codeSizeFromStream >= LZW_MAXBITS)
    return false;
  // Read buffer related
  byteBuffer = 0;
  numBitsInByte = 0;
  bytesRemaining = 0;

  // Decoder related
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

// This code is a modification of Steven A. Bennett's GIF decoder in 1987
// The Stackoverflow post here: http://stackoverflow.com/questions/10450395/lzw-decompression-algorithm
// And the GIF decompression algorithm here: https://www.eecis.udel.edu/~amer/CISC651/lzw.and.gif.explained.html
// Are useful resources to write a GIF decoder
int LZWState::decode(std::vector<uint8_t>& buffer, int length) {
  int currLength = length;
  int code;

  while (true) {
    // If we have pixel to output, we write it out
    if (sp > stack) {
      buffer.push_back(*(--sp));
      if ((--currLength) == 0) {
        break;
      }
      continue;
    }
    code = getCode();
    if (code == endCode) { // We are done!
      break;
    }
    else if (code == clearCode) { // Clear related state to start over
      currSize = codeSize + 1;
      currMask = mask[currSize];
      dictCurrSlot = newCodeStart;
      dictLastSlot = 1 << currSize;
      firstChar = oldCode = -1;
    }
    else {
      // This is our first code
      // We treat it as a special case because the process relies on the "old code"
      if (oldCode == -1) {
        firstChar = oldCode = code;
        *sp++ = code;
        continue;
      }

      int codeBackup = code;
      
      // The code should not jump, if a new code shows up, it must be the next slot
      if (code > dictCurrSlot) {
        break;
      }

      // We have a code we have not seen before!
      // According to the algorithm, we output "the string behind code + the first character of that string"
      // We output the first character first, in the while loop below, we output the rest of the string
      if (code == dictCurrSlot) {
        *sp++ = firstChar;
        code = oldCode;
      }

      // Trace the suffix/prefix array to output the string behind a code that is not in the initial dictionary
      while (code >= newCodeStart) {
        *sp++ = suffix[code];
        code = prefix[code];
      }

      // Output the first character in the code
      // This will always be in the initial dictionary
      *sp++ = firstChar = code;

      // If we still have space in the dictionary
      // We add the code to the dictionary
      if (dictCurrSlot < dictLastSlot) {
        suffix[dictCurrSlot] = code;
        prefix[dictCurrSlot++] = oldCode;
      }

      oldCode = codeBackup;

      // Expand the dictionary if we have not surpassed limit of GIF specification
      if (dictCurrSlot >= dictLastSlot && currSize < LZW_MAXBITS) {
        dictLastSlot <<= 1;
        currMask = mask[++currSize];
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