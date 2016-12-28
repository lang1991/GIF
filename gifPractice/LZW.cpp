#include "LZW.h"
#include "FileUtils.h"
#include <iostream>

LZWState::LZWState(std::ifstream& gifFile): 
  file(gifFile) {}

bool LZWState::init(int codeSize) {
  if (codeSize < 1 || codeSize >= LZW_MAXBITS)
    return false;
  /* read buffer */
  bbuf = 0;
  bbits = 0;
  bs = 0;

  /* decoder */
  codesize = codeSize;
  cursize = this->codesize + 1;
  curmask = mask[cursize];
  top_slot = 1 << cursize;
  clear_code = 1 << codesize;
  end_code = clear_code + 1;
  slot = newcodes = clear_code + 2;
  oc = fc = -1;
  sp = stack;

  return true;
}

int LZWState::getCode() {
  int c;

  while (bbits < cursize) { // if not enough bits in current byte buffer for a code
    if (!bs) { // if the current sub block has been depleted (or upon start up), read a byte from the data block
      bs = getByte(file);
    }
    // Add the newly read byte into the byte buffer
    bbuf |= getByte(file) << bbits;
    // We read a new byte, therefore, we read 8 more bits
    bbits += 8;
    // We just consumed a byte from the file; therefore we decrease the bytes remaining in the stream by 1
    bs--;
  }
  // The byte buffer has the code we need, but it is mingled within the same byte with the next code
  c = bbuf;
  // We can update the byte buffer now since "c" already has the information we need to extract the code
  bbuf >>= cursize;


  // The code is extracted by "bbuf >>= cursize", so we chop off the bits for that code
  bbits -= cursize;
  // using the mask on "c", we get our code!
  return c & curmask;
}

int LZWState::decode(std::vector<uint8_t>& buffer, int length) {
  int l, c, code, oc, fc;
  uint8_t *sp;

  if (this->end_code < 0)
    return 0;

  l = length;
  sp = this->sp;
  oc = this->oc;
  fc = this->fc;

  while(true) {
    if (sp > this->stack) {
      buffer.push_back(*(--sp));
      if ((--l) == 0) {
        break;
      }
      continue;
    }
    c = getCode();
    if (c == this->end_code) {
      break;
    }
    else if (c == this->clear_code) {
      this->cursize = this->codesize + 1;
      this->curmask = mask[this->cursize];
      this->slot = this->newcodes;
      this->top_slot = 1 << this->cursize;
      fc = oc = -1;
    }
    else {
      code = c;
      if (code == this->slot && fc >= 0) {
        *sp++ = fc;
        code = oc;
      }
      else if (code >= this->slot)
        break;
      while (code >= this->newcodes) {
        *sp++ = this->suffix[code];
        code = this->prefix[code];
      }
      *sp++ = code;
      if (this->slot < this->top_slot && oc >= 0) {
        this->suffix[this->slot] = code;
        this->prefix[this->slot++] = oc;
      }
      fc = code;
      oc = c;
      if (this->slot >= this->top_slot) {
        if (this->cursize < LZW_MAXBITS) {
          this->top_slot <<= 1;
          this->curmask = mask[++this->cursize];
        }
      }
    }
  }
  this->sp = sp;
  this->oc = oc;
  this->fc = fc;
  return length - l;
}

void LZWState::skipBadData() {
  std::cout << "Skipping bad LZW data\n";
  char buffer[256];
  while (bs > 0) {
    uint32_t count = getBlock(file, buffer, bs);
    if (count != bs) {
      break;
    }
    bs = getByte(file);
  }
}