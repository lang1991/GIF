#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>

struct Pixel {
  uint8_t red;       /* red component (0-255) */
  uint8_t green;     /* green component (0-255) */
  uint8_t blue;      /* blue component (0-255) */
  bool isTransparent;
};

struct GIFImage {
  
};

struct GIFFile {
  std::string version;
  std::vector<GIFImage> images;
  std::vector<Pixel> globalColormap;
  uint16_t backgroundColorIndex;        /* 256 means no background */
  uint16_t screenWidth;
  uint16_t screenHeight;
  long loopcount;             /* -1 means no loop count */

  void printProperties() {
    std::cout << "Version: " << version << "\n";
    std::cout << "Logical screen width: " << screenWidth << "\n";
    std::cout << "Logical screen height: " << screenHeight << "\n";
    std::cout << "Background color index: " << backgroundColorIndex << "\n";
  }
};

class GIFReader {
private:
  std::ifstream& file;

public:
  GIFReader(std::ifstream& gifFile) : file (gifFile) {}
  char getc();
  uint8_t getByte();
  uint16_t getUnsigned();
  uint32_t getBlock(char* result, uint32_t length);
  bool parseGIF(GIFFile& result);
  bool readLogicalScreenDesc(GIFFile& result);
};