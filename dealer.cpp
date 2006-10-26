/*
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
#include "dealer.h"
#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include "deck.h"
#include <assert.h>
#include "kmainwindow.h"
#include <kapplication.h>
#include <kpixmapeffect.h>
#include <kxmlguifactory.h>
#include <kicon.h>
#include <QTimer>
//Added by qt3to4:
#include <QWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif
#include <QPixmap>
#include <QDebug>
#include <QList>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QDomElement>
#include <krandom.h>
#include <kaction.h>
#include <klocale.h>
#include "cardmaps.h"
#include "speeds.h"
#include <kconfig.h>
#include <kglobal.h>
#include "version.h"
#include <ktoggleaction.h>

#include <math.h>

// ================================================================
//                         class MoveHint

MoveHint::MoveHint(Card *card, Pile *to, bool d)
{
    m_card         = card;
    m_to           = to;
    m_dropiftarget = d;
}


// ================================================================
//                    class DealerInfoList


DealerInfoList *DealerInfoList::_self = 0;
static KStaticDeleter<DealerInfoList> dl;


DealerInfoList *DealerInfoList::self()
{
    if (!_self)
        _self = dl.setObject(_self, new DealerInfoList());
    return _self;
}

void DealerInfoList::add(DealerInfo *dealer)
{
    list.append(dealer);
}


// ================================================================
//                            class Dealer


Dealer *Dealer::s_instance = 0;

DealerScene::DealerScene():
    towait(0),
    _autodrop(true),
    _waiting(0),
    gamenumber( 0 ),
    stop_demo_next(false),
    _won(false),
    _gameRecorded(false),
    wonItem( 0 )
{
    kDebug() << "DealerScene\n";

    demotimer = new QTimer(this);
    connect(demotimer, SIGNAL(timeout()), SLOT(demo()));
    m_autoDropFactor = 1;
    connect( this, SIGNAL( gameWon( bool ) ), SLOT( slotShowGame() ) );
}

DealerScene::~DealerScene()
{
    if (!_won)
        countLoss();

    // don't delete the deck
    if ( Deck::deck()->scene() == this )
    {
        Deck::deck()->setParent( 0 );
        removeItem( Deck::deck() );
    }
    removePile( Deck::deck() );
    while (!piles.isEmpty()) {
        delete piles.first(); // removes itself
    }
}

Dealer::Dealer( KMainWindow* _parent )
  : QGraphicsView( _parent ),
    myActions(0),
    ademo(0),
    ahint(0),
    aredeal(0),
    m_shown( false )
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAlignment( Qt::AlignLeft | Qt::AlignTop );

    assert(!s_instance);
    s_instance = this;

    setCacheMode(QGraphicsView::CacheBackground);

#ifndef QT_NO_OPENGL
    //QGLWidget *wgl = new QGLWidget();
    //setupViewport(wgl);
#endif
}

void Dealer::setScene( QGraphicsScene *_scene )
{
    parent()->guiFactory()->unplugActionList( parent(), QString::fromLatin1("game_actions"));
    QGraphicsScene *oldscene = scene();
    QGraphicsView::setScene( _scene );
    delete oldscene;
    dscene()->rescale(true);
    dscene()->setGameNumber( KRandom::random() );
//    dscene()->setGameNumber( 1438470683 );

    // dscene()->setSceneRect( QRectF( 0,0,700,500 ) );
    scaleFactor = 1;
    dscene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    // connect( _scene, SIGNAL( gameWon( bool ) ), SIGNAL( gameWon( bool ) ) );
    connect( dscene(), SIGNAL( undoPossible( bool ) ), SIGNAL( undoPossible( bool ) ) );

    if ( ademo )
        connect( dscene(), SIGNAL( demoActive( bool ) ), ademo, SLOT( setEnabled( bool ) ) );
    if ( oldscene )
        dscene()->relayoutPiles();

    setupActions();
}

Dealer *Dealer::instance()
{
    return s_instance;
}

void Dealer::setupActions() {

    QList<KAction*> actionlist;

    kDebug(11111) << "setupActions " << actions() << endl;

    if (actions() & Dealer::Hint) {

        ahint = new KAction( i18n("&Hint"),
                             parent()->actionCollection(), "game_hint");
        ahint->setCustomShortcut( Qt::Key_H );
        ahint->setIcon( KIcon( "wizard" ) );
        connect( ahint, SIGNAL( triggered( bool ) ), SLOT(hint()) );
        actionlist.append(ahint);
    } else
        ahint = 0;

    if (actions() & Dealer::Demo) {
        ademo = new KToggleAction( i18n("&Demo"), parent()->actionCollection(), "game_demo");
        ademo->setIcon( KIcon( "1rightarrow") );
        ademo->setCustomShortcut( Qt::CTRL+Qt::Key_D );
        connect( ademo, SIGNAL( triggered( bool ) ), dscene(), SLOT( toggleDemo() ) );
        actionlist.append(ademo);
    } else
        ademo = 0;

    if (actions() & Dealer::Redeal) {
        aredeal = new KAction (i18n("&Redeal"), parent()->actionCollection(), "game_redeal");
        aredeal->setIcon( KIcon( "queue") );
        connect( aredeal, SIGNAL( triggered( bool ) ), dscene(), SLOT( redeal() ) );
        actionlist.append(aredeal);
    } else
        aredeal = 0;

    parent()->guiFactory()->plugActionList( parent(), QString::fromLatin1("game_actions"), actionlist);
}

Dealer::~Dealer()
{
    dscene()->clearHints();

    if (s_instance == this)
        s_instance = 0;
}

KMainWindow *Dealer::parent() const
{
    return dynamic_cast<KMainWindow*>(QGraphicsView::parent());
}


// ----------------------------------------------------------------

void DealerScene::hint()
{
    kDebug() << "hint\n";
    unmarkAll();
    clearHints();
    getHints();
    for (HintList::ConstIterator it = hints.begin(); it != hints.end(); ++it)
        mark((*it)->card());
    clearHints();
}

void Dealer::hint()
{
    dscene()->hint();
}

void Dealer::setAutoDropEnabled(bool a)
{
    dscene()->setAutoDropEnabled( a );
}

void DealerScene::getHints()
{
    for (PileList::Iterator it = piles.begin(); it != piles.end(); ++it)
    {
        if ((*it)->target())
            continue;

        Pile *store = *it;
        if (store->isEmpty())
            continue;
//        kDebug(11111) << "trying " << store->top()->name() << endl;

        CardList cards = store->cards();
        while (cards.count() && !cards.first()->realFace()) cards.erase(cards.begin());

        CardList::Iterator iti = cards.begin();
        while (iti != cards.end())
        {
            if (store->legalRemove(*iti)) {
//                kDebug(11111) << "could remove " << (*iti)->name() << endl;
                for (PileList::Iterator pit = piles.begin(); pit != piles.end(); ++pit)
                {
                    Pile *dest = *pit;
                    if (dest == store)
                        continue;
                    if (store->indexOf(*iti) == 0 && dest->isEmpty() && !dest->target())
                        continue;
                    if (!dest->legalAdd(cards))
                        continue;

                    bool old_prefer = checkPrefering( dest->checkIndex(), dest, cards );
                    if (dest->target())
                        newHint(new MoveHint(*iti, dest));
                    else {
                        store->hideCards(cards);
                        // if it could be here as well, then it's no use
                        if ((store->isEmpty() && !dest->isEmpty()) || !store->legalAdd(cards))
                            newHint(new MoveHint(*iti, dest));
                        else {
                            if (old_prefer && !checkPrefering( store->checkIndex(),
                                                               store, cards ))
                            { // if checkPrefers says so, we add it nonetheless
                                newHint(new MoveHint(*iti, dest));
                            }
                        }
                        store->unhideCards(cards);
                    }
                }
            }
            cards.erase(iti);
            iti = cards.begin();
        }
    }
}

bool DealerScene::checkPrefering( int /*checkIndex*/, const Pile *, const CardList& ) const
{
    return false;
}

