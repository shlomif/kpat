/*
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

#ifndef ABSTRACT_FC_SOLVE_SOLVER_H
#define ABSTRACT_FC_SOLVE_SOLVER_H

#include "patsolve.h"

template<size_t NumberPiles>
struct FcSolveSolver : public Solver<NumberPiles>
{
public:
    FcSolveSolver();
    virtual ~FcSolveSolver();
    virtual int get_possible_moves(int *a, int *numout);
    virtual bool isWon();
    virtual void make_move(MOVE *m);
    virtual void undo_move(MOVE *m);
    virtual int getOuts();
    virtual unsigned int getClusterNumber();
    virtual void translate_layout() = 0;
    virtual void unpack_cluster( unsigned int k );
    virtual MoveHint translateMove(const MOVE &m) = 0;
    virtual SolverInterface::ExitStatus patsolve( int _max_positions = -1, bool _debug = false);
    virtual void setFcSolverGameParams() = 0;

    virtual void print_layout();

    virtual int get_cmd_line_arg_count() = 0;
    virtual const char * * get_cmd_line_args() = 0;
/* Names of the cards.  The ordering is defined in pat.h. */

    void * solver_instance;
    int solver_ret;
    // More than enough space for two decks.
    char board_as_string[4 * 13 * 2 * 4 * 3];
};

/*
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

const int CHUNKSIZE = 100;
const long int MAX_ITERS_LIMIT = 200000;

#define PRINT 0

/* These two routines make and unmake moves. */

template<size_t NumberPiles>
void FcSolveSolver<NumberPiles>::make_move(MOVE *)
{
    return;
}

template<size_t NumberPiles>
void FcSolveSolver<NumberPiles>::undo_move(MOVE *)
{
    return;
}

/* Get the possible moves from a position, and store them in Possible[]. */
template<size_t NumberPiles>
SolverInterface::ExitStatus FcSolveSolver<NumberPiles>::patsolve( int _max_positions, bool _debug )
{
    gy = 60;
    int current_iters_count;
    // max_positions = (_max_positions < 0) ? MAX_ITERS_LIMIT : _max_positions;
    // debug = _debug;

    // init();

    if (!solver_instance)
    {
        {
            solver_instance = freecell_solver_user_alloc();

            solver_ret = FCS_STATE_NOT_BEGAN_YET;

            char * error_string;
            int error_arg;
            const char * known_parameters[1] = {NULL};
            /*  A "char *" copy instead of "const char *". */

            int parse_args_ret_code = freecell_solver_user_cmd_line_parse_args(
                solver_instance,
                get_cmd_line_arg_count() ,
                get_cmd_line_args(),
                0,
                known_parameters,
                NULL,
                NULL,
                &error_string,
                &error_arg
            );

            Q_ASSERT(!parse_args_ret_code);
        }

        /*  Not needed for Simple Simon because it's already specified in
         *  freecell_solver_cmd_line_args. TODO : abstract .
         *
         *      Shlomi Fish
         *  */
        setFcSolverGameParams();

        current_iters_count = CHUNKSIZE;
        freecell_solver_user_limit_iterations(solver_instance, current_iters_count);
    }

    if (solver_instance)
    {
        bool continue_loop = true;
        while (continue_loop &&
                (   (solver_ret == FCS_STATE_NOT_BEGAN_YET)
                 || (solver_ret == FCS_STATE_SUSPEND_PROCESS))
                    &&
                 (current_iters_count < MAX_ITERS_LIMIT)
              )
        {
            current_iters_count += CHUNKSIZE;
            freecell_solver_user_limit_iterations(solver_instance, current_iters_count);

            if (solver_ret == FCS_STATE_NOT_BEGAN_YET)
            {
                solver_ret =
                    freecell_solver_user_solve_board(
                        solver_instance,
                        board_as_string
                    );
            }
            else
            {
                solver_ret = freecell_solver_user_resume_solution(solver_instance);
            }
        }
    }

    switch (solver_ret)
    {
        case FCS_STATE_IS_NOT_SOLVEABLE:
            if (solver_instance)
            {
                freecell_solver_user_free(solver_instance);
                solver_instance = NULL;
            }
            return SolverInterface::NoSolutionExists;

        case FCS_STATE_WAS_SOLVED:
            {
                if (solver_instance)
                {
                    m_winMoves.clear();
                    while (freecell_solver_user_get_moves_left(solver_instance))
                    {
                        fcs_move_t move;
                        MOVE new_move;
                        const int verdict = !freecell_solver_user_get_next_move(
                                                                solver_instance, &move)
                            ;

                        Q_ASSERT (verdict);

                        new_move.fcs = move;

                        m_winMoves.append( new_move );
                    }

                    freecell_solver_user_free(solver_instance);
                    solver_instance = NULL;
                }
                return SolverInterface::SolutionExists;
            }

        case FCS_STATE_SUSPEND_PROCESS:
            return SolverInterface::UnableToDetermineSolvability;

        default:
            if (solver_instance)
            {
                freecell_solver_user_free(solver_instance);
                solver_instance = NULL;
            }
            return SolverInterface::NoSolutionExists;
    }
}

/* Get the possible moves from a position, and store them in Possible[]. */

template<size_t NumberPiles>
int FcSolveSolver<NumberPiles>::get_possible_moves(int *, int *)
{
    return 0;
}

template<size_t NumberPiles>
bool FcSolveSolver<NumberPiles>::isWon()
{
    return true;
}

template<size_t NumberPiles>
int FcSolveSolver<NumberPiles>::getOuts()
{
    return 0;
}

template<size_t NumberPiles>
FcSolveSolver<NumberPiles>::FcSolveSolver()
    : Solver()
    , solver_instance(NULL)
    , solver_ret(FCS_STATE_NOT_BEGAN_YET)
    , board_as_string("")
{
}

template<size_t NumberPiles>
unsigned int FcSolveSolver<NumberPiles>::getClusterNumber()
{
    return 0;
}

template<size_t NumberPiles>
void FcSolveSolver<NumberPiles>::print_layout()
{
#if 0
    int i, w, o;

    fprintf(stderr, "print-layout-begin\n");
    for (w = 0; w < 10; ++w) {
        Q_ASSERT( Wp[w] == &W[w][Wlen[w]-1] );
        fprintf( stderr, "Play%d: ", w );
        for (i = 0; i < Wlen[w]; ++i) {
            printcard(W[w][i], stderr);
        }
        fputc('\n', stderr);
    }
    fprintf( stderr, "Off: " );
    for (o = 0; o < 4; ++o) {
        if ( O[o] != -1 )
            printcard( O[o] + PS_KING, stderr);
    }
    fprintf(stderr, "\nprint-layout-end\n");
#endif
}

template<size_t NumberPiles>
void FcSolveSolver<NumberPiles>::unpack_cluster( unsigned int)
{
    return;
}

template<size_t NumberPiles>
FcSolveSolver<NumberPiles>::~FcSolveSolver()
{
    if (solver_instance)
    {
        freecell_solver_user_free(solver_instance);
        solver_instance = NULL;
    }
}

#endif // ABSTRACT_FC_SOLVE_SOLVER_H
