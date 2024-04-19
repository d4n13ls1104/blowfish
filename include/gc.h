#ifndef _GC_H
#define _GC_H

#include "board.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

#define RENDERW 640
#define SQW (RENDERW/BOARDW)

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *txmap;
	SDL_Point txsz;
	bool quit;

	SDL_Point mpos;
	Piece mpiece;
	int  msq;
	bool mdown;

	Board *board;
} GameContext;

GameContext *gc_create();
void gc_destroy(GameContext *ctx);
void gc_poll_events(GameContext *ctx);
void gc_render(GameContext *ctx);

#endif
