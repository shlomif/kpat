#ifndef _DECK_H_
#define _DECK_H_

#include "pile.h"
class dealer;

/***************************************

  Deck -- create and shuffle 52 cards

**************************************/
class Deck: public Pile
{

public:

    Deck( int index, Dealer* parent = 0, int m = 1 );
    virtual ~Deck();

    static const long n;

    void collectAndShuffle();

    Card* nextCard();

    uint decksNum() const { return mult; }

 private: // functions

    void makedeck();
    void addToDeck();
    void shuffle();

private:

    uint mult;
    Card** deck;
};

#endif
