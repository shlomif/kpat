/*
 * Copyright (C) 1998-2002 Tom Holroyd <tomh@kurage.nimh.nih.gov>
 * Copyright (C) 2006-2009 Stephan Kulow <coolo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "freecell-solver/fcs_user.h"
#include "freecell-solver/fcs_cl.h"

#include "freecellsolver.h"

#include "../freecell.h"

const int CHUNKSIZE = 100;
const long int MAX_ITERS_LIMIT = 200000;

/* The following macro implements
	(Same_suit ? (suit(a) == suit(b)) : (color(a) != color(b)))
*/
#define suitable(a, b) ((((a) ^ (b)) & PS_COLOR) == PS_COLOR)

/* Statistics. */

#if 0
int FreecellSolver::Xparam[] = { 4, 1, 8, -1, 7, 11, 4, 2, 2, 1, 2 };
#endif

/* These two routines make and unmake moves. */

#if 0
void FreecellSolver::make_move(MOVE *m)
{
	int from, to;
	card_t card;

	from = m->from;
	to = m->to;

	/* Remove from pile. */

        card = *Wp[from]--;
        Wlen[from]--;
        hashpile(from);

	/* Add to pile. */

	if (m->totype == O_Type) {
            O[to]++;
        } else {
            *++Wp[to] = card;
            Wlen[to]++;
            hashpile(to);
	}
}

void FreecellSolver::undo_move(MOVE *m)
{
	int from, to;
	card_t card;

	from = m->from;
	to = m->to;

	/* Remove from 'to' pile. */

	if (m->totype == O_Type) {
            card = O[to] + Osuit[to];
            O[to]--;
        } else {
            card = *Wp[to]--;
            Wlen[to]--;
            hashpile(to);
	}

	/* Add to 'from' pile. */
        *++Wp[from] = card;
        Wlen[from]++;
        hashpile(from);
}
#endif

#if 0
/* Move prioritization.  Given legal, pruned moves, there are still some
that are a waste of time, especially in the endgame where there are lots of
possible moves, but few productive ones.  Note that we also prioritize
positions when they are added to the queue. */

#define NNEED 8

void FreecellSolver::prioritize(MOVE *mp0, int n)
{
	int i, j, s, w, pile[NNEED], npile;
	card_t card, need[4];
	MOVE *mp;

	/* There are 4 cards that we "need": the next cards to go out.  We
	give higher priority to the moves that remove cards from the piles
	containing these cards. */

	for (i = 0; i < NNEED; ++i) {
		pile[i] = -1;
	}
	npile = 0;

	for (s = 0; s < 4; ++s) {
		need[s] = NONE;
		if (O[s] == NONE) {
			need[s] = Osuit[s] + PS_ACE;
		} else if (O[s] != PS_KING) {
			need[s] = Osuit[s] + O[s] + 1;
		}
	}

	/* Locate the needed cards.  There's room for optimization here,
	like maybe an array that keeps track of every card; if maintaining
	such an array is not too expensive. */

	for (w = 0; w < Nwpiles; ++w) {
		j = Wlen[w];
		for (i = 0; i < j; ++i) {
			card = W[w][i];
			s = SUIT(card);

			/* Save the locations of the piles containing
			not only the card we need next, but the card
			after that as well. */

			if (need[s] != NONE &&
			    (card == need[s] || card == need[s] + 1)) {
				pile[npile++] = w;
				if (npile == NNEED) {
					break;
				}
			}
		}
		if (npile == NNEED) {
			break;
		}
	}

	/* Now if any of the moves remove a card from any of the piles
	listed in pile[], bump their priority.  Likewise, if a move
	covers a card we need, decrease its priority.  These priority
	increments and decrements were determined empirically. */

	for (i = 0, mp = mp0; i < n; ++i, ++mp) {
		if (mp->card_index != -1) {
			w = mp->from;
			for (j = 0; j < npile; ++j) {
				if (w == pile[j]) {
					mp->pri += Xparam[0];
				}
			}
			if (Wlen[w] > 1) {
				card = W[w][Wlen[w] - 2];
				for (s = 0; s < 4; ++s) {
					if (card == need[s]) {
						mp->pri += Xparam[1];
						break;
					}
				}
			}
			if (mp->totype == W_Type) {
				for (j = 0; j < npile; ++j) {
					if (mp->to == pile[j]) {
						mp->pri -= Xparam[2];
					}
				}
			}
		}
	}
}
#endif

