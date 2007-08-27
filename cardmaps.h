/***********************-*-C++-*-********

  cardmaps.h  defines pixmaps for playing cards

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

****************************************/

#ifndef KPAT_CARDMAPS_H
#define KPAT_CARDMAPS_H

#include "card.h"
//Added by qt3to4:
#include <QPixmap>

class cardMapPrivate;
class KConfigGroup;

class cardMap : public QObject
{
    Q_OBJECT
public:

    static cardMap *self();
    cardMap();
    ~cardMap();

    static const int NumColors = 4;
    static const int CardsPerColor = 13;

    double scaleFactor() const;
    double wantedCardWidth() const;
    double wantedCardHeight() const;
    void setWantedCardWidth( double w );

    QPixmap renderCard( const QString &element );
    void registerCard( const QString &element );
    QString pickRandom();
    QRect opaqueRect() const;

    void updateTheme(const KConfigGroup &cg);

protected slots:
    void slotThreadFinished();
    void slotThreadEnded();

public slots:
    void triggerRescale();

private:
    cardMapPrivate *d;

    static cardMap *_self;
};

#endif // KPAT_CARDMAPS_H
