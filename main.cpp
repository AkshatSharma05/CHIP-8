#include "chip8.h"

#include <stdio.h>
#include <chrono>
#include <thread>
#include "SDL2/SDL.h"
#include <iostream>
#include "stdint.h"

// Keypad keymap
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: chip8 <ROM file>\n");
		return 1;
	}

	Chip8 chip8 = Chip8();
	SDL_Init(SDL_INIT_EVERYTHING);

	int w = 1024;
	int h = 512;

	SDL_Window *window = NULL;

	window = SDL_CreateWindow("Chip8 Emulator", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		w, 
		h, 
		SDL_WINDOW_SHOWN);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, w, h);

	SDL_Texture *tex = SDL_CreateTexture(renderer, 
		SDL_PIXELFORMAT_ARGB8888, 
		SDL_TEXTUREACCESS_STREAMING, 
		64, 
		32);

	uint32_t pixels[2048];

	load:

	if(!chip8.load(argv[1])){
		return 2;
	}

	while(true){
	//std::cin.get();
	chip8.emulateCycle();
	SDL_Event event;

	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT) exit(0);

		if(event.type == SDL_KEYDOWN){
			if(event.key.keysym.sym == SDLK_ESCAPE){
				exit(0);
			}

			if(event.key.keysym.sym == SDLK_F1){
				goto load;
			}

			for(int i = 0; i < 16; i++){
				if(event.key.keysym.sym == keymap[i]){
					chip8.keys[i] = 1;
				}
			}

		}
		// Process keyup events
            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym == keymap[i]) {
                        chip8.keys[i] = 0;
                    }
                }
            }
	}
		if(chip8.drawFlag){
		chip8.drawFlag = false;

		// Store pixels in temporary buffer
		for(int i = 0; i < 2048; i++){
			uint8_t pixel = chip8.graphics[i];
			pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
		}

		SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(uint32_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);
		}

		// Sleep to slow down emulation speed
        std::this_thread::sleep_for(std::chrono::microseconds(1200));
	}

}
