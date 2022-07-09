#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Colour codes */
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define CEMPTY	'.'
#define CPAWN	'p'
#define CROOK	'R'
#define CKNIGHT 'N'
#define CBISHOP 'B'
#define CQUEEN	'Q'
#define CKING	'K'

#define white true
#define black false
typedef bool colour;

#define PIECE(moveset, c)                                                          \
	piece* p = malloc(sizeof(piece));                                              \
	num_pieces++;                                                                  \
	all_pieces				   = realloc(all_pieces, num_pieces * sizeof(piece*)); \
	all_pieces[num_pieces - 1] = p;                                                \
	p->x					   = x;                                                \
	p->y					   = y;                                                \
	p->col					   = col;                                              \
	p->get_moves			   = moveset;                                          \
	p->character			   = c;                                                \
	return p;

typedef struct _piece piece;
typedef struct _move  move;

typedef struct _piece {
	void (*get_moves)(move***, int*, piece*, piece***, move**, int);
	int	   x, y;
	colour col;
	char   character;
} piece;

typedef struct _move {
	int	   x1, y1, x2, y2;
	piece* p;
	void (*performance)(move*, piece*, piece***);
} move;

bool movecmp(move m1, move m2) {
	return (m1.x1 == m2.x1 && m1.x2 == m2.x2 && m1.y1 == m2.y1 && m1.y2 == m2.y2);
}

int min(int a, int b) {
	return a < b ? a : b;
}

piece*** copy_board(piece*** to_copy);
move**	 copy_moves(move** to_copyu, int count);
void	 destroy_board(piece*** to_destroy);
void	 destroy_mooves(move** to_destroy, int count);
bool	 is_on_board(int x, int y);
bool	 can_take(move* m, piece*** spaces);
bool	 can_move(move* m, piece*** spaces);
bool	 can_move_take(move* m, piece*** spaces);
bool	 has_moved(piece* p, move** moves, int count);
move*	 get_move(colour col, piece*** spaces, move** moves, int num_moves);
void	 perform_move(move* m, piece*** spaces, move*** moves, int* num_moves);
void	 check_for_check(bool*	  white_check_var,
						 bool*	  black_check_var,
						 piece*** spaces,
						 move**	  recorded_moves,
						 int	  num_recorded_moves);
bool	 check_for_mate(colour checked, piece*** spaces, move** recorded_moves, int num_recorded_moves);
void	 display_board(piece*** spaces, bool white_check, bool black_check, move** moves, int num_moves);
bool	 does_move_lose(move* m, piece*** spaces, move** moves, int num_moves);
move*	 piece_check_move(int x2, int y2, piece* self, piece*** spaces, move** recorded_moves, int num_recorded_moves);

move* piece_check_move(int x2, int y2, piece* self, piece*** spaces, move** recorded_moves, int num_recorded_moves) {
	int	   num_moves = 0;
	move** moves	 = malloc(sizeof(move*));

	self->get_moves(&moves, &num_moves, self, spaces, recorded_moves, num_recorded_moves);

	for(unsigned int i = 0; i < num_moves; i++) {
		if(moves[i]->x2 == x2 && moves[i]->y2 == y2) {
			move* m = malloc(sizeof(move));
			*m		= *(moves[i]);
			for(unsigned int i = 0; i < num_moves; i++) {
				free(moves[i]);
			}
			free(moves);
			return m;
		}
	}
	if(moves) {
		for(unsigned int i = 0; i < num_moves; i++) {
			free(moves[i]);
		}
		free(moves);
	}
	return NULL;
}

piece** all_pieces;
int		num_pieces = 0; /* Promote all pawns for a max of 48 */

piece*** copy_board(piece*** to_copy) {
	piece*** swap = malloc(sizeof(piece*) * 8 * 8);
	for(unsigned int i = 0; i < 8; i++) {
		swap[i] = malloc(sizeof(piece*) * 8);
		for(unsigned int j = 0; j < 8; j++) {
			if(to_copy[i][j]) {
				swap[i][j]	  = malloc(sizeof(piece));
				*(swap[i][j]) = *(to_copy[i][j]);
			} else {
				swap[i][j] = NULL;
			}
		}
	}
	return swap;
}

