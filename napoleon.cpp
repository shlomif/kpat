/*
  napoleon.cpp  implements a patience card game

      Copyright (C) 1995  Paul Olav Tvete

 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.

 */

#include "napoleon.h"
#include <qevent.h>
#include <qpainter.h>
#include "dealer.h"
#include <klocale.h>
#include <kmainwindow.h>
#include <deck.h>
#include <pile.h>

Napoleon::Napoleon( KMainWindow* parent, const char* _name )
  : Dealer( parent, _name )
{
    deck = new Deck( 0, this );
    deck->move( 500, 290);
    connect(deck, SIGNAL(clicked()), SLOT(deal1()));

    pile = new Pile( 1, this );
    pile->setAddFlags( Pile::disallow );
    pile->move(400, 290);

    for (int i = 0; i < 4; i++)
    {
        store[i] = new Pile( 2 + i, this );
        store[i]->setCheckIndex( 0 );
        target[i] = new Pile( 6 + i, this);
        target[i]->setRemoveFlags( Pile::disallow );
        target[i]->setCheckIndex(2);
        target[i]->setTarget(true);
    }

    centre = new Pile( 10, this );
    centre->setRemoveFlags( Pile::disallow );
    centre->setCheckIndex(1);
    centre->setTarget(true);

    store[0]->move( 160,  10);
    store[1]->move( 270, 150);
    store[2]->move( 160, 290);
    store[3]->move(  50, 150);
    target[0]->move(  70,  30);
    target[1]->move( 250,  30);
    target[2]->move( 250, 270);
    target[3]->move(  70, 270);
    centre->move(160, 150);

    deal();
}

void Napoleon::restart() {
    deck->collectAndShuffle();
    deal();
}

bool Napoleon::CanPutTarget( const Pile* c1, const CardList& cl) const {
    Card *c2 = cl.first();

    if (c1->isEmpty())
        return c2->value() == Card::Seven;
    else
        return (c2->value() == c1->top()->value() + 1);
}

bool Napoleon::CanPutCentre( const Pile* c1, const CardList& cl) const {
    Card *c2 = cl.first();

    if (c1->isEmpty())
        return c2->value() == Card::Six;

    if (c1->top()->value() == Card::Ace)
        return (c2->value() == Card::Six);
    else
        return (c2->value() == c1->top()->value() - 1);
}

bool Napoleon::checkAdd( int checkIndex, const Pile *c1, const CardList& c2) const
{
    switch (checkIndex) {
        case 0:
            return c1->isEmpty();
        case 1:
            return CanPutCentre(c1, c2);
        case 2:
            return CanPutTarget(c1, c2);
        default:
            return false;
    }
}

void Napoleon::deal() {
    if (deck->isEmpty())
        return;

    for (int i=0; i<4; i++)
        store[i]->add(deck->nextCard(), false, false);
}

void Napoleon::deal1() {
    pile->add(deck->nextCard(), false, false);
}

static class LocalDealerInfo4 : public DealerInfo
{
public:
    LocalDealerInfo4() : DealerInfo(I18N_NOOP("&Napoleon's Tomb"), 4) {}
    virtual Dealer *createGame(KMainWindow *parent) { return new Napoleon(parent); }
} ldi3;

#include "napoleon.moc"
