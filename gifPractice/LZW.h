#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <exception>

const int LZW_MAXBITS = 12;
const int LZW_SIZTABLE = (1 << LZW_MAXBITS);

static const uint16_t mask[17] =
{
  0x0000, 0x0001, 0x0003, 0x0007,
  0x000F, 0x001F, 0x003F, 0x007F,
  0x00FF, 0x01FF, 0x03FF, 0x07FF,
  0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

class LZWState {
private:
  std::ifstream& file;

  int numBitsInByte;                  ///< Number of bits left in current byte buffer
  unsigned int byteBuffer;          ///< Byte buffer to store the bits read from stream
  int bytesRemaining;                     ///< Number of bytes of data in the stream. This is the first byte in the sub data block

  int currSize;                ///< The current code size. This should be codeSize + 1 at the very beginning, since codeSize does not include EOI and end code, which we need
  int currMask;
  int codeSize;               ///< The code size read from LZW data block (first byte)
  int clearCode;
  int endCode;
  int newCodeStart;               ///< First available code, this means the first code that is out of the dictionary
  int dictLastSlot;               ///< Highest code for current size. This marks the limit of the current dictionary. If we reach this, we need to restart and expand 
  int dictCurrSlot;                   ///< Last read code
  int firstChar, oldCode;                 ///< first character and old code
  uint8_t *sp;                ///< stack pointer, pointing at the bottom of the stack. This stores decoded pixels(codes)
  uint8_t stack[LZW_SIZTABLE];
  ///< suffix + prefix is an optimization technique for the dictionary lookup. 
  ///< http://warp.povusers.org/EfficientLZW/part5.html this page explains the optimization technique
  uint8_t suffix[LZW_SIZTABLE]; 
  uint16_t prefix[LZW_SIZTABLE];

public:
  LZWState(std::ifstream& gifFile);
  bool init(int codeSize);
  int getCode();
  int decode(std::vector<uint8_t>& buffer, int length);
  void skipBadData();
};