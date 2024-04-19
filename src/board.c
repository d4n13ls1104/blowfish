#include "board.h"
#include "bb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#define N  8
#define S -8
#define W  1
#define E -1

static bool is_legal_attack(Board *b, int pkind, int pcolor, int from, int to)
{
	if (!bb_nth(b->attacks[pkind][from], to))
		return false;

	if ((pcolor == WHITE && bb_nth(b->colors[WHITE], to)) ||
	    (pcolor == BLACK && bb_nth(b->colors[BLACK], to)))
	    return false;

	int dx = (to%BOARDW)-(from%BOARDW);
	int dy = (to/BOARDW)-(from/BOARDW);

	int xstep = (dx>0)?W:((dx<0)?E:0);
	int ystep = (dy>0)?N:((dy<0)?S:0);

	uint64_t allmask = b->colors[WHITE] | b->colors[BLACK];

	for (int i = (from+xstep+ystep); i != to; i += (xstep+ystep))
		if (bb_nth(allmask, i))
			return false;
	return true;
}

bool board_move(Board *b, int pkind, int pcolor, int from, int to)
{
	if (!is_legal_attack(b, pkind, pcolor, from, to))
		return false;

	bb_clear(&b->pieces[pkind], from);
	bb_clear(&b->colors[pcolor], from);

	bb_set(&b->pieces[pkind], to);
	bb_set(&b->colors[pcolor], to);
	return true;
}

Piece board_nthpiece(Board *b, int n)
{
	Piece p = {-1,-1,-1};

	int i;
	for (i = 0; i < NCOLORS; i++)
		if (bb_nth(b->colors[i], n))
			p.color = i;
	for (i = 0; i < NPIECES; i++)
		if (bb_nth(b->pieces[i], n))
			p.kind = i;
	p.idx = n;
	return p;
}

void board_clear(Board *b)
{
	int i;
	for (i = 0; i < NCOLORS; i++)
		b->colors[i] = 0;

	for (i = 0; i < NPIECES; i++)
		b->pieces[i] = 0;
}

static enum PieceKind c2piece(char c)
{
	switch (toupper(c)) {
	case 'P':
		return PAWN;
	case 'N':
		return KNIGHT;
	case 'B':
		return BISHOP;
	case 'R':
		return ROOK;
	case 'Q':
		return QUEEN;
	case 'K':
		return KING;
	default:
		return -1;
	}
}

static void load_masks(const char *path, uint64_t masks[BOARDW*BOARDW])
{
	FILE *fp = fopen(path, "rb");
	if (fp == NULL)
		errx(EXIT_FAILURE, "fopen: %s\n", path);

	for (int i = 0; i < BOARDW*BOARDW; i++)
		fread(&masks[i], sizeof(uint64_t), 1, fp);
	fclose(fp);
}

Board *board_create(const char *fen)
{
	Board *b = malloc(sizeof(Board));
	if (b == NULL)
		return NULL;

	board_clear(b);

	load_masks("./assets/masks/cardinal.dat", b->attacks[ROOK]);
	load_masks("./assets/masks/diagonal.dat", b->attacks[BISHOP]);
	load_masks("./assets/masks/queen.dat", b->attacks[QUEEN]);

	char c;
	size_t sq = 0;
	for (size_t i = 0; i < strlen(fen); i++) {
		if ((c = fen[i]) == '/')
			continue;
		else if (isdigit(c))
			sq += c - '0';
		else {
			bb_set(&b->colors[isupper(c)?WHITE:BLACK], sq);
			bb_set(&b->pieces[c2piece(c)], sq);
			++sq;
		}
	}

	return b;
}