void DealerScene::clearHints()
{
    for (HintList::Iterator it = hints.begin(); it != hints.end(); ++it)
        delete *it;
    hints.clear();
}

void DealerScene::newHint(MoveHint *mh)
{
    hints.append(mh);
}

bool DealerScene::isMoving(Card *c) const
{
    return movingCards.indexOf(c) != -1;
}

void DealerScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (movingCards.isEmpty())
        return;

    moved = true;

    for (CardList::Iterator it = movingCards.begin(); it != movingCards.end(); ++it)
    {
        (*it)->moveBy(e->scenePos().x() - moving_start.x(),
                      e->scenePos().y() - moving_start.y());
    }

    PileList sources;
    QList<QGraphicsItem *> list = collidingItems( movingCards.first() );

    // kDebug() << "movingCards " << movingCards.first()->sceneBoundingRect() << endl;
    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        //kDebug() << "it " << ( *it )->type() << " " << ( *it )->sceneBoundingRect() << endl;
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId) {
            Card *c = dynamic_cast<Card*>(*it);
            assert(c);
            if (!c->isFaceUp())
                continue;
            if (c->source() == movingCards.first()->source())
                continue;
            if (sources.indexOf(c->source()) != -1)
                continue;
            sources.append(c->source());
        } else {
            if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId ) {
                Pile *p = static_cast<Pile*>(*it);
                if (p->isEmpty() && !sources.contains(p))
                    sources.append(p);
            } else {
                kDebug(11111) << "unknown object " << *it << " " << (*it)->type() << endl;
            }
        }
    }

    // TODO some caching of the results
    unmarkAll();

    for (PileList::Iterator it = sources.begin(); it != sources.end(); ++it)
    {
        bool b = (*it)->legalAdd(movingCards);
        // kDebug() << "legalAdd " << b << " " << ( *it )->x() << endl;
        if (b) {
            if ((*it)->isEmpty()) {
                (*it)->setHighlighted(true);
                marked.append(*it);
            } else {
                mark((*it)->top());
            }
        }
    }

    moving_start = e->scenePos();
}

void DealerScene::mark(Card *c)
{
//    kDebug() << "mark " << c->name() << endl;
    c->setHighlighted(true);
    if (!marked.contains(c))
        marked.append(c);
}

void DealerScene::unmarkAll()
{
    for (QList<QGraphicsItem *>::Iterator it = marked.begin(); it != marked.end(); ++it)
    {
        Card *c = dynamic_cast<Card*>( *it );
        if ( c )
            c->setHighlighted(false);
        Pile *p = dynamic_cast<Pile*>( *it );
        if ( p )
            p->setHighlighted(false);
    }
    marked.clear();
}

void DealerScene::mousePressEvent( QGraphicsSceneMouseEvent * e )
{
    unmarkAll();
    stopDemo();
    if (waiting())
        return;

    QList<QGraphicsItem *> list = items( e->scenePos() );

    kDebug(11111) << "mouse pressed " << list.count() << " " << items().count() << " " << e->scenePos().x() << " " << e->scenePos().y() << endl;
    moved = false;

    if (!list.count())
        return;

    QGraphicsScene::mousePressEvent( e );

    if (e->button() == Qt::LeftButton) {
        if (list.first()->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            Card *c = dynamic_cast<Card*>(list.first());
            assert(c);
            CardList mycards = c->source()->cardPressed(c);
            for (CardList::Iterator it = mycards.begin(); it != mycards.end(); ++it)
                (*it)->stopAnimation();
            movingCards = mycards;
            moving_start = e->scenePos();
        }
        return;
    }

    if (e->button() == Qt::RightButton) {
        if (list.first()->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            Card *preview = dynamic_cast<Card*>(list.first());
            assert(preview);
            if (!preview->animated() && !isMoving(preview))
                preview->getUp();
        }
        return;
    }

    // if it's nothing else, we move the cards back
    mouseReleaseEvent(e);

}

class Hit {
public:
    Pile *source;
    QRectF intersect;
    bool top;
};
typedef QList<Hit> HitList;

void DealerScene::startNew()
{
    if (!_won)
        countLoss();
    _won = false;
    _waiting = 0;
    _gameRecorded=false;
    delete wonItem;
    wonItem = 0;
}