/* Automove logic.  Freecell games must avoid certain types of automoves. */

#if 0
int FreecellSolver::good_automove(int o, int r)
{
	int i;

	if (r <= 2) {
		return true;
	}

	/* Check the Out piles of opposite color. */

	for (i = 1 - (o & 1); i < 4; i += 2) {
		if (O[i] < r - 1) {

#if 1   /* Raymond's Rule */
			/* Not all the N-1's of opposite color are out
			yet.  We can still make an automove if either
			both N-2's are out or the other same color N-3
			is out (Raymond's rule).  Note the re-use of
			the loop variable i.  We return here and never
			make it back to the outer loop. */

			for (i = 1 - (o & 1); i < 4; i += 2) {
				if (O[i] < r - 2) {
					return false;
				}
			}
			if (O[(o + 2) & 3] < r - 3) {
				return false;
			}

			return true;
#else   /* Horne's Rule */
			return false;
#endif
		}
	}

	return true;
}
#endif

#define CMD_LINE_ARGS_NUM 280

static const char * freecell_solver_cmd_line_args[CMD_LINE_ARGS_NUM] =
{
"--method", "soft-dfs", "-to", "0123456789", "-step",
"500", "--st-name", "1", "-nst", "--method",
"soft-dfs", "-to", "0123467", "-step", "500",
"--st-name", "2", "-nst", "--method", "random-dfs",
"-seed", "2", "-to", "0[01][23456789]", "-step",
"500", "--st-name", "3", "-nst", "--method",
"random-dfs", "-seed", "1", "-to", "0[0123456789]",
"-step", "500", "--st-name", "4", "-nst", "--method",
"random-dfs", "-seed", "3", "-to", "0[01][23467]",
"-step", "500", "--st-name", "5", "-nst", "--method",
"random-dfs", "-seed", "4", "-to", "0[0123467]",
"-step", "500", "--st-name", "9", "-nst", "--method",
"random-dfs", "-to", "[01][23456789]", "-seed", "8",
"-step", "500", "--st-name", "10", "-nst",
"--method", "random-dfs", "-to", "[01][23456789]",
"-seed", "268", "-step", "500", "--st-name", "12",
"-nst", "--method", "a-star", "-asw",
"0.2,0.3,0.5,0,0", "-step", "500", "--st-name", "16",
"-nst", "--method", "a-star", "-to", "0123467",
"-asw", "0.5,0,0.3,0,0", "-step", "500", "--st-name",
"18", "-nst", "--method", "soft-dfs", "-to",
"0126394875", "-step", "500", "--st-name", "19",
"--prelude",
"350@2,350@5,350@9,350@12,350@2,350@10,350@3,350@9,350@5,350@18,350@2,350@5,350@4,350@10,350@4,350@12,1050@9,700@18,350@10,350@5,350@2,350@10,1050@16,350@2,700@4,350@10,1050@2,1400@3,350@18,1750@5,350@16,350@18,700@4,1050@12,2450@5,1400@18,1050@2,1400@10,6300@1,4900@12,8050@18",
"-ni", "--method", "soft-dfs", "-to", "01ABCDE",
"-step", "500", "--st-name", "0", "-nst", "--method",
"random-dfs", "-to", "[01][ABCDE]", "-seed", "1",
"-step", "500", "--st-name", "1", "-nst", "--method",
"random-dfs", "-to", "[01][ABCDE]", "-seed", "2",
"-step", "500", "--st-name", "2", "-nst", "--method",
"random-dfs", "-to", "[01][ABCDE]", "-seed", "3",
"-step", "500", "--st-name", "3", "-nst", "--method",
"random-dfs", "-to", "01[ABCDE]", "-seed", "268",
"-step", "500", "--st-name", "4", "-nst", "--method",
"a-star", "-to", "01ABCDE", "-step", "500",
"--st-name", "5", "-nst", "--method", "a-star",
"-to", "01ABCDE", "-asw", "0.2,0.3,0.5,0,0", "-step",
"500", "--st-name", "6", "-nst", "--method",
"a-star", "-to", "01ABCDE", "-asw", "0.5,0,0.5,0,0",
"-step", "500", "--st-name", "7", "-nst", "--method",
"random-dfs", "-to", "[01][ABD][CE]", "-seed", "1900",
"-step", "500", "--st-name", "8", "-nst", "--method",
"random-dfs", "-to", "[01][ABCDE]", "-seed", "192",
"-step", "500", "--st-name", "9", "-nst", "--method",
"random-dfs", "-to", "[01ABCDE]", "-seed", "1977",
"-step", "500", "--st-name", "10", "-nst",
"--method", "random-dfs", "-to", "[01ABCDE]", "-seed",
"24", "-step", "500", "--st-name", "11", "-nst",
"--method", "soft-dfs", "-to", "01ABDCE", "-step",
"500", "--st-name", "12", "-nst", "--method",
"soft-dfs", "-to", "ABC01DE", "-step", "500",
"--st-name", "13", "-nst", "--method", "soft-dfs",
"-to", "01EABCD", "-step", "500", "--st-name", "14",
"-nst", "--method", "soft-dfs", "-to", "01BDAEC",
"-step", "500", "--st-name", "15", "--prelude",
"1000@0,1000@3,1000@0,1000@9,1000@4,1000@9,1000@3,1000@4,2000@2,1000@0,2000@1,1000@14,2000@11,1000@14,1000@3,1000@11,1000@2,1000@0,2000@4,2000@10,1000@0,1000@2,2000@10,1000@0,2000@11,2000@1,1000@10,1000@2,1000@10,2000@0,1000@9,1000@1,1000@2,1000@14,3000@8,1000@2,1000@14,1000@1,1000@10,3000@6,2000@4,1000@2,2000@0,1000@2,1000@11,2000@6,1000@0,5000@1,1000@0,2000@1,1000@2,3000@3,1000@10,1000@14,2000@6,1000@0,1000@2,2000@11,6000@8,8000@9,3000@1,2000@10,2000@14,3000@15,4000@0,1000@8,1000@10,1000@14,7000@0,14000@2,6000@3,7000@4,1000@8,4000@9,2000@15,2000@6,4000@3,2000@4,3000@15,2000@0,6000@1,2000@4,4000@6,4000@9,4000@14,7000@8,3000@0,3000@1,5000@2,3000@3,4000@9,8000@10,9000@3,5000@8,7000@11,11000@12,12000@0,8000@3,11000@9,9000@15,7000@2,12000@8,16000@5,8000@13,18000@0,9000@15,12000@10,16000@0,14000@3,16000@9,26000@4,23000@3,42000@6,22000@8,27000@10,38000@7,41000@0,42000@3,84000@13,61000@15,159000@5,90000@9"
};

