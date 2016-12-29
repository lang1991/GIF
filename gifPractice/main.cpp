#ifdef _WIN32
   #include <SDL.h>
   #ifdef _WIN64
   #endif
#elif __APPLE__
    #include <SDL2/SDL.h>
#endif

#include <iostream>
#include <fstream>
#include <exception>
#include <string>

#include "GIFReader.h"

void drawImage(GIFFile& result, GIFImage& image, SDL_Renderer* ren) {
  std::vector<Pixel>& colorTable = image.localColormap.empty() ? result.globalColormap : image.localColormap;
  for (int j = 0; j < image.height; ++j) {
    for (int i = 0; i < image.width; ++i) {
      uint8_t colorCode = image.data[image.width * j + i];
      colorCode = colorCode < colorTable.size() ? colorCode : 0;
      Pixel& color = colorTable[colorCode];
      SDL_SetRenderDrawColor(ren, color.red, color.green, color.blue, colorCode != image.transparentIndex ? 255 : 0);
      SDL_RenderDrawPoint(ren, i, j);
    }
  }
}

int main(int argc, char** argv) {
//  std::string gifPath = "/Users/tianyulang/code/GIF/GIFs/gif000.gif";
//  std::string gifPath = "/Users/tianyulang/code/GIF/GIFs/InvalidColormapIndex/001.gif";
  std::string gifPath = "/Users/tianyulang/code/GIF/GIFs/NegativeOrZeroImageSize/001.gif";
//  std::string gifPath = "/Users/tianyulang/code/GIF/GIFs/UnableToReadExtensionBlock/003.gif";
//  std::string gifPath = "D:\\Code\\gifPractice\\GIFs\\gif003.gif";
//  std::string gifPath = "D:\\Code\\gifPractice\\GIFs\\InvalidColormapIndex\\003.gif";
//  std::string gifPath = "D:\\Code\\gifPractice\\GIFs\\NegativeOrZeroImageSize\\003.gif";
//  std::string gifPath = "D:\\Code\\gifPractice\\GIFs\\UnableToReadExtensionBlock\\003.gif";
  SDL_Event event;
  std::ifstream gifFile(gifPath, std::ios_base::in | std::ios_base::binary);
  if (!gifFile.is_open()) {
    throw std::runtime_error("Cannot open GIF file. Bad path!");
  }
  GIFReader gifReader(gifFile);
  GIFFile result;
  if (gifReader.parseGIF(result)) {
    result.printProperties();

    // Init
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
      return 1;
    }

    // Create window
    SDL_Window *win = SDL_CreateWindow("GIF practice", 100, 100, result.screenWidth, result.screenHeight, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
      std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    // Create renderer
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (ren == nullptr) {
      SDL_DestroyWindow(win);
      std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    SDL_Texture* canvas = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, result.screenWidth, result.screenHeight);
    if (canvas == nullptr) {
      SDL_DestroyRenderer(ren);
      std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    
    size_t imageIndex = 0;
    while (true) {
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        break;
      }
      
      GIFImage& image = result.images[imageIndex];
      SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, image.width, image.height);
      SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
      
      Pixel backgroundColor(255, 255, 255, false);
      if (imageIndex == 0) {
        SDL_SetRenderTarget(ren, canvas);
        if (result.backgroundColorIndex < result.globalColormap.size()) {
          backgroundColor = result.globalColormap[result.backgroundColorIndex];
        }
      } else {
        SDL_SetRenderTarget(ren, tex);
        backgroundColor.isTransparent = true;
      }
      
      SDL_SetRenderDrawColor(ren, backgroundColor.red, backgroundColor.green, backgroundColor.blue, backgroundColor.isTransparent ? 0 : 255);
      SDL_RenderClear(ren);
      drawImage(result, image, ren);
      
      SDL_Rect rect;
      rect.x = image.left;
      rect.y = image.top;
      rect.w = image.width;
      rect.h = image.height;
      SDL_SetRenderTarget(ren, canvas);
      SDL_RenderCopy(ren, tex, nullptr, &rect);
      SDL_SetRenderTarget(ren, nullptr);
      SDL_RenderCopy(ren, canvas, nullptr, nullptr);
      SDL_RenderPresent(ren);
      if (image.disposal == DISPOSAL_BACKGROUND) {
        Pixel backgroundColor(0, 0, 0, false);
        if (result.backgroundColorIndex < result.globalColormap.size()) {
          SDL_SetRenderTarget(ren, tex);
          backgroundColor = result.globalColormap[result.backgroundColorIndex];
          SDL_SetRenderDrawColor(ren, backgroundColor.red, backgroundColor.green, backgroundColor.blue, 255);
          SDL_RenderClear(ren);
          SDL_SetRenderTarget(ren, canvas);
          SDL_RenderCopy(ren, tex, nullptr, &rect);
        }
      }
      
      SDL_DestroyTexture(tex);

      imageIndex = (imageIndex + 1) % result.images.size();
      SDL_Delay(image.delay * 10);
    }
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
  }
	return 0;
}