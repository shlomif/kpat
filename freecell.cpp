/*
 * Copyright (C) 1997 Rodolfo Borges <barrett@labma.ufrj.br>
 * Copyright (C) 1998-2009 Stephan Kulow <coolo@kde.org>
 * Copyright (C) 2010 Parker Coates <coates@kde.org>
 *
 * License of original code:
 * -------------------------------------------------------------------------
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 *   This file is provided AS IS with no warranties of any kind.  The author
 *   shall have no liability with respect to the infringement of copyrights,
 *   trade secrets or any patents by this file or any part thereof.  In no
 *   event will the author be liable for any lost revenue or profits or
 *   other special, indirect and consequential damages.
 * -------------------------------------------------------------------------
 *
 * License of modifications/additions made after 2009-01-01:
 * -------------------------------------------------------------------------
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of 
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------------
 */

#if 0
// TODO : Remove these includes later.
#include "freecell-solver/fcs_user.h"
#include "freecell-solver/fcs_cl.h"
#endif

#include "freecell.h"

#include "dealerinfo.h"
#include "pileutils.h"
#include "speeds.h"
#include "patsolve/freecellsolver.h"

#include <QDebug>
#include <KLocalizedString>


Freecell::Freecell( const DealerInfo * di )
  : DealerScene( di )
{
}


void Freecell::initialize()
{
    setDeckContents();

    const qreal topRowDist = 1.08;
    const qreal bottomRowDist = 1.13;
    const qreal targetOffsetDist = ( 7 * bottomRowDist + 1 ) - ( 3 * topRowDist + 1 );

    for ( int i = 0; i < 4; ++i )
    {
        freecell[i] = new PatPile ( this, 1 + 8 + i, QStringLiteral( "freecell%1" ).arg( i ) );
        freecell[i]->setPileRole(PatPile::Cell);
        freecell[i]->setLayoutPos(topRowDist * i, 0);
        freecell[i]->setKeyboardSelectHint( KCardPile::AutoFocusTop );
        freecell[i]->setKeyboardDropHint( KCardPile::AutoFocusTop );
    }

    for ( int i = 0; i < 8; ++i )
    {
        store[i] = new PatPile( this, 1 + i, QStringLiteral( "store%1" ).arg( i ) );
        store[i]->setPileRole(PatPile::Tableau);
        store[i]->setLayoutPos( bottomRowDist * i, 1.3 );
        store[i]->setBottomPadding( 2.5 );
        store[i]->setHeightPolicy( KCardPile::GrowDown );
        store[i]->setKeyboardSelectHint( KCardPile::AutoFocusDeepestRemovable );
        store[i]->setKeyboardDropHint( KCardPile::AutoFocusTop );
    }

    for ( int i = 0; i < 4; ++i )
    {
        target[i] = new PatPile(this, 1 + 8 + 4 + i, QStringLiteral( "target%1" ).arg( i ));
        target[i]->setPileRole(PatPile::Foundation);
        target[i]->setLayoutPos(targetOffsetDist + topRowDist * i, 0);
        target[i]->setSpread(0, 0);
        target[i]->setKeyboardSelectHint( KCardPile::NeverFocus );
        target[i]->setKeyboardDropHint( KCardPile::ForceFocusTop );
    }

    setActions(DealerScene::Demo | DealerScene::Hint);
    setSolver( new FreecellSolver( this ) );
    setNeededFutureMoves( 4 ); // reserve some
}


void Freecell::restart( const QList<KCard*> & cards )
{
    QList<KCard*> cardList = cards;

    int column = 0;
    while ( !cardList.isEmpty() )
    {
        addCardForDeal( store[column], cardList.takeLast(), true, store[0]->pos() );
        column = (column + 1) % 8;
    }

    startDealAnimation();
}

int getDeck(KCardDeck::Suit suit)
{
    switch (suit) {
        case KCardDeck::Hearts:
            return 0;
        case KCardDeck::Spades:
            return 1;
        case KCardDeck::Diamonds:
            return 2;
        case KCardDeck::Clubs:
            return 3;
        default:
            exit(-1);
    }
    return 0;
}

#if 0
void Freecell::findSolution()
{
    kDebug(11111) << "findSolution\n";

    QString output = solverFormat();
    kDebug(11111) << output << endl;

    resumeSolution();
}
#endif