int FreecellSolver::get_cmd_line_arg_count()
{
    return CMD_LINE_ARGS_NUM;
}

const char * * FreecellSolver::get_cmd_line_args()
{
    return freecell_solver_cmd_line_args;
}


void FreecellSolver::setFcSolverGameParams()
{
    /*
     * I'm using the more standard interface instead of the depracated
     * user_set_game one. I'd like that each function will have its
     * own dedicated purpose.
     *
     *     Shlomi Fish
     * */
    freecell_solver_user_set_num_freecells(solver_instance,4);
    freecell_solver_user_set_num_stacks(solver_instance,8);
    freecell_solver_user_set_num_decks(solver_instance,1);
    freecell_solver_user_set_sequences_are_built_by_type(solver_instance, FCS_SEQ_BUILT_BY_ALTERNATE_COLOR);
    freecell_solver_user_set_sequence_move(solver_instance, 0);
    freecell_solver_user_set_empty_stacks_filled_by(solver_instance, FCS_ES_FILLED_BY_ANY_CARD);
}
#if 0
void FreecellSolver::unpack_cluster( unsigned int k )
{
    /* Get the Out cells from the cluster number. */
    O[0] = k & 0xF;
    k >>= 4;
    O[1] = k & 0xF;
    k >>= 4;
    O[2] = k & 0xF;
    k >>= 4;
    O[3] = k & 0xF;
}
#endif


