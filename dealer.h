#ifndef _DEALER_H_
#define _DEALER_H_

#include <qvaluelist.h>
#include <qcanvas.h>
#include "card.h"
#include "pile.h"
#include "hint.h"
#include <krandomsequence.h>

class KMainWindow;
class Dealer;
class DealerInfo;
class KAction;
class KSelectAction;
class KToggleAction;
class KPixmap;

class DealerInfoList {
public:
    static DealerInfoList *self();
    void add(DealerInfo *);

    const QValueList<DealerInfo*> games() const { return list; }
private:
    QValueList<DealerInfo*> list;
    static DealerInfoList *_self;
};

class DealerInfo {
public:
    DealerInfo(const char *_name, int _index)
        : name(_name),
          gameindex(_index)
{
    DealerInfoList::self()->add(this);
}
    const char *name;
    uint gameindex;
    virtual Dealer *createGame(KMainWindow *parent) = 0;
};

class CardState;

typedef QValueList<CardState> CardStateList;

struct State
{
    CardStateList cards;
    QByteArray gameData;
};

/***************************************************************

  Dealer -- abstract base class of all varieties of patience

***************************************************************/
class Dealer: public QCanvasView
{
    Q_OBJECT

public:

    Dealer( KMainWindow* parent = 0, const char* name = 0 );
    virtual ~Dealer();

    void enlargeCanvas(QCanvasRectangle *c);
    void resetSize(const QSize &size);
    void setGameNumber(long gmn);
    long gameNumber() const;

    virtual bool isGameWon() const;

    void setViewSize(const QSize &size);
    virtual QSize sizeHint() const;

    void addPile(Pile *p);
    void removePile(Pile *p);

    virtual bool checkRemove( int checkIndex, const Pile *c1, const Card *c) const;
    virtual bool checkAdd   ( int checkIndex, const Pile *c1, const CardList& c2) const;
    virtual Card *demoNewCards();

    virtual void setupActions();

    bool demoActive() const;

    void drawPile(KPixmap &, Pile *p, bool selected);

    QColor midColor() const { return _midcolor; }
    void setBackgroundPixmap(const QPixmap &background, const QColor &midcolor);

    void saveGame(QDataStream &s);
    void openGame(QDataStream &s);

    void setGameId(int id) { _id = id; }

    void setTakeTargetForHints(bool e) { takeTargets = e; }
    bool takeTargetForHints() const { return takeTargets; }

    int freeCells() const;

public slots:

    // restart is pure virtual, so we need something else
    virtual void startNew();
    void undo();
    virtual void takeState();
    virtual bool startAutoDrop();
    void hint();
    void slotTakeState(Card *c);

signals:
    void undoPossible(bool poss);
    void gameWon(bool withhelp);
    void saveGame(); // emergency
    void gameInfo(const QString &info);

public slots:
    virtual void demo();
    void waitForDemo(Card *);
    void toggleDemo();
    virtual void stopDemo();
    void waitForAutoDrop(Card *);

protected:

    enum { None = 0, Hint = 1, Demo = 2, Redeal = 4 } Actions;

    void setActions(int actions) { myActions = actions; }
    int actions() const { return myActions; }

    virtual void restart() = 0;

    virtual void contentsMousePressEvent(QMouseEvent* e);
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    virtual void contentsMouseReleaseEvent( QMouseEvent* );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent* );
    virtual void viewportResizeEvent ( QResizeEvent * );

    void unmarkAll();
    void mark(Card *c);
    Pile *findTarget(Card *c);
    virtual void cardClicked(Card *);
    virtual void pileClicked(Pile *);
    virtual void cardDblClicked(Card *);
    void won();

    virtual void getHints();
    void newHint(MoveHint *mh);
    void clearHints();
    // it's not const because it changes the random seed
    virtual MoveHint *chooseHint();

    KMainWindow *parent() const;

    bool waiting() const { return _waiting != 0; }
    void setWaiting(bool w);

protected:
    PileList piles;

    State *getState();
    void setState(State *);

    void loadCardState( QDataStream& s, CardState& l, CardList &toload);

    // reimplement this to add game-specific information in the state structure
    virtual void getGameState( QDataStream & ) {}
    // reimplement this to use the game-specific information from the state structure
    virtual void setGameState( QDataStream & ) {}

    virtual void newDemoMove(Card *m);

    bool moved;
    CardList movingCards;
    QCanvasItemList marked;
    QPoint moving_start;
    Dealer( Dealer& );  // don't allow copies or assignments
    void operator = ( Dealer& );  // don't allow copies or assignments
    QCanvas myCanvas;
    QSize maxsize;
    QSize viewsize;
    QList<State> undoList;
    long gamenumber;
    QValueList<MoveHint*> hints;
    Card *towait;
    QTimer *demotimer;
    int myActions;

    KToggleAction *ademo;
    KAction *ahint, *aredeal;

    KRandomSequence randseq;
    QColor _midcolor;
    int _id;
    bool takeTargets;
    bool _won;
    int _waiting;
};

#endif
