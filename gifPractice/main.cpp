#ifdef _WIN32
   #include <Windows.h>
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

int main(int argc, char** argv) {
  std::string gifPath = "/Users/tianyulang/code/GIF/GIFs/gif008.gif";
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
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
      SDL_DestroyWindow(win);
      std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
      SDL_Quit();
      return 1;
    }

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
      
    size_t imageIndex = 0;
    while (true) {
      if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
        break;

      GIFImage& image = result.images[imageIndex];
      
      if (image.disposal == DISPOSAL_BACKGROUND) {
        Pixel backgroundColor(0, 0, 0, false);
        if (result.backgroundColorIndex < result.globalColormap.size() && result.backgroundColorIndex != image.transparentIndex) {
          backgroundColor = result.globalColormap[result.backgroundColorIndex];
          SDL_SetRenderDrawColor(ren, backgroundColor.red, backgroundColor.green, backgroundColor.blue, 255);
          SDL_RenderClear(ren);
        }
      }
      
      std::vector<Pixel>& colorTable = image.localColormap.empty() ? result.globalColormap : image.localColormap;
      for (int j = 0; j < image.height; ++j) {
        for (int i = 0; i < image.width; ++i) {
          uint8_t colorCode = image.data[image.width * j + i];
          colorCode = colorCode < colorTable.size() ? colorCode : 0;
          Pixel& color = colorTable[colorCode];
          if (colorCode != image.transparentIndex) {
            SDL_SetRenderDrawColor(ren, color.red, color.green, color.blue, 255);
            SDL_RenderDrawPoint(ren, i + image.left, j + image.top);
          }
        }
      }
      SDL_RenderPresent(ren);
      imageIndex = (imageIndex + 1) % result.images.size();
      SDL_Delay(image.delay * 10);
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
  }
	return 0;
}