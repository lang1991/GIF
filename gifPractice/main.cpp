#include <Windows.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <random>
#include <SDL.h>
#include "GIFReader.h"

const int width = 640;
const int height = 480;

int main(int, char**) {
	//SDL_Event event;
 // std::random_device rd;
 // std::mt19937 mt(rd());
 // std::uniform_int_distribution<> dist(0, 255);

	//// Init
	//if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	//	std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	//	return 1;
	//}
	//
	//// Create window
	//SDL_Window *win = SDL_CreateWindow("GIF practice", 100, 100, width, height, SDL_WINDOW_SHOWN);
	//if (win == nullptr) {
	//	std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
	//	SDL_Quit();
	//	return 1;
	//}

	//// Create renderer
	//SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//if (ren == nullptr) {
	//	SDL_DestroyWindow(win);
	//	std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
	//	SDL_Quit();
	//	return 1;
	//}

	//while (true) {
	//	if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
	//		break;

 //   SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
 //   SDL_RenderClear(ren);
 //   for (int j = 0; j < height; ++j) {
 //     SDL_SetRenderDrawColor(ren, dist(mt), dist(mt), dist(mt), 255);
 //     for (int i = 0; i < width; ++i) {
 //       SDL_RenderDrawPoint(ren, i, j);
 //     }
 //   }
 //   SDL_RenderPresent(ren);
	//}
	//SDL_DestroyRenderer(ren);
	//SDL_DestroyWindow(win);
	//SDL_Quit();
  std::string gifPath = "D:\\Code\\gifPractice\\GIFs\\gif000.gif";
  std::ifstream gifFile(gifPath);
  if (!gifFile.is_open()) {
    throw std::exception("Cannot open GIF file. Bad path!");
  }
  GIFReader gifReader(gifFile);
  GIFFile result;
  if (gifReader.parseGIF(result)) {
    result.printProperties();
  }

  std::getchar();
	return 0;
}