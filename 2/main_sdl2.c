#include <SDL2/SDL.h>
#include "snake.h"

SDL_RWops *urandom_rwops;
unsigned char fat_rng(void)
{
	return SDL_ReadU8(urandom_rwops);
}

void SDL_pt_cb(int x, int y, int set, void *data)
{
	if (set)
		SDL_SetRenderDrawColor(data, 255, 255, 255, 255);
	else
		SDL_SetRenderDrawColor(data, 0, 0, 0, 255);

	SDL_RenderDrawPoint(data, x, y);
}

int main(void)
{
	SDL_Window	*window;
	SDL_Renderer	*renderer;
	SDL_Event	event;
	struct snake_state snake;

	SDL_Init(SDL_INIT_VIDEO);
	
	urandom_rwops = SDL_RWFromFile("/dev/urandom", "r");

	window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 640, 640, 0);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(renderer, 16, 16);

	snake_init(&snake);
	while (1) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		while (SDL_PollEvent(&event))
			switch (event.type) {
			case SDL_QUIT:
				goto exit;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					snake_input(&snake, DIR_UP);
					break;
				case SDLK_DOWN:
					snake_input(&snake, DIR_DOWN);
					break;
				case SDLK_LEFT:
					snake_input(&snake, DIR_LEFT);
					break;
				case SDLK_RIGHT:
					snake_input(&snake, DIR_RIGHT);
					break;
				case SDLK_ESCAPE:
					goto exit;
				}
				break;
			}

		switch (snake_step(&snake, NULL, NULL)) {
		case SNAKE_GAMEOVER:
			printf("Game Over\nYour final size is %d pixels\n",
				snake.snake_len);
			goto exit;
		case SNAKE_WON:
			puts("Congratulations!\nYou're Winner!");
			goto exit;
		}

		snake_draw(&snake, SDL_pt_cb, renderer);
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
	}

exit:
	SDL_Delay(500);
	SDL_Quit();
	return 0;
}