#if 0
void Freecell::resumeSolution()
{
#if 0
    if (!solver_instance)
        return;

#if 0
    emit gameInfo(i18n("%1 tries - depth %2",
                   freecell_solver_user_get_num_times(solver_instance),
                   freecell_solver_user_get_current_depth(solver_instance)));
#endif

    if (solver_ret == FCS_STATE_WAS_SOLVED)
    {
#if 0
        emit gameInfo(i18n("solved after %1 tries",
                      freecell_solver_user_get_num_times(
                          solver_instance)));
#endif
        kDebug(11111) << "solved\n";
        DealerScene::demo();
        return;
    }
    if (solver_ret == FCS_STATE_IS_NOT_SOLVEABLE) {
#if 0
	int moves = freecell_solver_user_get_num_times(solver_instance);
#endif
        freeSolution();
#if 0
        emit gameInfo(i18n("unsolved after %1 moves",
                       moves));
#endif
        stopDemo();
        return;
    }

    unsigned int max_iters = freecell_solver_user_get_limit_iterations(
        solver_instance) + CHUNKSIZE;
    freecell_solver_user_limit_iterations(solver_instance,
                                          max_iters);

    if (max_iters > 120000) {
        solver_ret = FCS_STATE_IS_NOT_SOLVEABLE;
        resumeSolution();
        return;
    }

    solver_ret = freecell_solver_user_resume_solution(solver_instance);
    QTimer::singleShot(0, this, SLOT(resumeSolution()));
#endif
}
#endif

QString Freecell::solverFormat() const
{
    QString output;
    QString tmp;
    for (int i = 0; i < 4 ; i++) {
        if (target[i]->isEmpty())
            continue;
        tmp += suitToString(target[i]->topCard()->suit()) + '-' + rankToString(target[i]->topCard()->rank()) + ' ';
    }
    if (!tmp.isEmpty())
        output += QString::fromLatin1("Foundations: %1\n").arg(tmp);

    tmp.truncate(0);
    for (int i = 0; i < 4 ; i++) {
        if (freecell[i]->isEmpty())
            tmp += "- ";
        else
            tmp += rankToString(freecell[i]->topCard()->rank()) + suitToString(freecell[i]->topCard()->suit()) + ' ';
    }
    if (!tmp.isEmpty())
    {
        QString a = QString::fromLatin1("Freecells: %1\n");
        kDebug(11111) << "a is {{{{" << a << "}}}}" << endl;
        output += a.arg(tmp);
    }

    for (int i = 0; i < 8 ; i++)
    {
        QList<KCard*> cards = store[i]->cards();
        for (QList<KCard*>::ConstIterator it = cards.begin(); it != cards.end(); ++it)
            output += rankToString((*it)->rank()) + suitToString((*it)->suit()) + ' ';
        output += '\n';
    }
    {
        kDebug(11111) << "output is {{{{" << output << "}}}}" << endl;
    }
    return output;
}

#if 0
//  Idea stolen from klondike.cpp
//
//  This function returns true when it is certain that the card t is no longer
//  needed on any of the play piles.
//
//  To determine wether a card is no longer needed on any of the play piles we
//  obviously must know what a card can be used for there. According to the
//  rules a card can be used to store another card with 1 less unit of value
//  and opposite color. This is the only thing that a card can be used for
//  there. Therefore the cards with lowest value (1) are useless there (base
//  case). The other cards each have 2 cards that can be stored on them, let us
//  call those 2 cards *depending cards*.
//
//  The object of the game is to put all cards on the target piles. Therefore
//  cards that are no longer needed on any of the play piles should be put on
//  the target piles if possible. Cards on the target piles can not be moved
//  and they can not store any of its depending cards. Let us call this that
//  the cards on the target piles are *out of play*.
//
//  The simple and obvious rule is:
//    A card is no longer needed when both of its depending cards are out of
//    play.
//
//  More complex:
//    Assume card t is red.  Now, if the lowest unplayed black card is
//    t.value()-2, then t may be needed to hold that black t.value()-1 card.
//    If the lowest unplayed black card is t.value()-1, it will be playable
//    to the target, unless it is needed for a red card of value t.value()-2.
//
//  So, t is not needed if the lowest unplayed red card is t.value()-2 and the
//  lowest unplayed black card is t.value()-1, OR if the lowest unplayed black
//  card is t.value().  So, no recursion needed - we did it ahead of time.

bool Freecell::noLongerNeeded(const Card & t)
{

    if (t.rank() <= Card::Two) return true; //  Base case.

    bool cardIsRed = t.isRed();

    int numSame = 0, numDiff = 0;
    Card::Rank lowSame = Card::King, lowDiff = Card::King;
    for (PileList::Iterator it = target.begin(); it != target.end(); ++it)
    {
        if ((*it)->isEmpty())
            continue;
        if ((*it)->top()->isRed() == cardIsRed) {
            numSame++;
            if ((*it)->top()->rank() < lowSame)
                lowSame = static_cast<Card::Rank>((*it)->top()->rank()+1);
        } else {
            numDiff++;
            if ((*it)->top()->rank() < lowDiff)
                lowDiff = static_cast<Card::Rank>((*it)->top()->rank()+1);
        }
    }
    if (numSame < target.count()/2) lowSame = Card::Ace;
    if (numDiff < target.count()/2) lowDiff = Card::Ace;

    return (lowDiff >= t.rank() ||
        (lowDiff >= t.rank()-1 && lowSame >= t.rank()-2));
}
#endif

