/*
     patience -- main program
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

#include <kapp.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "version.h"
#include "pwidget.h"

static const char *description = I18N_NOOP("KDE Patience Game");

int main( int argc, char **argv )
{
    KAboutData aboutData( "kpat", I18N_NOOP("KPatience"),
                          KPAT_VERSION, description, KAboutData::License_GPL,
                          "(c) 1995, Paul Olav Tvete");
    aboutData.addAuthor("Paul Olav Tvete");
    aboutData.addAuthor("Mario Weilguni",0,"mweilguni@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0,"ettrich@kde.org");
    aboutData.addAuthor("Rodolfo Borges",0,"barrett@labma.ufrj.br");
    aboutData.addAuthor("Peter H. Ruegg",0,"kpat@incense.org");
    aboutData.addAuthor("Michael Koch", i18n("Bug fixes"), "koch@kde.org");
    aboutData.addAuthor("Stephan Kulow", i18n("Port to KDE2 and bug fixes"),
                        "coolo@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication a;

    pWidget *p = new pWidget("pwidget");
    p->show();

    a.setMainWidget(p);
    int r = a.exec();
    return r;
}
