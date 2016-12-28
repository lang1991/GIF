#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>

const int MAX_GIF_BLOCK_SIZE = 255;
const uint8_t NETSCAPE_LOOPING_EXTENSION_SUB_BLOCK_ID = 1;
const uint8_t NETSCAPE_BUFFERING_EXTENSION_SUB_BLOCK_ID = 2;

enum DISPOSAL_METHOD {
  DISPOSAL_NONE = 0,
  DISPOSAL_ASIS = 1,
  DISPOSAL_BACKGROUND = 2,
  GIF_DISPOSAL_PREVIOUS = 3,
};

struct Pixel {
  uint8_t red;       /* red component (0-255) */
  uint8_t green;     /* green component (0-255) */
  uint8_t blue;      /* blue component (0-255) */
  bool isTransparent;

  Pixel(uint8_t r, uint8_t g, uint8_t b, bool transparent): 
    red (r),
    green (g),
    blue (b),
    isTransparent (transparent) {}

  void print() {
    std::cout << "(" << red << "," << green << "," << blue << "," << isTransparent << ")";
  }
};

struct GIFImage {
  uint16_t width;
  uint16_t height;
  uint16_t left;
  uint16_t top;
  uint16_t delay;
  uint8_t disposal;
  short transparentIndex;          /* -1 = no transparent index */
  bool interlace;
  std::vector<uint8_t> data;
  std::vector<Pixel> localColormap;

  GIFImage() {
    reset();
  }

  void reset() {
    width = 0;
    height = 0;
    left = 0;
    top = 0;
    delay = 0;
    disposal = DISPOSAL_NONE;
    transparentIndex = -1;
    interlace = false;
    data.clear();
    localColormap.clear();
  }
};

struct GIFFile {
  std::string version;
  std::vector<GIFImage> images;
  std::vector<Pixel> globalColormap;
  uint16_t backgroundColorIndex;        /* 256 = no background */
  uint16_t screenWidth;
  uint16_t screenHeight;
  long loopcount;             /* -1 = no loop count; 0 = infinite loop */
  std::vector<std::string> comments;

  GIFFile():
    backgroundColorIndex(256),
    screenWidth(0),
    screenHeight(0),
    loopcount(-1) {}

  void printProperties() {
    std::cout << "Version: " << version << "\n";
    std::cout << "Logical screen width: " << screenWidth << "\n";
    std::cout << "Logical screen height: " << screenHeight << "\n";
    std::cout << "global color table size: " << globalColormap.size() << "\n";
    std::cout << "Background color index: " << backgroundColorIndex << "\n";
    std::cout << "Comments: \n";
    for (size_t i = 0; i < comments.size(); ++i) {
      std::cout << "\t" << comments[i] << "\n";
    }
  }
};

class GIFReader {
private:
  std::ifstream& file;
  char buffer[MAX_GIF_BLOCK_SIZE];

public:
  GIFReader(std::ifstream& gifFile) : file (gifFile) {}
  bool parseGIF(GIFFile& result);
  bool readLogicalScreenDesc(GIFFile& result);
  void readColorTable(int tableSize, std::vector<Pixel>& result);
  bool readImageBlock(GIFFile& result, GIFImage& image);
  bool readExtensionBlock(GIFFile& result, GIFImage& image);
  void readGraphicControlExtension(GIFImage& image);
  void readCommentExtension(GIFFile& result);
  void readApplicationControlExtension(GIFFile& result);
};