move** copy_moves(move** to_copy, int count) {
	move** swap = malloc(sizeof(move*) * count);
	for(unsigned int i = 0; i < count; i++) {
		swap[i]	   = malloc(sizeof(move));
		*(swap[i]) = *(to_copy[i]);
	}
	return swap;
}

void destroy_board(piece*** to_destroy) {
	for(unsigned int i = 0; i < 8; i++) {
		for(unsigned int j = 0; j < 8; j++) {
			if(to_destroy[i][j]) {
				free(to_destroy[i][j]);
				to_destroy[i][j] = NULL;
			}
		}
		free(to_destroy[i]);
		to_destroy[i] = NULL;
	}
	free(to_destroy);
}

void destroy_moves(move** to_destroy, int count) {
	for(unsigned int i = 0; i < count; i++) {
		free(to_destroy[i]);
		to_destroy[i] = NULL;
	}
	free(to_destroy);
}

bool is_on_board(int x, int y) {
	if(x >= 0 && x < 8) {
		if(y >= 0 && y < 8) {
			return true;
		}
	}
	return false;
}

bool can_take(move* m, piece*** spaces) {
	if(is_on_board(m->x2, m->y2)) {
		if(spaces[m->x2][m->y2]) {
			if(spaces[m->x2][m->y2]->col != m->p->col)
				return true;
		}
	}
	return false;
}

bool can_move(move* m, piece*** spaces) {
	if(is_on_board(m->x2, m->y2)) {
		if(!spaces[m->x2][m->y2]) {
			return true;
		}
	}
	return false;
}

bool can_move_take(move* m, piece*** spaces) {
	return can_move(m, spaces) || can_take(m, spaces);
}

bool has_moved(piece* p, move** moves, int count) {
	for(unsigned int i = 0; i < count; i++) {
		move* m = moves[i];
		if(m->p == p) {
			return true;
		}
	}
	return false;
}

#define move_ctr(_x, _y, func)   \
	{                            \
		m.x1		  = self->x; \
		m.y1		  = self->y; \
		m.x2		  = _x;      \
		m.y2		  = _y;      \
		m.p			  = self;    \
		m.performance = func;    \
	}

#define add_move(m)                                                                \
	{                                                                              \
		*num_moves					= *num_moves + 1;                              \
		*moves						= realloc(*moves, *num_moves * sizeof(move*)); \
		(*moves)[*num_moves - 1]	= malloc(sizeof(move));                        \
		*((*moves)[*num_moves - 1]) = m;                                           \
	}

void replace(move* m, piece* self, piece*** spaces) {
	self->x = m->x2;
	self->y = m->y2;

	spaces[m->x1][m->y1] = NULL;
	spaces[m->x2][m->y2] = self;
}

void en_passant(move* m, piece* self, piece*** spaces) {
	replace(m, self, spaces);
	if(self->col == white) {
		spaces[m->x2][m->y2 - 1] = NULL;
	} else {
		spaces[m->x2][m->y2 + 1] = NULL;
	}
}