FreecellSolver::FreecellSolver(const Freecell *dealer)
    : FcSolveSolver()
{
#if 0
    Osuit[0] = PS_DIAMOND;
    Osuit[1] = PS_CLUB;
    Osuit[2] = PS_HEART;
    Osuit[3] = PS_SPADE;

    Nwpiles = 8;
    Ntpiles = 4;

    setNumberPiles( Nwpiles + Ntpiles );
#endif

    deal = dealer;
}

/* Read a layout file.  Format is one pile per line, bottom to top (visible
card).  Temp cells and Out on the last two lines, if any. */

#if 0
void FreecellSolver::translate_layout()
{
	/* Read the workspace. */

	int total = 0;
	for ( int w = 0; w < 8; ++w ) {
		int i = translate_pile(deal->store[w], W[w], 52);
		Wp[w] = &W[w][i - 1];
		Wlen[w] = i;
		total += i;
		if (w == Nwpiles) {
			break;
		}
	}

	/* Temp cells may have some cards too. */

	for (int w = 0; w < Ntpiles; ++w)
        {
            int i = translate_pile( deal->freecell[w], W[w+Nwpiles], 52 );
            Wp[w+Nwpiles] = &W[w+Nwpiles][i-1];
            Wlen[w+Nwpiles] = i;
            total += i;
	}

	/* Output piles, if any. */
	for (int i = 0; i < 4; ++i) {
		O[i] = NONE;
	}
	if (total != 52) {
            for (int i = 0; i < 4; ++i) {
                KCard *c = deal->target[i]->topCard();
                if (c) {
                    O[translateSuit( c->suit() ) >> 4] = c->rank();
                    total += c->rank();
                }
            }
	}
}
#endif

