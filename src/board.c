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

#define WHITE_NORTH -1
#define BLACK_NORTH  1

static bool sq_occupied(Board *b, int n)
{
	uint64_t allmask = b->colors[WHITE]|b->colors[BLACK];
	return bb_nth(allmask, n);
}

static bool is_legal_attack(Board *b, Move m)
{
	enum PieceKind pkind = m.piece.kind;
	enum ColorKind pcolor = m.piece.color;

	if (pkind == PAWN) {
		int fx = (m.from%BOARDW); 
		int fy = (m.from/BOARDW);
		int tx = (m.to%BOARDW);
		int ty = (m.to/BOARDW);

		int dx = tx-fx;
		int dy = ty-fy;

		if (dx != 0)
			return false;

		int dir = pcolor==WHITE?WHITE_NORTH:BLACK_NORTH;

		bool dst_occupied = sq_occupied(b, m.to);
		bool has_moved = !((pcolor==WHITE&&fy==6) || (pcolor==BLACK&&fy==1));
		bool singlepush = (dy==dir) && !dst_occupied;
		bool doublepush = (dy==dir*2) && !has_moved && !dst_occupied;

		if (!singlepush && !doublepush)
			return false;
	} else {
		if (!bb_nth(b->attacks[pkind][m.from], m.to))
			return false;

		if ((pcolor == WHITE && bb_nth(b->colors[WHITE], m.to)) ||
				(pcolor == BLACK && bb_nth(b->colors[BLACK], m.to)))
			return false;

		if (pkind == BISHOP || pkind == ROOK || pkind == QUEEN) {
			int dx = (m.to%BOARDW)-(m.from%BOARDW);
			int dy = (m.to/BOARDW)-(m.from/BOARDW);

			int xstep = (dx>0)?W:((dx<0)?E:0);
			int ystep = (dy>0)?N:((dy<0)?S:0);

			uint64_t allmask = b->colors[WHITE] | b->colors[BLACK];

			for (int i = (m.from+xstep+ystep); i != m.to; i += (xstep+ystep))
				if (bb_nth(allmask, i))
					return false;
		}
	}
	return true;
}

static void board_clear_square(Board *b, int n)
{
	int i;
	for (i = 0; i < NCOLORS; i++)
		bb_clear(&b->colors[i], n);
	for (i = 0; i < NPIECES; i++)
		bb_clear(&b->pieces[i], n);
}

static void board_set_square(Board *b, enum PieceKind k,  enum ColorKind c, int n)
{
	board_clear_square(b, n);
	bb_set(&b->pieces[k], n);
	bb_set(&b->colors[c], n);
}

bool board_move(Board *b, Move m)
{
	if (!is_legal_attack(b, m))
		return false;

	board_clear_square(b, m.from);
	board_set_square(b, m.piece.kind, m.piece.color, m.to);
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

void board_set_fen(Board *b, const char *fen)
{
	board_clear(b);

	int sq = 0;
	for (size_t i = 0; i < strlen(fen); i++) {
		char c = fen[i];
		if (c == '/')
			continue;
		else if (isdigit(c))
			sq += c - '0';
		else
			board_set_square(b, c2piece(c), isupper(c)?WHITE:BLACK, sq++);
	}
}

Board *board_create(const char *fen)
{
	Board *b = malloc(sizeof(Board));
	if (b == NULL)
		return NULL;

	board_set_fen(b, fen);
	b->orientation = WHITE;

	load_masks("./assets/masks/cardinal.dat", b->attacks[ROOK]);
	load_masks("./assets/masks/diagonal.dat", b->attacks[BISHOP]);
	load_masks("./assets/masks/queen.dat", b->attacks[QUEEN]);
	load_masks("./assets/masks/king.dat", b->attacks[KING]);
	load_masks("./assets/masks/knight.dat", b->attacks[KNIGHT]);


	return b;
}