void DealerScene::slotShowGame()
{
    QGraphicsSvgItem *rect = new QGraphicsSvgItem( KStandardDirs::locate( "data", "kpat/won.svg" ) );
    addItem( rect );
    rect->setElementId( "frame" );

    qreal scaleX, scaleY;

    rect->setVisible( true );
    scaleX = width() * 0.7 / rect->sceneBoundingRect().width();
    scaleY = height() * 0.6 / rect->sceneBoundingRect().height();
    rect->scale( scaleX , scaleY );

    QGraphicsSimpleTextItem *text = new QGraphicsSimpleTextItem( 0, this );
    text->setText( "Congratulation! You have won." );
    QFont font;
    font.setPointSize( 36 );
    text->setFont( font );
    rect->setZValue( 2000 );
    text->setZValue( 2001 );
    text->setVisible( true );
    rect->setPos( QPointF( ( width() - rect->sceneBoundingRect().width() ) / 2,
                           ( height() - rect->sceneBoundingRect().height() ) / 2 ) );

    int fontsize = 36;
    while (  text->sceneBoundingRect().width() >  rect->sceneBoundingRect().width() * 0.9 )
    {
        fontsize--;
        font.setPointSize( fontsize );
        text->setFont( font );
    }
    text->setPos( QPointF( ( width() - text->sceneBoundingRect().width() ) / 2,
                           ( height() - text->sceneBoundingRect().height() ) / 2 ) );
}

void DealerScene::mouseReleaseEvent( QGraphicsSceneMouseEvent *e )
{
    QGraphicsScene::mouseReleaseEvent( e );

    if (!moved) {
        if (!movingCards.isEmpty()) {
            movingCards.first()->source()->moveCardsBack(movingCards);
            movingCards.clear();
        }
        QList<QGraphicsItem *> list = items(e->scenePos());
        if (list.isEmpty())
            return;
        QList<QGraphicsItem *>::Iterator it = list.begin();
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            Card *c = dynamic_cast<Card*>(*it);
            assert(c);
            if (!c->animated()) {
                if ( cardClicked(c) ) {
                    countGame();
                }
                Dealer::instance()->takeState();
            }
            return;
        }
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId ) {
            Pile *c = dynamic_cast<Pile*>(*it);
            assert(c);
            pileClicked(c);
            Dealer::instance()->takeState();
            return;
        }
    }

    if (!movingCards.count())
        return;
    Card *c = static_cast<Card*>(movingCards.first());
    assert(c);

    unmarkAll();

    QList<QGraphicsItem *> list = collidingItems( movingCards.first() );
    HitList sources;

    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            Card *c = dynamic_cast<Card*>(*it);
            assert(c);
            if (!c->isFaceUp())
                continue;
            if (c->source() == movingCards.first()->source())
                continue;
            Hit t;
            t.source = c->source();
            t.intersect = c->sceneBoundingRect().intersect(movingCards.first()->sceneBoundingRect());
            t.top = (c == c->source()->top());

            bool found = false;
            for (HitList::Iterator hi = sources.begin(); hi != sources.end(); ++hi)
            {
                if ((*hi).source == c->source()) {
                    found = true;
                    if ((*hi).intersect.width() * (*hi).intersect.height() >
                        t.intersect.width() * t.intersect.height())
                    {
                        (*hi).intersect = t.intersect;
                        (*hi).top |= t.top;
                    }
                }
            }
            if (found)
                continue;

            sources.append(t);
        } else {
            if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId ) {
                Pile *p = static_cast<Pile*>(*it);
                if (p->isEmpty())
                {
                    Hit t;
                    t.source = p;
                    t.intersect = p->sceneBoundingRect().intersect(movingCards.first()->sceneBoundingRect());
                    t.top = true;
                    sources.append(t);
                }
            } else {
                kDebug(11111) << "unknown object " << *it << " " << (*it)->type() << endl;
            }
        }
    }

    for (HitList::Iterator it = sources.begin(); it != sources.end(); )
    {
        if (!(*it).source->legalAdd(movingCards))
            it = sources.erase(it);
        else
            ++it;
    }

    if (sources.isEmpty()) {
        c->source()->moveCardsBack(movingCards);
    } else {
        HitList::Iterator best = sources.begin();
        HitList::Iterator it = best;
        for (++it; it != sources.end(); ++it )
        {
            if ((*it).intersect.width() * (*it).intersect.height() >
                (*best).intersect.width() * (*best).intersect.height()
                || ((*it).top && !(*best).top))
            {
                best = it;
            }
        }
        countGame();
        c->source()->moveCards(movingCards, (*best).source);
        Dealer::instance()->takeState();
    }
    movingCards.clear();
}

void DealerScene::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *e )
{
    kDebug() << "mouseDoubleClickEvent " << waiting() << endl;;
    DealerScene::stopDemo();
    unmarkAll();
    if (waiting())
        return;

    if (!movingCards.isEmpty()) {
        movingCards.first()->source()->moveCardsBack(movingCards);
        movingCards.clear();
    }
    QList<QGraphicsItem *> list = items(e->scenePos());
    kDebug() << "mouseDoubleClickEvent " << list << endl;
    if (list.isEmpty())
        return;
    QList<QGraphicsItem *>::Iterator it = list.begin();
    if ((*it)->type() != QGraphicsItem::UserType + Dealer::CardTypeId )
        return;
    Card *c = dynamic_cast<Card*>(*it);
    assert(c);
    c->stopAnimation();
    kDebug() << "card " << c->name() << endl;
    if ( cardDblClicked(c) ) {
        countGame();
    }
    Dealer::instance()->takeState();
}

bool DealerScene::cardClicked(Card *c) {
    return c->source()->cardClicked(c);
}

void DealerScene::pileClicked(Pile *c) {
    c->cardClicked(0);
}

bool DealerScene::cardDblClicked(Card *c)
{
    if (c->source()->cardDblClicked(c))
        return true;

    if (c->animated())
        return false;

    if (c == c->source()->top() && c->realFace()) {
        Pile *tgt = findTarget(c);
        if (tgt) {
            CardList empty;
            empty.append(c);
            c->source()->moveCards(empty, tgt);
            return true;
        }
    }
    return false;
}

