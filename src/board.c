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

static enum ColorKind sqcolor(Board *b, int n)
{
	if (bb_nth(b->colors[WHITE], n))
		return WHITE;
	else if (bb_nth(b->colors[BLACK], n))
		return BLACK;
	else
		return -1;
}

static bool nthsq(Board *b, int n)
{
	uint64_t allmask = b->colors[WHITE]|b->colors[BLACK];
	return bb_nth(allmask, n);
}

static void I2XY(int i, int *x, int *y, int rwid)
{
	*x = i % rwid;
	*y = i / rwid;
}

static bool attack(Board *b, Move m)
{
	int pk = m.piece.kind;
	int pc = m.piece.color;

	int sqc = sqcolor(b, m.to);
	bool selfcap = (sqc==WHITE&&pc==WHITE)
		     ||(sqc==BLACK&&pc==BLACK);
	if (selfcap)
		return false;

	if (!bb_nth(b->attacks[pk][m.from], m.to))
		return false;

	printf("MASK: %016lX\n", b->attacks[pk][m.from]);

	int tx,ty,fx,fy;
	I2XY(m.to, &tx, &ty, BOARDW);
	I2XY(m.from, &fx, &fy, BOARDW);
	int dx = tx-fx;
	int dy = ty-fy;

	if (pk == PAWN && dx != 0)
		return true;
	else if (pk == BISHOP || pk == ROOK || pk == QUEEN) {
		int xst = (dx>0)?W:((dx<0)?E:0);
		int yst = (dy>0)?N:((dy<0)?S:0);

		for (int i = (m.from+xst+yst); i != m.to; i += (xst+yst))
			if (nthsq(b, i))
				return false;
	} 

	printf("LEGALATTACK\n");
	return true;
}

static bool pawnpush(Board *b, Move m)
{
	int pk = m.piece.kind;
	int pc = m.piece.color;
	if (pk != PAWN)
		return false;
	int fx,fy,tx,ty;
	I2XY(m.from, &fx, &fy, BOARDW);
	I2XY(m.to, &tx, &ty, BOARDW);
	int dy = ty-fy;
	int dx = tx-fx;
	if (nthsq(b, m.to) || dx != 0)
		return false;
	int pdir = pc==WHITE?-1:1;
	bool firstmove = ((pc==WHITE&&fy==6)||(pc==BLACK&&dy==1));
	bool doublepush = (dy==pdir*2)&&firstmove;
	bool singlepush = (dy==pdir);
	if (!singlepush && !doublepush)
		return false;
	printf("LEGAL PAWN PUSH\n");
	return true;
}

static bool legalmove(Board *b, Move m)
{
	if (pawnpush(b, m) || attack(b, m))
		return true;
	return false;
}

static void board_clear_square(Board *b, int n)
{
	int i;
	for (i = 0; i < NCOLORS; i++)
		bb_clear(&b->colors[i], n);
	for (i = 0; i < NPIECES; i++)
		bb_clear(&b->pieces[i], n);
}

static void board_set_square(Board *b, enum PieceKind k, enum ColorKind c, int n)
{
	board_clear_square(b, n);
	bb_set(&b->pieces[k], n);
	bb_set(&b->colors[c], n);
}

bool board_move(Board *b, Move m)
{
	if (!legalmove(b, m))
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
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
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
	load_masks("./assets/masks/pawn.dat", b->attacks[PAWN]);


	return b;
}
