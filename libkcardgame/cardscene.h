/*
 * Copyright (C) 1995 Paul Olav Tvete <paul@troll.no>
 * Copyright (C) 2000-2009 Stephan Kulow <coolo@kde.org>
 * Copyright (C) 2009-2010 Parker Coates <parker.coates@kdemail.net>
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


#ifndef CARDSCENE_H
#define CARDSCENE_H

class Card;
class CardDeck;
class HighlightableItem;
#include "libkcardgame_export.h"
class Pile;

#include <QtCore/QSet>
#include <QtGui/QGraphicsScene>

class LIBKCARDGAME_EXPORT CardScene : public QGraphicsScene
{
public:
    enum SceneAlignmentFlag
    {
        AlignLeft = 0x0001,
        AlignRight = 0x0002,
        AlignHCenter = 0x0004,
        AlignHSpread = 0x0008,
        AlignTop = 0x0010,
        AlignBottom = 0x0020,
        AlignVCenter = 0x0040,
        AlignVSpread = 0x0080
    };
    Q_DECLARE_FLAGS(SceneAlignment, SceneAlignmentFlag)

    CardScene( QObject * parent = 0 );
    ~CardScene();

    void setDeck( CardDeck * deck );
    CardDeck * deck() const;

    QList<Card*> cardsBeingDragged() const;

    virtual void resizeScene( const QSize & size );
    virtual void relayoutScene();
    virtual void relayoutPiles();

    virtual void addPile( Pile * pile );
    virtual void removePile( Pile * pile );
    QList<Pile*> piles() const;

    void setSceneAlignment( SceneAlignment alignment );
    SceneAlignment sceneAlignment() const;
    void setLayoutMargin( qreal margin );
    qreal layoutMargin() const;
    void setLayoutSpacing( qreal spacing );
    qreal layoutSpacing() const;

    void setHighlightedItems( QList<QGraphicsItem*> items );
    void clearHighlightedItems();
    QList<QGraphicsItem*> highlightedItems() const;

protected:
    virtual void onGameStateAlteredByUser();

    virtual bool allowedToAdd( const Pile * pile, const QList<Card*> & cards ) const;
    virtual bool allowedToRemove( const Pile * pile, const Card * card ) const;
    virtual Pile * targetPile();

    virtual void setItemHighlight( QGraphicsItem * item, bool highlight );

    virtual bool pileClicked( Pile * pile );
    virtual bool pileDoubleClicked( Pile * pile );
    virtual bool cardClicked( Card * card );
    virtual bool cardDoubleClicked( Card * card );

    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * e );
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * e );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * e );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * e );
    virtual void wheelEvent( QGraphicsSceneWheelEvent * e );

    virtual void drawForeground ( QPainter * painter, const QRectF & rect );

private:
    CardDeck * m_deck;
    QList<Pile*> m_piles;
    QSet<QGraphicsItem*> m_highlightedItems;

    QList<Card*> m_cardsBeingDragged;
    QPointF m_startOfDrag;
    bool m_dragStarted;

    SceneAlignment m_alignment;
    qreal m_layoutMargin;
    qreal m_layoutSpacing;
    QSizeF m_contentSize;

    bool m_sizeHasBeenSet;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( CardScene::SceneAlignment )

#endif