void Dealer::startNew()
{
    kDebug() << "startnew\n";
    if ( ahint )
        ahint->setEnabled( true );
    if ( ademo )
        ademo->setEnabled( true );
    if ( aredeal )
        aredeal->setEnabled( true );
    toldAboutLostGame = false;
    dscene()->startNew();

    kDebug(11111) << "startNew stopDemo\n";
    dscene()->stopDemo();
    kDebug(11111) << "startNew unmarkAll\n";
    dscene()->unmarkAll();
    kDebug(11111) << "startNew setAnimated(false)\n";
    QList<QGraphicsItem *> list = scene()->items();
    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it) {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId )
        {
            Card *c = static_cast<Card*>(*it);
            c->disconnect();
            c->stopAnimation();
        }
    }

    qDeleteAll(undoList);
    undoList.clear();
    emit undoPossible(false);
    emit updateMoves();
    kDebug(11111) << "startNew restart\n";
    dscene()->restart();
    takeState();
    Card *towait = 0;
    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it) {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            towait = static_cast<Card*>(*it);
            if (towait->animated())
                break;
        }
    }

    kDebug(11111) << "startNew takeState\n";
    if (!towait)
        takeState();
    else
        connect(towait, SIGNAL(stoped(Card*)), SLOT(slotTakeState(Card *)));
}

void Dealer::slotTakeState(Card *c) {
    if (c)
        c->disconnect();
    takeState();
}

void Dealer::slotEnableRedeal( bool en )
{
    if ( aredeal )
        aredeal->setEnabled( en );
}

class CardState {
public:
    Card *it;
    Pile *source;
    double x;
    double y;
    double z;
    bool faceup;
    bool tookdown;
    int source_index;
    CardState() {}
public:
    // as every card is only once we can sort after the card.
    // < is the same as <= in that context. == is different
    bool operator<(const CardState &rhs) const { return it < rhs.it; }
    bool operator<=(const CardState &rhs) const { return it <= rhs.it; }
    bool operator>(const CardState &rhs) const { return it > rhs.it; }
    bool operator>=(const CardState &rhs) const { return it > rhs.it; }
    bool operator==(const CardState &rhs) const {
        return (it == rhs.it && source == rhs.source && x == rhs.x &&
                y == rhs.y && z == rhs.z && faceup == rhs.faceup
                && source_index == rhs.source_index && tookdown == rhs.tookdown);
    }
    void fillNode(QDomElement &e) const {
        e.setAttribute("value", it->rank());
        e.setAttribute("suit", it->suit());
        e.setAttribute("z", z);
        e.setAttribute("faceup", faceup);
        e.setAttribute("tookdown", tookdown);
    }
};

typedef class QList<CardState> CardStateList;

bool operator==( const State & st1, const State & st2) {
    return st1.cards == st2.cards && st1.gameData == st2.gameData;
}

State *DealerScene::getState()
{
    QList<QGraphicsItem *> list = items();
    State * st = new State;

    for (QList<QGraphicsItem *>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
       if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
           Card *c = dynamic_cast<Card*>(*it);
           assert(c);
           CardState s;
           s.it = c;
           s.source = c->source();
           if (!s.source) {
               kDebug(11111) << c->name() << " has no parent\n";
               assert(false);
           }
           s.source_index = c->source()->indexOf(c);
           s.x = c->realX();
           s.y = c->realY();
           s.z = c->realZ();
           s.faceup = c->realFace();
           s.tookdown = c->takenDown();
           st->cards.append(s);
       }
    }
    qSort(st->cards);

    // Game specific information
    st->gameData = getGameState( );

    return st;
}

void DealerScene::setState(State *st)
{
    CardStateList * n = &st->cards;
    QList<QGraphicsItem *> list = items();

    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId )
        {
            Pile *p = dynamic_cast<Pile*>(*it);
            assert(p);
            CardList cards = p->cards();
            for (CardList::Iterator it = cards.begin(); it != cards.end(); ++it)
                (*it)->setTakenDown(p->target());
            p->clear();
        }
    }

    for (CardStateList::ConstIterator it = n->begin(); it != n->end(); ++it)
    {
        Card *c = (*it).it;
        c->stopAnimation();
        CardState s = *it;
        // kDebug() << "c " << c->name() << " " << s.source->objectName() << " " << s.faceup << endl;
        bool target = c->takenDown(); // abused
        s.source->add(c, s.source_index);
        c->setVisible(s.source->isVisible());
        c->setPos(s.x, s.y);
        c->setZValue(int(s.z));
        c->setTakenDown(s.tookdown || (target && !s.source->target()));
        c->turn(s.faceup);
    }

    // restore game-specific information
    setGameState( st->gameData );

    delete st;
}

void Dealer::takeState()
{
//    kDebug(11111) << "takeState " << endl;

    State *n = dscene()->getState();

    if (!undoList.count()) {
        emit updateMoves();
        undoList.append(n);
    } else {
        State *old = undoList.last();

        if (*old == *n) {
            delete n;
            n = 0;
        } else {
            emit updateMoves();
            undoList.append(n);
        }
    }

    if (n) {
        if (dscene()->isGameWon()) {
            dscene()->won();
            return;
        }
        else if ( dscene()->isGameLost() && !toldAboutLostGame) {
            if ( ahint )
                ahint->setEnabled( false );
            if ( ademo )
                ademo->setEnabled( false );
            if ( aredeal )
                aredeal->setEnabled( false );
            QTimer::singleShot(400, this, SIGNAL(gameLost()));
            toldAboutLostGame = true;
            return;
        }
    }
    if (!dscene()->demoActive() && !dscene()->waiting())
        QTimer::singleShot(TIME_BETWEEN_MOVES, dscene(), SLOT(startAutoDrop()));

    emit undoPossible(undoList.count() > 1 && !dscene()->waiting());
}