void rook_move(move***	moves,
			   int*		num_moves,
			   piece*	self,
			   piece*** spaces,
			   move**	recorded_moves,
			   int		num_recorded_moves) {
	move m;
	for(unsigned int i = self->x + 1; i < 8; i++) {
		/* Just check every horizontal tile on this y */
		move_ctr(i, self->y, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = self->x - 1; i >= 0; i--) {
		move_ctr(i, self->y, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = self->y + 1; i < 8; i++) {
		move_ctr(self->x, i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = self->y - 1; i >= 0; i--) {
		move_ctr(self->x, i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
}

void knight_move(move***  moves,
				 int*	  num_moves,
				 piece*	  self,
				 piece*** spaces,
				 move**	  recorded_moves,
				 int	  num_recorded_moves) {
	int	 x = self->x, y = self->y;
	move m;
	move_ctr(x + 1, y + 2, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x - 1, y + 2, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x + 1, y - 2, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x - 1, y - 2, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x + 2, y - 1, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x - 2, y - 1, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x + 2, y - 1, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
	move_ctr(x - 2, y - 1, replace);
	if(can_move_take(&m, spaces))
		add_move(m);
}

void bishop_move(move***  moves,
				 int*	  num_moves,
				 piece*	  self,
				 piece*** spaces,
				 move**	  recorded_moves,
				 int	  num_recorded_moves) {
	move m;
	for(unsigned int i = 1; i < min(8 - self->x, 8 - self->y); i++) {
		move_ctr(self->x + i, self->y + i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = 1; i < min(self->x, 8 - self->y); i++) {
		move_ctr(self->x - i, self->y + i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}

		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = 1; i < min(self->x, self->y); i++) {
		move_ctr(self->x - i, self->y - i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
	for(unsigned int i = 1; i < min(8 - self->x, self->y); i++) {
		move_ctr(self->x + i, self->y - i, replace);
		if(can_move_take(&m, spaces)) {
			add_move(m);
		}
		if(!can_move(&m, spaces)) {
			break;
		}
	}
}

void queen_move(move***	 moves,
				int*	 num_moves,
				piece*	 self,
				piece*** spaces,
				move**	 recorded_moves,
				int		 num_recorded_moves) {
	bishop_move(moves, num_moves, self, spaces, recorded_moves, num_recorded_moves);
	rook_move(moves, num_moves, self, spaces, recorded_moves, num_recorded_moves);
}

void pawn_move(move***	moves,
			   int*		num_moves,
			   piece*	self,
			   piece*** spaces,
			   move**	recorded_moves,
			   int		num_recorded_moves) {
	/* Straight forward */
	move m;
	if(self->col == white) {
		move_ctr(self->x, self->y + 1, replace);
		if(can_move(&m, spaces)) {
			add_move(m);
			move_ctr(self->x, self->y + 2, replace);
			if(can_move(&m, spaces) && !has_moved(self, recorded_moves, num_recorded_moves)) {
				add_move(m);
			}
		}
	} else {
		move_ctr(self->x, self->y - 1, replace);
		if(can_move(&m, spaces)) {
			add_move(m);
			move_ctr(self->x, self->y - 2, replace);
			if(can_move(&m, spaces) && !has_moved(self, recorded_moves, num_recorded_moves)) {
				add_move(m);
			}
		}
	}

	/* Taking */
	if(self->col == white) {
		move_ctr(self->x - 1, self->y + 1, replace);
		if(can_take(&m, spaces))
			add_move(m);
		move_ctr(self->x + 1, self->y + 1, replace);
		if(can_take(&m, spaces))
			add_move(m);
	} else {
		move_ctr(self->x - 1, self->y - 1, replace);
		if(can_take(&m, spaces))
			add_move(m);
		move_ctr(self->x + 1, self->y - 1, replace);
		if(can_take(&m, spaces))
			add_move(m);
	}

	/* En passant */
	if(num_recorded_moves > 0) {
		if(self->col == white) {
			move en_passant_r;
			en_passant_r.x1 = self->x + 1;
			en_passant_r.x2 = self->x + 1;
			en_passant_r.y1 = 6;
			en_passant_r.y2 = 4;
			move en_passant_l;
			en_passant_l.x1 = self->x - 1;
			en_passant_l.x2 = self->x - 1;
			en_passant_l.y1 = 6;
			en_passant_l.y2 = 4;

			if(movecmp(*recorded_moves[num_recorded_moves - 1], en_passant_r)) {
				if(spaces[self->x + 1][4]) {
					if(spaces[self->x + 1][4]->character == CPAWN) {
						move_ctr(self->x + 1, self->y + 1, en_passant);
						add_move(m);
					}
				}
			}

			if(movecmp(*recorded_moves[num_recorded_moves - 1], en_passant_l)) {
				if(spaces[self->x - 1][4]) {
					if(spaces[self->x - 1][4]->character == CPAWN) {
						move_ctr(self->x - 1, self->y + 1, en_passant);
						add_move(m);
					}
				}
			}
		} else {
			move en_passant_r;
			en_passant_r.x1 = self->x + 1;
			en_passant_r.x2 = self->x + 1;
			en_passant_r.y1 = 1;
			en_passant_r.y2 = 3;
			move en_passant_l;
			en_passant_l.x1 = self->x - 1;
			en_passant_l.x2 = self->x - 1;
			en_passant_l.y1 = 1;
			en_passant_l.y2 = 3;

			if(movecmp(*recorded_moves[num_recorded_moves - 1], en_passant_r)) {
				if(spaces[self->x + 1][3]) {
					if(spaces[self->x + 1][3]->character == CPAWN) {
						move_ctr(self->x + 1, self->y + 1, en_passant);
						add_move(m);
					}
				}
			}

			if(movecmp(*recorded_moves[num_recorded_moves - 1], en_passant_l)) {
				if(spaces[self->x - 1][3]) {
					if(spaces[self->x - 1][3]->character == CPAWN) {
						move_ctr(self->x - 1, self->y + 1, en_passant);
						add_move(m);
					}
				}
			}
		}
	}
}

void king_move(move***	moves,
			   int*		num_moves,
			   piece*	self,
			   piece*** spaces,
			   move**	recorded_moves,
			   int		num_recorded_moves) {
}

piece* pawn(int x, int y, colour col) {
	PIECE(pawn_move, CPAWN);
}

piece* rook(int x, int y, colour col) {
	PIECE(rook_move, CROOK);
}

piece* knight(int x, int y, colour col) {
	PIECE(knight_move, CKNIGHT);
}

piece* bishop(int x, int y, colour col) {
	PIECE(bishop_move, CBISHOP);
}

piece* queen(int x, int y, colour col) {
	PIECE(queen_move, CQUEEN);
}

piece* king(int x, int y, colour col) {
	PIECE(king_move, CKING);
}

void init(piece**** spaces, move*** moves, int* num_moves) {
	/* Allocate all piece memory */
	all_pieces = malloc(sizeof(piece*) * num_pieces);

	/* Allocate all board memory */
	*spaces = malloc(sizeof(piece*) * 8 * 8);
	for(unsigned int i = 0; i < 8; i++) {
		(*spaces)[i] = malloc(sizeof(piece*) * 8);
		for(unsigned int j = 0; j < 8; j++) {
			(*spaces)[i][j] = NULL;
		}
	}
	moves	   = malloc(sizeof(move**));
	*moves	   = malloc(sizeof(move*));
	*num_moves = 0;

	/* Create and allocate pieces */
	/* Pawns */
	for(unsigned int i = 0; i < 8; i++) {
		/* 2nd rank */
		(*spaces)[i][1] = pawn(i, 1, white);
		(*spaces)[i][6] = pawn(i, 6, black);
	}

	/* Rooks */
	(*spaces)[0][0] = rook(0, 0, white);
	(*spaces)[7][0] = rook(7, 0, white);
	(*spaces)[0][7] = rook(0, 7, black);
	(*spaces)[7][7] = rook(7, 7, black);

	/* Knights */
	(*spaces)[1][0] = knight(1, 0, white);
	(*spaces)[6][0] = knight(6, 0, white);
	(*spaces)[1][7] = knight(1, 7, black);
	(*spaces)[6][7] = knight(6, 7, black);

	/* Bishops */
	(*spaces)[2][0] = bishop(2, 0, white);
	(*spaces)[5][0] = bishop(5, 0, white);
	(*spaces)[2][7] = bishop(2, 7, black);
	(*spaces)[5][7] = bishop(5, 7, black);

	/* Queens */
	(*spaces)[3][0] = queen(3, 0, white);
	(*spaces)[3][7] = queen(3, 7, black);

	/* Kings */
	(*spaces)[4][0] = king(4, 0, white);
	(*spaces)[4][7] = king(4, 7, black);
}

bool game_running() {
	return true;
}

bool does_move_lose(move* m, piece*** spaces, move** moves, int num_moves);

move* get_move(colour col, piece*** spaces, move** moves, int num_moves) {
	move* m = NULL;

	char* colour_string = col == white ? "white" : "black";

	while(!m) {
		char file1, file2;
		int	 rank1, rank2;

		printf("Please input a move for %s (eg. e2 e3), or type \'resign\' to resign: \n", colour_string);
		size_t buf_size = 16;
		char*  buffer	= malloc(sizeof(char) * buf_size); // Should not need to be large
		getline(&buffer, &buf_size, stdin);

		if(strstr(buffer, "resign") != NULL) {
			free(buffer);
			return NULL;
		}

		sscanf(buffer, "%c%d %c%d", &file1, &rank1, &file2, &rank2);

		free(buffer);

		int x1 = file1 - 'a';
		int x2 = file2 - 'a';
		int y1 = rank1 - 1;
		int y2 = rank2 - 1;

		if(is_on_board(x1, y1)) {
			if(spaces[x1][y1]) {
				if(spaces[x1][y1]->col == col) {
					m = piece_check_move(x2, y2, spaces[x1][y1], spaces, moves, num_moves);
					if(m) {
						if(!does_move_lose(m, spaces, moves, num_moves)) {
							return m;
						} else {
							printf("\nSorry, that isn't a legal move.\n");
							free(m);
							m = NULL;
						}
					} else {
						printf("\nSorry, that isn't a legal move.\n");
					}
				} else {
					printf("\nSorry, that position doesn't have a %s piece on it. (%c)\n",
						   colour_string,
						   spaces[x1][y1]->character);
				}
			} else {
				printf("\nSorry, that position doesn't have a piece on it.\n");
			}
		} else {
			printf("\nSorry, but %c%d is not on the board.\n", file1, rank1);
		}
	}
	return NULL;
}

void perform_move(move* m, piece*** spaces, move*** moves, int* num_moves) {
	/* Add to move record */
	*num_moves				 = *num_moves + 1;
	*moves					 = realloc(*moves, sizeof(move*) * *num_moves);
	(*moves)[*num_moves - 1] = m;

	/* Enact the actual move (must already be proven legal) */
	piece* affected = spaces[m->x1][m->y1];
	m->performance(m, affected, spaces);
}

void check_for_check(bool*	  white_check_var,
					 bool*	  black_check_var,
					 piece*** spaces,
					 move**	  recorded_moves,
					 int	  num_recorded_moves) {
	int white_x, white_y, black_x, black_y;

	*white_check_var = false;
	*black_check_var = false;

	for(unsigned int i = 0; i < 8; i++) {
		for(unsigned int j = 0; j < 8; j++) {
			if(spaces[i][j]) {
				if(spaces[i][j]->character == CKING) {
					if(spaces[i][j]->col == white) {
						white_x = i;
						white_y = j;
					} else {
						black_x = i;
						black_y = j;
					}
				}
			}
		}
	}

	for(unsigned int i = 0; i < 8; i++) {
		for(unsigned int j = 0; j < 8; j++) {
			if(spaces[i][j]) {
				if(spaces[i][j]->character != CKING) {
					if(spaces[i][j]->col == white) {
						if(piece_check_move(black_x, black_y, spaces[i][j], spaces, recorded_moves, num_recorded_moves)) {
							*black_check_var = true;
						}
					} else {
						if(piece_check_move(white_x, white_y, spaces[i][j], spaces, recorded_moves, num_recorded_moves)) {
							*white_check_var = true;
						}
					}
				}
			}
		}
	}
}

bool does_move_lose(move* m, piece*** spaces, move** moves, int num_moves) {
	/* Copy old board */
	piece*** swap			= copy_board(spaces);
	move**	 swap_moves		= copy_moves(moves, num_moves);
	int		 num_swap_moves = num_moves;

	/* Perform move, see if there's a check. */
	perform_move(m, swap, &swap_moves, &num_swap_moves);
	bool wc = false, bc = false;
	check_for_check(&wc, &bc, swap, swap_moves, num_swap_moves);

	destroy_board(swap);
	destroy_moves(swap_moves,
				  num_swap_moves - 1); /* We don't want to free() the last move (that's the one we just added) */

	swap		   = NULL;
	swap_moves	   = NULL;
	num_swap_moves = 0;

	if((m->p->col == black && !bc) || (m->p->col == white && !wc)) {
		return false;
	}

	return true;
}

bool check_for_mate(colour checked, piece*** spaces, move** recorded_moves, int num_recorded_moves) {
	/* Get all possible moves, and if the result is that all of those moves lose, then it's checkmate! */
	move** possible	 = malloc(sizeof(move*));
	int	   num_moves = 0;

	for(unsigned int i = 0; i < 8; i++) {
		for(unsigned int j = 0; j < 8; j++) {
			if(spaces[i][j]) {
				if(spaces[i][j]->col == checked) {
					spaces[i][j]
						->get_moves(&possible, &num_moves, spaces[i][j], spaces, recorded_moves, num_recorded_moves);
					for(unsigned int k = 0; k < num_moves; k++) {
						if(!does_move_lose(possible[k], spaces, recorded_moves, num_recorded_moves)) {
							return false;
						}
					}
					for(unsigned int k = 0; k < num_moves; k++) {
						free(possible[k]);
						num_moves = 0;
					}
				}
			}
		}
	}

	return true;
}

void display_board(piece*** spaces, bool white_check, bool black_check, move** moves, int num_moves) {
	printf("\e[1;1H\e[2J");
	for(int j = 7; j >= 0; j--) {
		printf("%d ", j + 1);
		for(int i = 0; i < 8; i++) {
			char  piece_char = CEMPTY;
			char* col		 = KNRM;
			if(spaces[i][j]) {
				piece_char = spaces[i][j]->character;
				col		   = spaces[i][j]->col == white ? KWHT : KNRM;
				if(piece_char == CKING) { /* King */
					if(spaces[i][j]->col == white && white_check) {
						col = KRED;
					} else if(spaces[i][j]->col == black && black_check) {
						col = KRED;
					}
				}
			}
			printf("%s%c " KNRM, col, piece_char);
		}
		int num_turns = num_moves / 2;
		if(num_turns > 0 && j < num_turns) {
			move* wm = moves[(num_turns - 1 - j) * 2];
			move* bm = moves[(num_turns - 1 - j) * 2 + 1];
			printf("\t%d. %c%c%d %c%c%d",
				   (num_moves / 2 - j),
				   wm->p->character,
				   wm->x2 + 'a',
				   wm->y2 + 1,
				   bm->p->character,
				   bm->x2 + 'a',
				   bm->y2 + 1);
		}
		printf("\n");
	}
	printf("\n  a b c d e f g h\n");
	printf("\n");
}

void destroy(piece*** spaces, move** moves, int num_moves) {
	for(unsigned int i = 0; i < 8; i++) {
		free(spaces[i]);
	}
	free(spaces);
	for(unsigned int i = 0; i < num_moves; i++) {
		free(moves[i]);
	}
	free(moves);
	for(unsigned int i = 0; i < num_pieces; i++) {
		free(all_pieces[i]);
	}
	free(all_pieces);
}

void black_win() {
	printf("\nBlack won!\n");
}

void white_win() {
	printf("\nWhite won!\n");
}

int main(int argc, char** argv) {
	piece*** spaces			  = NULL;
	move**	 moves_record	  = NULL;
	int		 num_moves_record = 0;
	bool	 white_check	  = false;
	bool	 black_check	  = false;

	init(&spaces, &moves_record, &num_moves_record);

	colour current_playing = white;

	while(game_running()) {
		display_board(spaces, white_check, black_check, moves_record, num_moves_record);
		move* m = get_move(current_playing, spaces, moves_record, num_moves_record);
		if(!m) {
			/* Resigns */
			if(current_playing == white) {
				black_win();
				break;
			}
			if(current_playing == black) {
				white_win();
				break;
			}
		}
		perform_move(m, spaces, &moves_record, &num_moves_record);
		check_for_check(&white_check, &black_check, spaces, moves_record, num_moves_record);
		check_for_mate(!current_playing, spaces, moves_record, num_moves_record);
		current_playing = !current_playing;
	}

	destroy(spaces, moves_record, num_moves_record);

	return 0;
}
