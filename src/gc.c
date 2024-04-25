#include "gc.h"
#include "board.h"

#include <err.h>
#include <SDL2/SDL_image.h>

#define TITLE "blowfish"
#define STARTFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPPRNBQKBNR"
/* #define STARTFEN "Q7/8/KRN5/8/7N/8/8/7N" */

static SDL_Window *win_init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		errx(EXIT_FAILURE, "SDL: failed init: %s\n", SDL_GetError());
	SDL_Window *w = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			RENDERW, RENDERW, SDL_WINDOW_SHOWN);
	if (w == NULL)
		errx(EXIT_FAILURE, "SDL: failed create window: %s\n", SDL_GetError());
	return w;
}

static SDL_Renderer *render_init(SDL_Window *w)
{
	SDL_Renderer *r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
	if (r == NULL)
		errx(EXIT_FAILURE, "SDL: failed create renderer: %s:\n", SDL_GetError());
	return r;
}

static SDL_Texture *load_sprites(GameContext *ctx)
{
	IMG_Init(IMG_INIT_PNG);
	SDL_Texture *tx = IMG_LoadTexture(ctx->renderer, "./assets/spritemap.png");
	return tx;
}

GameContext *gc_create()
{
	GameContext *ctx = malloc(sizeof(GameContext));
	if (ctx == NULL)
		return NULL;

	ctx->window = win_init();
	ctx->renderer = render_init(ctx->window);
	ctx->quit = false;

	ctx->mpiece = (Piece){-1,-1,-1};
	ctx->mdown = false;
	ctx->msq = 0;

	ctx->txmap = load_sprites(ctx);
	SDL_QueryTexture(ctx->txmap, NULL, NULL, &ctx->txsz.x, &ctx->txsz.y);
	ctx->txsz.x /= NPIECES;
	ctx->txsz.y /= NCOLORS;

	ctx->board = board_create(STARTFEN);

	return ctx;
}

void gc_destroy(GameContext *ctx)
{
	if (ctx == NULL)
		return;

	SDL_DestroyTexture(ctx->txmap);
	SDL_DestroyRenderer(ctx->renderer);
	SDL_DestroyWindow(ctx->window);
	IMG_Quit();
	SDL_Quit();
	free(ctx->board);
	free(ctx);
}

static void draw_pieces(GameContext *ctx)
{
	for (int i = 0; i < BOARDW*BOARDW; i++) {
		Piece p = board_nthpiece(ctx->board, i);
		if (p.kind < 0)
			continue;
		if (ctx->mdown && ctx->mpiece.idx == i)
			continue;

		if (p.kind < 0)
			continue;

		SDL_Rect src = {
			p.kind*ctx->txsz.x,
			p.color*ctx->txsz.y,
			ctx->txsz.x,ctx->txsz.y
		};

		int sq = ctx->board->orientation==WHITE?i:((BOARDW*BOARDW)-i-1);

		SDL_Rect dst = {
			SQW*(sq%BOARDW),
			SQW*(sq/BOARDW),
			SQW,SQW
		};
		SDL_RenderCopy(ctx->renderer, ctx->txmap, &src, &dst);
	}
}

static void draw_mpiece(GameContext *ctx)
{
	if (ctx->mpiece.kind < 0)
		return;

	SDL_Rect src = {
		ctx->mpiece.kind*ctx->txsz.x,
		ctx->mpiece.color*ctx->txsz.y,
		ctx->txsz.x,ctx->txsz.y
	};

	SDL_Rect dst = {
		ctx->mpos.x-(SQW/2),
		ctx->mpos.y-(SQW/2),
		SQW,SQW
	};

	SDL_RenderCopy(ctx->renderer, ctx->txmap, &src, &dst);
}

static void draw_square(GameContext *ctx, int x, int y)
{
	static const char colors[][3] = {
		{240, 217, 181},
		{181, 136, 99}
	};

	const char *c = colors[(x+y)%2!=0];
	SDL_Rect dst = {x*SQW,y*SQW,SQW,SQW};

	SDL_SetRenderDrawColor(ctx->renderer, c[0],c[1],c[2],255);
	SDL_RenderFillRect(ctx->renderer, &dst);
}

static void draw_bg(GameContext *ctx)
{
	for (int x = 0; x < BOARDW; x++)
		for (int y = 0; y < BOARDW; y++)
			draw_square(ctx, x, y);
}

void gc_render(GameContext *ctx)
{
	SDL_SetRenderDrawColor(ctx->renderer, 0,0,0,0);
	SDL_RenderClear(ctx->renderer);

	draw_bg(ctx);
	draw_pieces(ctx);
	draw_mpiece(ctx);

	SDL_RenderPresent(ctx->renderer);
}

#define XY2X(row, col, width) ((col * width + row))

void gc_poll_events(GameContext *ctx)
{
	Move m;
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			ctx->quit = true;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_q:
				ctx->quit = true;
				break;
			case SDLK_f:
				ctx->board->orientation = ctx->board->orientation==WHITE?BLACK:WHITE;
				break;
			case SDLK_r:
				board_set_fen(ctx->board, STARTFEN);
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			ctx->mpos.x = e.motion.x;
			ctx->mpos.y = e.motion.y;

			ctx->msq = XY2X((ctx->mpos.x/SQW), (ctx->mpos.y/SQW), BOARDW);
			if (ctx->board->orientation == BLACK)
				ctx->msq = (BOARDW*BOARDW)-ctx->msq-1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			ctx->mdown = (e.button.button == SDL_BUTTON_LEFT);
			ctx->mpiece = board_nthpiece(ctx->board, ctx->msq);
			break;
		case SDL_MOUSEBUTTONUP:
			m = (Move){
				.from = ctx->mpiece.idx,
				.to = ctx->msq,
				.piece = ctx->mpiece
			};
			board_move(ctx->board, m);

			ctx->mdown = !(e.button.button == SDL_BUTTON_LEFT);
			ctx->mpiece = (Piece){-1,-1,-1};
			break;
		}
	}
}
