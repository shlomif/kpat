/*
 * Copyright (C) 1995 Paul Olav Tvete <paul@troll.no>
 * Copyright (C) 2000-2009 Stephan Kulow <coolo@kde.org>
 * Copyright (C) 2009 Parker Coates <parker.coates@kdemail.net>
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

#include "carddeck.h"

#include "pile.h"

#include <KCardDeckInfo>
#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KSharedConfig>

#include <QtCore/QCache>
#include <QtCore/QTimer>
#include <QtGui/QGraphicsScene>

class CardDeckPrivateStatic
{
public:
    CardDeckPrivateStatic()
      : m_cacheCache( 3 )
    {}

    KCardCache * getCardCache( const QString & frontSide, const QString & backSide )
    {
        KCardCache * cache = m_cacheCache.take( frontSide );
        if ( !cache )
        {
            cache = new KCardCache();
            cache->setFrontTheme( frontSide );
        }
        cache->setBackTheme( backSide );
        return cache;
    };

    void stashCardCache( KCardCache * cache )
    {
        if ( cache )
            m_cacheCache.insert( cache->frontTheme(), cache );
    };

private:
    QCache<QString,KCardCache> m_cacheCache;
};

K_GLOBAL_STATIC( CardDeckPrivateStatic, cdps )


CardDeck::CardDeck( int copies, QList<Card::Suit> suits, QList<Card::Rank> ranks )
  : m_cache( 0 ),
    m_originalCardSize( 1, 1 ),
    m_currentCardSize( 0, 0 ),
    m_cardsWaitedFor()
{
    Q_ASSERT( copies >= 1 );
    Q_ASSERT( suits.size() >= 1 );
    Q_ASSERT( ranks.size() >= 1 );

    // Note the order the cards are created in can't be changed as doing so
    // will mess up the game numbering.
    for ( int i = 0; i < copies; ++i )
    {
        foreach ( const Card::Rank r, ranks )
        {
            foreach ( const Card::Suit s, suits )
            {
                Card * c = new Card( r, s, this );
                kDebug() << c->data() << c->suit() << c->rank();
                connect( c, SIGNAL(animationStarted(Card*)), this, SLOT(cardStartedAnimation(Card*)) );
                connect( c, SIGNAL(animationStopped(Card*)), this, SLOT(cardStoppedAnimation(Card*)) );
                m_allCards << c;
            }
        }
    }
    m_undealtCards = m_allCards;

    Q_ASSERT( m_allCards.size() == copies * ranks.size() * suits.size() );
}


CardDeck::~CardDeck()
{
    returnAllCards();
    foreach ( Card * c, m_allCards )
        delete c;
    m_allCards.clear();
    m_undealtCards.clear();

    cdps->stashCardCache( m_cache );
}


QList< Card* > CardDeck::cards() const
{
    return m_allCards;
}


bool CardDeck::hasUndealtCards() const
{
    return !m_undealtCards.isEmpty();
}


Card * CardDeck::takeCard()
{
    if ( m_undealtCards.isEmpty() )
        return 0;

    return m_undealtCards.takeLast();
}


Card * CardDeck::takeCard( Card::Rank rank, Card::Suit suit )
{
    for ( QList<Card*>::iterator it = m_undealtCards.begin();
          it != m_undealtCards.end();
          ++it )
    {
        Card * c = *it;
        if ( c->rank() == rank && c->suit() == suit )
        {
            m_undealtCards.erase( it );
            return c;
        }
    }
    return 0;
}


void CardDeck::takeAllCards( Pile * p )
{
    while ( !m_undealtCards.isEmpty() )
    {
        Card * c = m_undealtCards.takeFirst();
        c->setPos( p->pos() );
        c->turn( false );
        p->add( c );
    }
}


void CardDeck::returnCard( Card * c )
{
    c->setTakenDown( false );
    if ( c->source() )
        c->source()->remove( c );
    if ( c->scene() )
        c->scene()->removeItem( c );
    m_undealtCards.append( c );
}


void CardDeck::returnAllCards()
{
    m_undealtCards.clear();
    foreach ( Card * c, m_allCards )
        returnCard( c );
}


// Shuffle all undealt cards
void CardDeck::shuffle( int gameNumber )
{
    Q_ASSERT( gameNumber >= 0 );
    m_pseudoRandomSeed = gameNumber;

    Card* t;
    int z;
    int left = m_undealtCards.size();
    for ( int i = 0; i < m_undealtCards.size(); ++i )
    {
        z = pseudoRandom() % left;
        t = m_undealtCards[ z ];
        m_undealtCards[ z ] = m_undealtCards[ left - 1 ];
        m_undealtCards[ left - 1 ] = t;
        --left;
    }
}


void CardDeck::setCardWidth( int width )
{
    if ( width > 200 || width < 20 )
        return;

    int height = width * m_originalCardSize.height() / m_originalCardSize.width();
    QSize newSize( width, height );

    if ( newSize != m_currentCardSize )
    {
        m_currentCardSize = newSize;
        m_cache->setSize( newSize );
        foreach ( Card * c, m_allCards )
            c->updatePixmap();

        QTimer::singleShot( 200, this, SLOT(loadInBackground()) );;
    }
}


int CardDeck::cardWidth() const
{
    return m_currentCardSize.width();
}


void CardDeck::setCardHeight( int height )
{
    setCardWidth( height * m_originalCardSize.width() / m_originalCardSize.height() );
}


int CardDeck::cardHeight() const
{
    return m_currentCardSize.height();
}


QSize CardDeck::cardSize() const
{
    return m_currentCardSize;
}


QPixmap CardDeck::frontsidePixmap( Card::Rank r, Card::Suit s ) const
{
    KCardInfo::Suit suit;
    switch ( s )
    {
        case Card::Clubs :    suit = KCardInfo::Club;    break;
        case Card::Spades :   suit = KCardInfo::Spade;   break;
        case Card::Diamonds : suit = KCardInfo::Diamond; break;
        case Card::Hearts :   suit = KCardInfo::Heart;   break;
    }

    KCardInfo::Card rank;
    switch ( r )
    {
        case Card::Ace :   rank = KCardInfo::Ace;   break;
        case Card::Two :   rank = KCardInfo::Two;   break;
        case Card::Three : rank = KCardInfo::Three; break;
        case Card::Four :  rank = KCardInfo::Four;  break;
        case Card::Five :  rank = KCardInfo::Five;  break;
        case Card::Six :   rank = KCardInfo::Six;   break;
        case Card::Seven : rank = KCardInfo::Seven; break;
        case Card::Eight : rank = KCardInfo::Eight; break;
        case Card::Nine :  rank = KCardInfo::Nine;  break;
        case Card::Ten :   rank = KCardInfo::Ten;   break;
        case Card::Jack :  rank = KCardInfo::Jack;  break;
        case Card::Queen : rank = KCardInfo::Queen; break;
        case Card::King :  rank = KCardInfo::King;  break;
    }

    return m_cache->frontside( KCardInfo( suit, rank ) );
}


QPixmap CardDeck::backsidePixmap( int variant ) const
{
    return m_cache->backside( variant );
}


void CardDeck::updateTheme( const KConfigGroup & cs )
{
    QString fronttheme = CardDeckInfo::frontTheme( cs );
    QString backtheme = CardDeckInfo::backTheme( cs );
    Q_ASSERT ( !backtheme.isEmpty() );
    Q_ASSERT ( !fronttheme.isEmpty() );

    cdps->stashCardCache( m_cache );
    m_cache = cdps->getCardCache( fronttheme, backtheme );

    m_originalCardSize = m_cache->defaultFrontSize( KCardInfo( KCardInfo::Spade, KCardInfo::Ace ) );
    Q_ASSERT( !m_originalCardSize.isNull() );
    m_currentCardSize = m_originalCardSize.toSize();
}


bool CardDeck::hasAnimatedCards() const
{
    return !m_cardsWaitedFor.isEmpty();
}


void CardDeck::loadInBackground()
{
    m_cache->loadTheme( KCardCache::LoadFrontSide | KCardCache::Load52Cards );
}


void CardDeck::cardStartedAnimation( Card *card )
{
    Q_ASSERT( !m_cardsWaitedFor.contains( card ) );
    m_cardsWaitedFor.insert( card );
}


void CardDeck::cardStoppedAnimation( Card *card )
{
    Q_ASSERT( m_cardsWaitedFor.contains( card ) );
    m_cardsWaitedFor.remove( card );

    if ( m_cardsWaitedFor.isEmpty() )
        emit cardAnimationDone();
}


// KPat uses the same pseudorandom number generation algorithm as Windows
// Freecell, so that game numbers are the same between the two applications.
// For more inforation, see 
// http://support.microsoft.com/default.aspx?scid=kb;EN-US;Q28150
int CardDeck::pseudoRandom() {
    m_pseudoRandomSeed = 214013 * m_pseudoRandomSeed + 2531011;
    return ( m_pseudoRandomSeed >> 16 ) & 0x7fff;
}

#include "carddeck.moc"