void Dealer::saveGame(QDomDocument &doc) {
    QDomElement dealer = doc.createElement("dealer");
    doc.appendChild(dealer);
    dealer.setAttribute("id", dscene()->gameId());
    dealer.setAttribute("number", QString::number(dscene()->gameNumber()));
    QString data = dscene()->getGameState();
    if (!data.isEmpty())
        dealer.setAttribute("data", data);
    dealer.setAttribute("moves", QString::number(getMoves()));

    bool taken[1000];
    memset(taken, 0, sizeof(taken));

    QList<QGraphicsItem *> list = scene()->items();
    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId ) {
            Pile *p = dynamic_cast<Pile*>(*it);
            assert(p);
            if (taken[p->index()]) {
                kDebug(11111) << "pile index " << p->index() << " taken twice\n";
                return;
            }
            taken[p->index()] = true;

            QDomElement pile = doc.createElement("pile");
            pile.setAttribute("index", p->index());
            pile.setAttribute("z", p->zValue());

            CardList cards = p->cards();
            for (CardList::Iterator it = cards.begin();
                 it != cards.end();
                 ++it)
            {
                QDomElement card = doc.createElement("card");
                card.setAttribute("suit", (*it)->suit());
                card.setAttribute("value", (*it)->rank());
                card.setAttribute("faceup", (*it)->isFaceUp());
                card.setAttribute("z", (*it)->realZ());
                card.setAttribute("name", (*it)->name());
                pile.appendChild(card);
            }
            dealer.appendChild(pile);
        }
    }

    /*
    QDomElement eList = doc.createElement("undo");

    QPtrListIterator<State> it(undoList);
    for (; it.current(); ++it)
    {
        State *n = it.current();
        QDomElement state = doc.createElement("state");
        if (!n->gameData.isEmpty())
            state.setAttribute("data", n->gameData);
        QDomElement cards = doc.createElement("cards");
        for (QValueList<CardState>::ConstIterator it2 = n->cards.begin();
             it2 != n->cards.end(); ++it2)
        {
            QDomElement item = doc.createElement("item");
            (*it2).fillNode(item);
            cards.appendChild(item);
        }
        state.appendChild(cards);
        eList.appendChild(state);
    }
    dealer.appendChild(eList);
    */
    // kDebug(11111) << doc.toString() << endl;
}

