/*
 *  Copyright (C) 2010 Parker Coates <parker.coates@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of 
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PILEUTILS_H
#define PILEUTILS_H
class Card;

#include <QList>


bool isSameSuitAscending( const QList<Card*> & cards );
bool isSameSuitDescending( const QList<Card*> & cards );
bool isAlternateColorDescending( const QList<Card*> & cards );

bool checkAddSameSuitAscendingFromAce( const QList<Card*> & oldCards, const QList<Card*> & newCards );
bool checkAddAlternateColorDescending( const QList<Card*> & oldCards, const QList<Card*> & newCards );
bool checkAddAlternateColorDescendingFromKing( const QList<Card*> & oldCards, const QList<Card*> & newCards );

#endif