#include "GIFReader.h"
#include "FileUtils.h"
#include "LZW.h"

bool GIFReader::parseGIF(GIFFile& result) {
  std::string signature(6, ' ');
  getBlock(file, &signature[0], 6);
  if (signature != "GIF87a" && signature != "GIF89a") {
    std::cout << "Bad image signature, aborting.\n";
    return false;
  }
  result.version = signature;

  if (!readLogicalScreenDesc(result)) {
    return false;
  }

  GIFImage image;

  while (!file.eof()) {
    uint8_t blockType = getByte(file);
    if (blockType == ',') {
      std::cout << "Image block hit\n";
      if (!readImageBlock(result, image)) {
        if (result.images.empty()) { // no image is decoded successfully, fail
          return false;
        } else { // use whatever we have decoded and call it the day
          break;
        }
      }
    } else if (blockType == '!') {
      std::cout << "Extension block hit\n";
      assert(readExtensionBlock(result, image));
    } else if (blockType == ';') {
      std::cout << "Terminator block hit\n";
    } else {
      std::cout << "Unknown block hit\n";
      while (getSizeThenBlock(file, buffer) > 0);
    }
  }

  return true;
}

bool GIFReader::readLogicalScreenDesc(GIFFile& result) {
  result.screenWidth = getUnsigned(file);
  result.screenHeight = getUnsigned(file);
  uint8_t packed = getByte(file);
  result.backgroundColorIndex = getByte(file);
  // we don't care about aspect ratio
  getByte(file);

  // Read global color table
  if (packed & 0x80) {
    int size = 1 << ((packed & 0x07) + 1);
    readColorTable(size, result.globalColormap);
  } else {
    result.backgroundColorIndex = 256;
  }

  return true;
}

void GIFReader::readColorTable(int tableSize, std::vector<Pixel>& result) {
  result.reserve(tableSize);
  for (int i = 0; i < tableSize; ++i) {
    uint8_t r = getByte(file);
    uint8_t g = getByte(file);
    uint8_t b = getByte(file);
    result.emplace_back(Pixel(r, g, b, false));
  }
}

bool GIFReader::readImageBlock(GIFFile& result, GIFImage& image) {
  image.left = getUnsigned(file);
  image.top = getUnsigned(file);
  image.width = getUnsigned(file);
  image.height = getUnsigned(file);
  if (image.width == 0) {
    image.width = result.screenWidth;
  }

  if (image.height == 0) {
    image.height = result.screenHeight;
  }

  if (image.width == 0 || image.height == 0) {
    std::cout << "Invalid image width/height.\n";
    return false;
  }

  if (image.width > result.screenWidth || image.left > result.screenWidth || image.left + image.width > 0xFFFF
    || image.height > result.screenHeight || image.top > result.screenHeight || image.top + image.height > 0xFFFF) {
    std::cout << "Image width/height out of range.\n";
    return false;
  }

  uint8_t packed = getByte(file);
  if (packed & 0x80) { // have a local color table
    int size = 1 << ((packed & 0x07) + 1);
    readColorTable(size, image.localColormap);
  } else if (result.globalColormap.empty()) { // have no local or global color table
    std::cout << "Image has no local or global color table\n";
    return false;
  }
  image.interlace = (packed & 0x40) != 0;

  uint8_t codeSize = getByte(file);
  LZWState lzw(file);
  if (!lzw.init(codeSize)) {
    return false;
  }

  for (size_t y = 0; y < image.height; ++y) {
    int count = lzw.decode(image.data, image.width);
    if (count != image.width) {
      lzw.skipBadData();
      return false;
    }
  }
  
  lzw.skipBadData();
  result.images.emplace_back(image);
  image.reset();
  return true;
}

bool GIFReader::readExtensionBlock(GIFFile& result, GIFImage& image) {
  uint8_t blockType = getByte(file);
  switch (blockType) {
    case 0xF9:
      std::cout << "Graphic control extension\n";
      readGraphicControlExtension(image);
      break;
    case 0xFE:
      std::cout << "Comment extension\n";
      readCommentExtension(result);
      break;
    case 0xFF:
      std::cout << "Application control extension\n";
      readApplicationControlExtension(result);
      break;
    default:
      std::cout << "Unknown extension\n";
      char buffer[MAX_GIF_BLOCK_SIZE];
      while (getSizeThenBlock(file, buffer) > 0);
      break;
  }

  return true;
}

void GIFReader::readGraphicControlExtension(GIFImage& image) {
  uint8_t length = getByte(file);

  if (length == 4) {
    uint8_t packed = getByte(file);
    image.disposal = (packed >> 2) & 0x07;
    if (image.disposal > 3) {
      image.disposal = DISPOSAL_NONE;
    }
    image.delay = getUnsigned(file);
    if (image.delay < 10) {
      image.delay = 10;
    }
    image.transparentIndex = getByte(file);
    if (!(packed & 0x01)) { // transparent color doesn't exist
      image.transparentIndex = -1;
    } 
    length -= 4;
  }

  if (length > 0) {
    getBlock(file, buffer, length);
  }

  while (getSizeThenBlock(file, buffer) > 0);
}

void GIFReader::readCommentExtension(GIFFile& result) {
  std::string comment;
  char buffer[MAX_GIF_BLOCK_SIZE];
  uint8_t length;
  while ((length = getSizeThenBlock(file, buffer)) > 0) {
    comment.append(buffer, length);
  }

  result.comments.emplace_back(comment);
}

void GIFReader::readApplicationControlExtension(GIFFile& result) {
  uint8_t length = getSizeThenBlock(file, buffer);

  if (length == 11 && 
      (strncmp(buffer, "NETSCAPE2.0", 11) == 0 
        || strncmp(buffer, "ANIMEXTS1.0", 11) == 0)) {
    length = getSizeThenBlock(file, buffer);
    if (length == 3) {
      uint8_t netscapeSubblockID = buffer[0] & 7;
      long loopcount = convertToUnsigned(buffer[1], buffer[2]);
      switch (netscapeSubblockID) {
        case NETSCAPE_LOOPING_EXTENSION_SUB_BLOCK_ID:
          result.loopcount = loopcount;
          break;
        default:
          std::cout << "Ignoring bad Netscape sub block\n";
          break;
      }
    }
  }

  while (getSizeThenBlock(file, buffer) > 0);
}