#if 0
MoveHint *Freecell::chooseHint()
{
    return NULL;
#if 0
    if (solver_instance && freecell_solver_user_get_moves_left(solver_instance)) {
#if 0
        emit gameInfo(i18n("%1 moves before finish", freecell_solver_user_get_moves_left(solver_instance)));
#endif
        fcs_move_t move;
        if (!freecell_solver_user_get_next_move(solver_instance, &move)) {
            MoveHint *mh = translateMove(&move);
            oldmoves.append(mh);
            return mh;
        } else
            return 0;
    } else
        return DealerScene::chooseHint();
#endif
}
#endif


void Freecell::cardsDroppedOnPile( const QList<KCard*> & cards, KCardPile * pile )
{
    if ( cards.size() <= 1 )
    {
        DealerScene::moveCardsToPile( cards, pile, DURATION_MOVE );
        return;
    }

    QList<KCardPile*> freeCells;
    for ( int i = 0; i < 4; ++i )
        if ( freecell[i]->isEmpty() )
            freeCells << freecell[i];

    QList<KCardPile*> freeStores;
    for ( int i = 0; i < 8; ++i )
        if ( store[i]->isEmpty() && store[i] != pile )
            freeStores << store[i];

    multiStepMove( cards, pile, freeStores, freeCells, DURATION_MOVE );
}


bool Freecell::tryAutomaticMove(KCard *c)
{
    // target move
    if (DealerScene::tryAutomaticMove(c))
        return true;

    if (c->isAnimated())
        return false;

    if (c == c->pile()->topCard() && c->isFaceUp())
    {
        for (int i = 0; i < 4; i++)
        {
            if (freecell[i]->isEmpty())
            {
                moveCardToPile( c, freecell[i], DURATION_MOVE );
                return true;
            }
        }
    }
    return false;
}

bool Freecell::canPutStore( const KCardPile * pile, const QList<KCard*> & cards ) const
{
    int freeCells = 0;
    for ( int i = 0; i < 4; ++i )
        if ( freecell[i]->isEmpty() )
            ++freeCells;

    int freeStores = 0;
    for ( int i = 0; i < 8; ++i )
        if ( store[i]->isEmpty() && store[i] != pile )
            ++freeStores;

    return cards.size() <= (freeCells + 1) << freeStores
           && ( pile->isEmpty()
                || ( pile->topCard()->rank() == cards.first()->rank() + 1
                     && pile->topCard()->color() != cards.first()->color() ) );

}

bool Freecell::checkAdd(const PatPile * pile, const QList<KCard*> & oldCards, const QList<KCard*> & newCards) const
{
    switch (pile->pileRole())
    {
    case PatPile::Tableau:
        return canPutStore(pile, newCards);
    case PatPile::Cell:
        return oldCards.isEmpty() && newCards.size() == 1;
    case PatPile::Foundation:
        return checkAddSameSuitAscendingFromAce(oldCards, newCards);
    default:
        return false;
    }
}

bool Freecell::checkRemove(const PatPile * pile, const QList<KCard*> & cards) const
{
    switch (pile->pileRole())
    {
    case PatPile::Tableau:
        return isAlternateColorDescending(cards);
    case PatPile::Cell:
        return cards.first() == pile->topCard();
    case PatPile::Foundation:
    default:
        return false;
    }
}

QList<MoveHint> Freecell::getHints()
{
    QList<MoveHint> hintList = getSolverHints();

    if ( isDemoActive() )
        return hintList;

    foreach (PatPile * store, patPiles())
    {
        if (store->isEmpty())
            continue;

        QList<KCard*> cards = store->cards();
        while (cards.count() && !cards.first()->isFaceUp())
            cards.erase(cards.begin());

        QList<KCard*>::Iterator iti = cards.begin();
        while (iti != cards.end())
        {
            if (allowedToRemove(store, (*iti)))
            {
                foreach (PatPile * dest, patPiles())
                {
                    int cardIndex = store->indexOf(*iti);
                    if (cardIndex == 0 && dest->isEmpty() && !dest->isFoundation())
                        continue;

                    if (!checkAdd(dest, dest->cards(), cards))
                        continue;

                    if ( dest->isFoundation() ) // taken care by solver
                        continue;

                    QList<KCard*> cardsBelow = cards.mid(0, cardIndex);
                    // if it could be here as well, then it's no use
                    if ((cardsBelow.isEmpty() && !dest->isEmpty()) || !checkAdd(store, cardsBelow, cards))
                    {
                        hintList << MoveHint( *iti, dest, 0 );
                    }
                    else if (checkPrefering( dest, dest->cards(), cards )
                             && !checkPrefering( store, cardsBelow, cards ))
                    { // if checkPrefers says so, we add it nonetheless
                        hintList << MoveHint( *iti, dest, 0 );
                    }
                }
            }
            cards.erase(iti);
            iti = cards.begin();
        }
    }
    return hintList;
}


static class FreecellDealerInfo : public DealerInfo
{
public:
    FreecellDealerInfo()
      : DealerInfo(I18N_NOOP("Freecell"), FreecellId)
    {}

    DealerScene *createGame() const Q_DECL_OVERRIDE
    {
        return new Freecell( this );
    }
} freecellDealerInfo;



