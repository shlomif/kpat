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

#ifndef FREECELLSOLVER_H
#define FREECELLSOLVER_H

#include "abstract_fc_solve_solver.h"

class Freecell;

class FreecellSolver : public FcSolveSolver
{
public:
    FreecellSolver(const Freecell *dealer);
#if 0
    int good_automove(int o, int r);
    virtual bool isWon();
    virtual void make_move(MOVE *m);
    virtual void undo_move(MOVE *m);
    virtual void prioritize(MOVE *mp0, int n);
    virtual int getOuts();
    virtual unsigned int getClusterNumber();
    virtual int get_possible_moves(int *a, int *numout);
#endif
    virtual void translate_layout();
#if 0
    virtual void unpack_cluster( unsigned int k );
#endif
    virtual MoveHint translateMove(const MOVE &m);
    virtual void setFcSolverGameParams();
    virtual int get_cmd_line_arg_count();
    virtual const char * * get_cmd_line_args();
#if 0
    virtual void print_layout();

    int Nwpiles; /* the numbers we're actually using */
    int Ntpiles;

/* Names of the cards.  The ordering is defined in pat.h. */

    card_t O[4]; /* output piles store only the rank or NONE */
    card_t Osuit[4];


    static int Xparam[];
#endif

    const Freecell *deal;
};

#endif // FREECELLSOLVER_H