MoveHint FreecellSolver::translateMove( const MOVE &m )
{
    fcs_move_t move = m.fcs;
    int cards = fcs_move_get_num_cards_in_seq(move);
    PatPile *from = 0;
    PatPile *to = 0;

    switch(fcs_move_get_type(move))
    {
        case FCS_MOVE_TYPE_STACK_TO_STACK:
            from = deal->store[fcs_move_get_src_stack(move)];
            to = deal->store[fcs_move_get_dest_stack(move)];
            break;

        case FCS_MOVE_TYPE_FREECELL_TO_STACK:
            from = deal->freecell[fcs_move_get_src_freecell(move)];
            to = deal->store[fcs_move_get_dest_stack(move)];
            cards = 1;
            break;

        case FCS_MOVE_TYPE_FREECELL_TO_FREECELL:
            from = deal->freecell[fcs_move_get_src_freecell(move)];
            to = deal->freecell[fcs_move_get_dest_freecell(move)];
            cards = 1;
            break;

        case FCS_MOVE_TYPE_STACK_TO_FREECELL:
            from = deal->store[fcs_move_get_src_stack(move)];
            to = deal->freecell[fcs_move_get_dest_freecell(move)];
            cards = 1;
            break;

        case FCS_MOVE_TYPE_STACK_TO_FOUNDATION:
            from = deal->store[fcs_move_get_src_stack(move)];
            cards = 1;
            to = 0;
            break;

        case FCS_MOVE_TYPE_FREECELL_TO_FOUNDATION:
            from = deal->freecell[fcs_move_get_src_freecell(move)];
            cards = 1;
            to = 0;
    }
    Q_ASSERT(from);
    Q_ASSERT(cards <= from->cards().count());
    Q_ASSERT(to || cards == 1);
    KCard *card = from->cards()[from->cards().count() - cards];

    if (!to)
    {
        PatPile *target = 0;
        PatPile *empty = 0;
        for (int i = 0; i < 4; ++i) {
            KCard *c = deal->target[i]->topCard();
            if (c) {
                if ( c->suit() == card->suit() )
                {
                    target = deal->target[i];
                    break;
                }
            } else if ( !empty )
                empty = deal->target[i];
        }
        to = target ? target : empty;
    }
    Q_ASSERT(to);

    return MoveHint(card, to, 0);

#if 0
    // this is tricky as we need to want to build the "meta moves"

    PatPile *frompile = nullptr;
    if ( m.from < 8 )
        frompile = deal->store[m.from];
    else
        frompile = deal->freecell[m.from-8];
    KCard *card = frompile->at( frompile->count() - m.card_index - 1);

    if ( m.totype == O_Type )
    {
        PatPile *target = nullptr;
        PatPile *empty = nullptr;
        for (int i = 0; i < 4; ++i) {
            KCard *c = deal->target[i]->topCard();
            if (c) {
                if ( c->suit() == card->suit() )
                {
                    target = deal->target[i];
                    break;
                }
            } else if ( !empty )
                empty = deal->target[i];
        }
        if ( !target )
            target = empty;
        return MoveHint( card, target, m.pri );
    } else {
        PatPile *target = nullptr;
        if ( m.to < 8 )
            target = deal->store[m.to];
        else
            target = deal->freecell[m.to-8];

        return MoveHint( card, target, m.pri );
    }
#endif
}

void FreecellSolver::translate_layout()
{
    strcpy(board_as_string, deal->solverFormat().toLatin1());

    if (solver_instance)
    {
        freecell_solver_user_recycle(solver_instance);
        solver_ret = FCS_STATE_NOT_BEGAN_YET;
    }
#if 0
    /* Read the workspace. */
    int total = 0;

    for ( int w = 0; w < 10; ++w ) {
        int i = translate_pile(deal->store[w], W[w], 52);
        Wp[w] = &W[w][i - 1];
        Wlen[w] = i;
        total += i;
    }

    for (int i = 0; i < 4; ++i) {
        O[i] = -1;
        KCard *c = deal->target[i]->top();
        if (c) {
            total += 13;
            O[i] = translateSuit( c->suit() );
        }
    }
#endif
}



#if 0
unsigned int FreecellSolver::getClusterNumber()
{
    int i = O[0] + (O[1] << 4);
    unsigned int k = i;
    i = O[2] + (O[3] << 4);
    k |= i << 8;
    return k;
}
#endif

#if 0
void FreecellSolver::print_layout()
{
       int i, t, w, o;

       fprintf(stderr, "print-layout-begin\n");
       for (w = 0; w < Nwpiles; ++w) {
	 fprintf(stderr, "W-Pile%d: ", w);
               for (i = 0; i < Wlen[w]; ++i) {
                       printcard(W[w][i], stderr);
               }
               fputc('\n', stderr);
       }
       for (t = 0; t < Ntpiles; ++t) {
	 fprintf(stderr, "T-Pile%d: ", t+Nwpiles);
           printcard(W[t+Nwpiles][Wlen[t+Nwpiles]], stderr);
       }
       fprintf( stderr, "\n" );
       for (o = 0; o < 4; ++o) {
               printcard(O[o] + Osuit[o], stderr);
       }
       fprintf(stderr, "\nprint-layout-end\n");
}
#endif
