/* -*- C++ -*-
 * 
 * patience -- main program
 *   Copyright (C) 1995  Paul Olav Tvete
 *
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
 *
 *
 * Heavily modified by Mario Weilguni <mweilguni@sime.com>
 *
 */

#ifndef __PWIDGET__H__
#define __PWIDGET__H__

#include <qwidget.h>
#include <qpopupmenu.h>
#include <qkeycode.h>

#include <kmenubar.h>
#include <ktmainwindow.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kapp.h>

#include "card.h"
#include "klondike.h"
#include "microsol.h"
#include "computation.h"
#include "grandf.h"
#include "ten.h"
#include "idiot.h"
#include "napoleon.h"
#include "mod3.h"
#include "freecell.h"

// type identifier for games
#define PT_NONE		0
#define PT_KLONDIKE	1
#define PT_MSOLITAIRE	2
#define PT_IDIOT	3
#define PT_GRANDFATHER	4
#define PT_NAPOLEON	5
#define PT_TEN		6
#define PT_COMPUTATION	7
#define PT_MOD3		8
#define PT_FREECELL	9

class pWidget: public KTMainWindow {
  Q_OBJECT
public:
  pWidget(const char *name=0 );

protected:
  virtual void focusInEvent(QFocusEvent *);

private:
  void help();
  void helpRules();
  void about();
  void setDefaultType();
  int  getDefaultType();
  void actionNewGame(int);
  void setBackSide(int);

private slots:
  void action(int);
  void slotToolbarChanged();

private:
  KMenuBar *m;
  KToolBar *tb;
  KStatusBar *sb;
  dealer* dill;
  int type;
  bool inInit;
};

#endif