void Dealer::openGame(QDomDocument &doc)
{
    if (!dscene())
        return;

    dscene()->unmarkAll();
    QDomElement dealer = doc.documentElement();

    dscene()->setGameNumber(dealer.attribute("number").toULong());
    qDeleteAll(undoList);
    undoList.clear();

    QDomNodeList piles = dealer.elementsByTagName("pile");

    QList<QGraphicsItem *> list = scene()->items();

    CardList cards;
    for (QList<QGraphicsItem *>::ConstIterator it = list.begin(); it != list.end(); ++it)
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId )
            cards.append(static_cast<Card*>(*it));

    Deck::deck()->collectAndShuffle();

    for (QList<QGraphicsItem *>::Iterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::PileTypeId )
        {
            Pile *p = dynamic_cast<Pile*>(*it);
            assert(p);

            for (int i = 0; i < piles.count(); ++i)
            {
                QDomElement pile = piles.item(i).toElement();
                if (pile.attribute("index").toInt() == p->index())
                {
                    QDomNodeList pcards = pile.elementsByTagName("card");
                    for (int j = 0; j < pcards.count(); ++j)
                    {
                        QDomElement card = pcards.item(j).toElement();
                        Card::Suit s = static_cast<Card::Suit>(card.attribute("suit").toInt());
                        Card::Rank v = static_cast<Card::Rank>(card.attribute("value").toInt());

                        for (CardList::Iterator it2 = cards.begin();
                             it2 != cards.end(); ++it2)
                        {
                            if ((*it2)->suit() == s && (*it2)->rank() == v) {
                                (*it2)->setVisible(p->isVisible());
                                p->add(*it2, !card.attribute("faceup").toInt());
                                (*it2)->stopAnimation();
                                (*it2)->setVisible(p->isVisible());
                                cards.erase(it2);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    dscene()->setGameState( dealer.attribute("data") );

    if (undoList.count() > 1) {
        dscene()->setState(undoList.takeLast());
        takeState(); // copying it again
        emit undoPossible(undoList.count() > 1);
    }

    emit updateMoves();
    takeState();
}

void Dealer::undo()
{
    dscene()->unmarkAll();
    dscene()->stopDemo();
    kDebug() << "::undo " << undoList.count() << endl;
    if (undoList.count() > 1) {
        delete undoList.last();
        undoList.removeLast(); // the current state
        dscene()->setState(undoList.takeLast());
        emit updateMoves();
        takeState(); // copying it again
        emit undoPossible(undoList.count() > 1);
        if ( toldAboutLostGame ) { // everything's possible again
            if ( ahint )
                ahint->setEnabled( true );
            if ( ademo )
                ademo->setEnabled( true );
            toldAboutLostGame = false;
        }
    }
}

Pile *DealerScene::findTarget(Card *c)
{
    if (!c)
        return 0;

    CardList empty;
    empty.append(c);
    for (PileList::ConstIterator it = piles.begin(); it != piles.end(); ++it)
    {
        if (!(*it)->target())
            continue;
        if ((*it)->legalAdd(empty))
            return *it;
    }
    return 0;
}

void DealerScene::setWaiting(bool w)
{
    if (w)
        _waiting++;
    else if ( _waiting > 0 )
        _waiting--;
    emit undoPossible(!waiting());
    //kDebug(11111) << "setWaiting " << w << " " << _waiting << endl;
}

void DealerScene::setAutoDropEnabled(bool a)
{
    _autodrop = a;
    QTimer::singleShot(TIME_BETWEEN_MOVES, this, SLOT(startAutoDrop()));
}

bool DealerScene::startAutoDrop()
{
    if (!autoDrop())
        return false;

    QList<QGraphicsItem *> list = items();

    //kDebug( 11111 ) << "startAutoDrop\n";
    for (QList<QGraphicsItem *>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            Card *c = static_cast<Card*>(*it);
            if (c->animated()) {
                QTimer::singleShot(qRound( TIME_BETWEEN_MOVES * m_autoDropFactor ), this, SLOT(startAutoDrop()));
                // kDebug() << "animation still going on\n";
                return true;
            }
        }
    }

    //kDebug(11111) << "startAutoDrop2\n";

    unmarkAll();
    clearHints();
    getHints();
    for (HintList::ConstIterator it = hints.begin(); it != hints.end(); ++it) {
        MoveHint *mh = *it;
        if (mh->pile()->target() && mh->dropIfTarget() && !mh->card()->takenDown()) {
            setWaiting(true);
            Card *t = mh->card();
            CardList cards = mh->card()->source()->cards();
            while (cards.count() && cards.first() != t)
                cards.erase(cards.begin());
            t->stopAnimation();
            t->turn(true);
            qreal x = t->x();
            qreal y = t->y();
            t->source()->moveCards(cards, mh->pile());
            t->stopAnimation();
            t->setPos(x, y);
            //kDebug(11111) << "autodrop " << t->name() << endl;
            t->moveTo(t->source()->x(), t->source()->y(), t->zValue(), qRound( DURATION_AUTODROP * m_autoDropFactor ) );
            connect(t, SIGNAL(stoped(Card*)), SLOT(waitForAutoDrop(Card*)));
            m_autoDropFactor *= 0.8;
            return true;
        }
    }
    clearHints();
    m_autoDropFactor = 1;
    return false;
}

void DealerScene::waitForAutoDrop(Card * c) {
    //kDebug(11111) << "waitForAutoDrop " << c->name() << endl;
    setWaiting(false);
    c->disconnect();
    c->stopAnimation(); // should be a no-op
    Dealer::instance()->takeState();
}

void DealerScene::waitForWonAnim(Card *c) {
    if ( !waiting() )
        return;

    c->setVisible( false ); // hide it

    setWaiting(false);

    if ( !waiting() )
    {
        bool demo = demoActive();
        stopDemo();
        emit gameWon(demo);
    }
}

long DealerScene::gameNumber() const
{
    return gamenumber;
}

void DealerScene::rescale(bool onlypiles)
{
    if ( !onlypiles )
        Deck::deck()->update();
    for (PileList::ConstIterator it = piles.begin(); it != piles.end(); ++it)
    {
        ( *it )->rescale();
    }
}

void DealerScene::setGameNumber(long gmn)
{
    // Deal in the range of 1 to INT_MAX.
    gamenumber = ((gmn < 1) ? 1 : ((gmn > 0x7FFFFFFF) ? 0x7FFFFFFF : gmn));
}

void DealerScene::addPile(Pile *p)
{
    piles.append(p);
}

void DealerScene::removePile(Pile *p)
{
    piles.removeAll(p);
}

DealerScene *Dealer::dscene() const
{
    return dynamic_cast<DealerScene*>( scene() );
}

void Dealer::setGameNumber(long gmn) { dscene()->setGameNumber(gmn); }
long Dealer::gameNumber() const { return dscene()->gameNumber(); }

void DealerScene::stopDemo()
{
    kDebug(11111) << "stopDemo " << waiting() << " " << stop_demo_next << endl;

    if (waiting()) {
        _waiting = 0;
        stop_demo_next = true;
        return;
    } else stop_demo_next = false;

    if (towait == (Card*)-1)
        towait = 0;

    if (towait) {
        towait->stopAnimation();
        towait->disconnect();
        towait = 0;
    }
    demotimer->stop();
    emit demoActive( false );
}

bool DealerScene::demoActive() const
{
    return (towait || demotimer->isActive());
}

void DealerScene::toggleDemo()
{
    if (demoActive()) {
        stopDemo();
    } else
        demo();
}

class CardPtr
{
 public:
    Card *ptr;
};

bool operator <(const CardPtr &p1, const CardPtr &p2)
{
    return ( p1.ptr->zValue() < p2.ptr->zValue() );
}

void DealerScene::won()
{
    if (_won)
        return;
    _won = true;

    for (PileList::Iterator it = piles.begin(); it != piles.end(); ++it)
    {
        ( *it )->relayoutCards(); // stop the timer
    }

    // update score, 'win' in demo mode also counts (keep it that way?)
    KConfigGroup kc(KGlobal::config(), scores_group);
    unsigned int n = kc.readEntry(QString("won%1").arg(_id),0) + 1;
    kc.writeEntry(QString("won%1").arg(_id),n);
    n = kc.readEntry(QString("winstreak%1").arg(_id),0) + 1;
    kc.writeEntry(QString("winstreak%1").arg(_id),n);
    unsigned int m = kc.readEntry(QString("maxwinstreak%1").arg(_id),0);
    if (n>m)
        kc.writeEntry(QString("maxwinstreak%1").arg(_id),n);
    kc.writeEntry(QString("loosestreak%1").arg(_id),0);

    // sort cards by increasing z
    QList<QGraphicsItem *> list = items();
    QList<CardPtr> cards;
    for (QList<QGraphicsItem *>::ConstIterator it=list.begin(); it!=list.end(); ++it)
        if ((*it)->type() == QGraphicsItem::UserType + Dealer::CardTypeId ) {
            CardPtr p;
            p.ptr = dynamic_cast<Card*>(*it);
            assert(p.ptr);
            cards.push_back(p);
        }
    qSort(cards);

    // disperse the cards everywhere
    QRectF can(0, 0, width(), height());
    QListIterator<CardPtr> it(cards);
    while (it.hasNext()) {
        CardPtr card = it.next();
        card.ptr->turn(true);
        QRectF p = card.ptr->sceneBoundingRect();
        p.moveTo( 0, 0 );
        qreal x, y;
        do {
            x = 3*width()/2 - KRandom::random() % int(width() * 2);
            y = 3*height()/2 - (KRandom::random() % int(height() * 2));
            p.moveTopLeft(QPointF(x, y));
        } while (can.intersects(p));

	card.ptr->moveTo( x, y, 0, DURATION_WON);
        connect(card.ptr, SIGNAL(stoped(Card*)), SLOT(waitForWonAnim(Card*)));
        setWaiting(true);
    }
}

MoveHint *DealerScene::chooseHint()
{
    if (hints.isEmpty())
        return 0;

    for (HintList::ConstIterator it = hints.begin(); it != hints.end(); ++it)
    {
        if ((*it)->pile()->target() && (*it)->dropIfTarget())
            return *it;
    }

    return hints[randseq.getLong(hints.count())];
}

void DealerScene::demo()
{
    if ( waiting() )
        return;

    if (stop_demo_next) {
        stopDemo();
        return;
    }
    stop_demo_next = false;
    unmarkAll();
    towait = (Card*)-1;
    clearHints();
    getHints();
    demotimer->stop();

    MoveHint *mh = chooseHint();
    if (mh) {
        // assert(mh->card()->source()->legalRemove(mh->card()));

        CardList empty;
        CardList cards = mh->card()->source()->cards();
        bool after = false;
        for (CardList::Iterator it = cards.begin(); it != cards.end(); ++it) {
            if (*it == mh->card())
                after = true;
            if (after)
                empty.append(*it);
        }

        assert(!empty.isEmpty());

        qreal *oldcoords = new qreal[2*empty.count()];
        int i = 0;

        for (CardList::Iterator it = empty.begin(); it != empty.end(); ++it) {
            Card *t = *it;
            t->stopAnimation();
            t->turn(true);
            oldcoords[i++] = t->realX();
            oldcoords[i++] = t->realY();
        }

        assert(mh->card()->source() != mh->pile());
        // assert(mh->pile()->legalAdd(empty));

        mh->card()->source()->moveCards(empty, mh->pile());

        i = 0;

        for (CardList::Iterator it = empty.begin(); it != empty.end(); ++it) {
            Card *t = *it;
            qreal x1 = oldcoords[i++];
            qreal y1 = oldcoords[i++];
            qreal x2 = t->realX();
            qreal y2 = t->realY();
            t->stopAnimation();
            t->setPos(x1, y1);
            t->moveTo(x2, y2, t->zValue(), DURATION_DEMO);
        }

        delete [] oldcoords;

        newDemoMove(mh->card());

    } else {
        Card *t = demoNewCards();
        if (t) {
            newDemoMove(t);
        } else if (isGameWon()) {
            emit gameWon(true);
            return;
        } else
            stopDemo();
    }

    Dealer::instance()->takeState();
}

Card *DealerScene::demoNewCards()
{
    return 0;
}

void DealerScene::newDemoMove(Card *m)
{
    //kDebug() << "newDemoMove " << m->name() << endl;
    setWaiting( true );
    towait = m;
    connect(m, SIGNAL(stoped(Card*)), SLOT(waitForDemo(Card*)));
}

void DealerScene::waitForDemo(Card *t)
{
    // kDebug() << "waitForDemo " << t->name() << endl;
    // Q_ASSERT( towait );
    if (t == (Card*)-1)
        return;
    /* if (towait != t)
       return; */
    t->disconnect();
    towait = 0;
    setWaiting( false );
    if ( !waiting() )
    {
        demotimer->setSingleShot( true );
        demotimer->start(250);
    }
}

bool DealerScene::isGameWon() const
{
    for (PileList::ConstIterator it = piles.begin(); it != piles.end(); ++it)
    {
        if (!(*it)->target() && !(*it)->isEmpty())
            return false;
    }
    return true;
}

bool DealerScene::isGameLost() const
{
    return false;
}

bool DealerScene::checkRemove( int, const Pile *, const Card *) const {
    return true;
}

bool DealerScene::checkAdd( int, const Pile *, const CardList&) const {
    return true;
}

void Dealer::setAnchorName(const QString &name)
{
    kDebug(11111) << "setAnchorname " << name << endl;
    ac = name;
}

QString Dealer::anchorName() const { return ac; }

void Dealer::wheelEvent( QWheelEvent *e )
{
    return; // Maren hits the wheel mouse function of the touch pad way too often :)
    qreal scaleFactor = pow((double)2, -e->delta() / (10*120.0));
    cardMap::self()->setWantedCardWidth( cardMap::self()->wantedCardWidth() / scaleFactor );
}

void DealerScene::countGame()
{
    if ( !_gameRecorded ) {
        kDebug(11111) << "counting game as played." << endl;
        KConfigGroup kc(KGlobal::config(), scores_group);
        unsigned int Total = kc.readEntry(QString("total%1").arg(_id),0);
        ++Total;
        kc.writeEntry(QString("total%1").arg(_id),Total);
        _gameRecorded = true;
    }
}

void DealerScene::countLoss()
{
    if ( _gameRecorded ) {
        // update score
        KConfigGroup kc(KGlobal::config(), scores_group);
        unsigned int n = kc.readEntry(QString("loosestreak%1").arg(_id),0) + 1;
        kc.writeEntry(QString("loosestreak%1").arg(_id),n);
        unsigned int m = kc.readEntry(QString("maxloosestreak%1").arg(_id),0);
        if (n>m)
            kc.writeEntry(QString("maxloosestreak%1").arg(_id),n);
        kc.writeEntry(QString("winstreak%1").arg(_id),0);
    }
}

QPointF DealerScene::maxPilePos() const
{
    QPointF maxp( 0, 0 );
    for (PileList::ConstIterator it = piles.begin(); it != piles.end(); ++it)
    {
        // kDebug() << ( *it )->objectName() << " " <<  ( *it )->pilePos() << " " << ( *it )->reservedSpace() << endl;
        // we call fabs here, because reserved space of -50 means for a pile at -1x-1 means it wants at least
        // 49 all together. Even though the real logic should be more complex, it's enough for our current needs
        maxp = QPointF( qMax( fabs( ( *it )->pilePos().x() + ( *it )->reservedSpace().width() ), maxp.x() ),
                        qMax( fabs( ( *it )->pilePos().y() + ( *it )->reservedSpace().height() ), maxp.y() ) );
    }
    return maxp;
}

void DealerScene::relayoutPiles()
{
    setSceneSize( Dealer::instance()->size());
}

void DealerScene::setSceneSize( const QSize &s )
{
//    kDebug() << "setSceneSize " << kBacktrace() << endl;
    setSceneRect( QRectF( 0, 0, s.width(), s.height() ) );

    QPointF defaultSceneRect = maxPilePos();

    kDebug() << "setSceneSize " << defaultSceneRect << " " << s << endl;
    Q_ASSERT( cardMap::self()->wantedCardWidth() > 0 );
    Q_ASSERT( cardMap::self()->wantedCardHeight() > 0 );

    qreal scaleX = s.width() / cardMap::self()->wantedCardWidth() / ( defaultSceneRect.x() + 1 );
    qreal scaleY = s.height() / cardMap::self()->wantedCardHeight() / ( defaultSceneRect.y() + 1 );
    kDebug() << "scaleY " << scaleY << " " << scaleX << endl;
    qreal n_scaleFactor = qMin(scaleX, scaleY);

    kDebug() << "scale " << n_scaleFactor << endl;
    cardMap::self()->setWantedCardWidth( int( n_scaleFactor * 10 * cardMap::self()->wantedCardWidth() ) );

    for (PileList::Iterator it = piles.begin(); it != piles.end(); ++it)
    {
        Pile *p = *it;
        if ( p->pilePos().x() < 0 || p->pilePos().y() < 0 )
            p->rescale();
        QSizeF ms( cardMap::self()->wantedCardWidth(),
                   cardMap::self()->wantedCardHeight() );
        // kDebug() << p->objectName() <<  " " << p->reservedSpace() << endl;
        if ( p->reservedSpace().width() > 10 )
            ms.setWidth( s.width() - p->x() );
        if ( p->reservedSpace().height() > 10 )
            ms.setHeight( s.height() - p->y() );
        if ( p->reservedSpace().width() < 0 )
            ms.setWidth( p->x() );
        Q_ASSERT( p->reservedSpace().height() > 0 ); // no such case yet

        //kDebug() << "setMaximalSpace " << ms << endl;
        // kDebug() << ms << " " << cardMap::self()->wantedCardHeight() << endl;
        Q_ASSERT( ms.width() >= cardMap::self()->wantedCardWidth() - 0.1 );
        Q_ASSERT( ms.height() >= cardMap::self()->wantedCardHeight() - 0.1 );

        p->setMaximalSpace( ms );
    }

    for (PileList::Iterator it = piles.begin(); it != piles.end(); ++it)
    {
        Pile *p = *it;
        if ( !p->isVisible() || ( p->reservedSpace().width() == 10 && p->reservedSpace().height() == 10 ) )
            continue;

        QRectF myRect( p->pos(), p->maximalSpace() );
        if ( p->reservedSpace().width() < 0 )
        {
            myRect.moveRight( p->x() + cardMap::self()->wantedCardWidth() );
            myRect.moveLeft( p->x() - p->maximalSpace().width() );
        }

        // kDebug() << p->objectName() << " " << p->spread() << " " << myRect << endl;

        for ( PileList::ConstIterator it2 = piles.begin(); it2 != piles.end(); ++it2 )
        {
            if ( *it2 == p || !( *it2 )->isVisible() )
                continue;

            QRectF pileRect( (*it2)->pos(), (*it2)->maximalSpace() );
            if ( (*it2)->reservedSpace().width() < 0 )
                pileRect.moveRight( (*it2)->x() );
            if ( (*it2)->reservedSpace().height() < 0 )
                pileRect.moveBottom( (*it2)->y() );


            // kDebug() << "compa " << p->objectName() << " " << myRect << " " << ( *it2 )->objectName() << " " << pileRect << " " << myRect.intersects( pileRect ) << endl;
            // if the growing pile intersects with another pile, we need to solve the conflict
            if ( myRect.intersects( pileRect ) )
            {
                // kDebug() << "compa " << p->objectName() << " " << myRect << " " << ( *it2 )->objectName() << " " << pileRect << " " << myRect.intersects( pileRect ) << endl;
                QSizeF pms = ( *it2 )->maximalSpace();

                if ( p->reservedSpace().width() != 10 )
                {
                    if ( ( *it2 )->reservedSpace().height() != 10 )
                    {
                        Q_ASSERT( ( *it2 )->reservedSpace().height() > 0 );
                        // if it's growing too, we win
                        pms.setHeight( qMin( myRect.top() - ( *it2 )->y() - 1, pms.height() ) );
                        // kDebug() << "1. reduced height of " << ( *it2 )->objectName() << endl;
                    } else // if it's fixed height, we loose
                        if ( p->reservedSpace().width() < 0 ) {
                            // this isn't made for two piles one from left and one from right both growing
                            Q_ASSERT( ( *it2 )->reservedSpace().width() == 10 );
                            myRect.setLeft( ( *it2 )->x() + cardMap::self()->wantedCardWidth() + 1);
                            // kDebug() << "2. reduced width of " << p->objectName() << endl;
                        } else {
                            myRect.setRight( ( *it2 )->x() - 1 );
                            // kDebug() << "3. reduced width of " << p->objectName() << endl;
                        }
                }

                if ( p->reservedSpace().height() != 10 )
                {
                    if ( ( *it2 )->reservedSpace().width() != 10 )
                    {
                        Q_ASSERT( ( *it2 )->reservedSpace().height() > 0 );
                        // if it's growing too, we win
                        pms.setWidth( qMin( myRect.right() - ( *it2 )->x() - 1, pms.width() ) );
                        kDebug() << "4. reduced width of " << ( *it2 )->objectName() << endl;
                    } else // if it's fixed height, we loose
                        if ( p->reservedSpace().height() < 0 ) {
                            // this isn't made for two piles one from left and one from right both growing
                            Q_ASSERT( ( *it2 )->reservedSpace().height() == 10 );
                            Q_ASSERT( false ); // TODO
                            myRect.setLeft( ( *it2 )->x() + cardMap::self()->wantedCardWidth() + 1);
                            kDebug() << "5. reduced height of " << p->objectName() << endl;
                        } else {
                            myRect.setBottom( ( *it2 )->y() - 1 );
                            // kDebug() << "6. reduced height of " << p->objectName() << endl;
                        }
                }

                Q_ASSERT( pms.width() >= cardMap::self()->wantedCardWidth() - 0.1 );
                Q_ASSERT( pms.height() >= cardMap::self()->wantedCardHeight() - 0.1 );
                ( *it2 )->setMaximalSpace( pms );
            }
        }
        Q_ASSERT( myRect.width() >= cardMap::self()->wantedCardWidth() - 0.1 );
        Q_ASSERT( myRect.height() >= cardMap::self()->wantedCardHeight() - 0.1 );

        p->setMaximalSpace( myRect.size() );
    }
}

void Dealer::resizeEvent( QResizeEvent *e )
{
    if ( e )
        QGraphicsView::resizeEvent(e);

    kDebug() << "resizeEvent " << wasShown() << endl;

    if ( !wasShown() )
        return;

#if 0
    foreach (QWidget *widget, QApplication::allWidgets())
        kDebug() << widget << " " << widget->objectName() << " " << widget->geometry() << endl;

    kDebug() << "resizeEvent " << size() << " " << e << " " << dscene() << " " << parent()->isVisible() << /* " " << kBacktrace() << */ endl;
#endif

    if ( !dscene() )
        return;

    dscene()->setSceneSize( size() );
}

#include "dealer.moc"
