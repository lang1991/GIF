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

  int bbits;                  ///< Number of bits left in current byte buffer
  unsigned int bbuf;          ///< Byte buffer to store the bits read from stream

  int cursize;                ///< The current code size. This should be codesize + 1 at the very beginning, since codesize does not include EOI and end code, which we need
  int curmask;
  int codesize;               ///< The code size read from LZW data block (first byte)
  int clear_code;
  int end_code;
  int newcodes;               ///< First available code
  int top_slot;               ///< Highest code for current size, if we reach this, we need to restart and expand 
  int slot;                   ///< Last read code
  int fc, oc;                 ///< first code and old code
  uint8_t *sp;                ///< stack pointer, pointing at the bottom of the stack. This stores decoded pixels(codes)
  uint8_t stack[LZW_SIZTABLE];
  uint8_t suffix[LZW_SIZTABLE];
  uint16_t prefix[LZW_SIZTABLE];
  int bs;                     ///< Number of bytes of data in the stream. This is the first byte in the sub data block

public:
  LZWState(std::ifstream& gifFile);
  bool init(int codeSize);
  int getCode();
  int decode(std::vector<uint8_t>& buffer, int length);
  void skipBadData();
};