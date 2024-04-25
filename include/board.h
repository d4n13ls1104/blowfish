#ifndef _BOARD_H
#define _BOARD_H

#include <stdint.h>
#include <stdbool.h>

#define BOARDW 8
#define NCOLORS 2
#define NPIECES 6
#define NSQUARES (BOARDW*BOARDW) 

enum ColorKind {
	BLACK = 0,
	WHITE = 1
};

typedef struct {
	uint64_t colors[NCOLORS];
	uint64_t pieces[NPIECES];
	uint64_t attacks[NPIECES][BOARDW*BOARDW];
	enum ColorKind orientation;
} Board;

enum PieceKind {
	PAWN 	= 0,
	KNIGHT 	= 1,
	BISHOP 	= 2,
	ROOK 	= 3,
	QUEEN 	= 4,
	KING 	= 5,
};

typedef struct {
	int kind;
	int color;
	int idx;
} Piece;

/* TODO: make this shit fucking make sense asshole */
typedef struct {
	Piece piece;
	int from, to;
} Move;

Board *board_create(const char *fen);
void board_destroy(Board *b);

Piece board_nthpiece(Board *b, int n);
void board_clear(Board *b);
bool board_move(Board *b, Move m);
void board_set_fen(Board *b, const char *fen);

#endif
