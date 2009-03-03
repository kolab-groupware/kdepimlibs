/**
 * Copyright (C)  2006  Laurent Montel <montel@kde.org>
 * Copyright (C)  2008  Thomas McGuire <mcguire@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "emailquotehighlighter.h"

#include "textedit.h"

namespace PimTextEdit {

class EMailQuoteHighlighter::EMailQuoteHighlighterPrivate
{
public:
    QColor col1, col2, col3, misspelledColor;
    bool spellCheckingEnabled;
    TextEdit *parent;
};

EMailQuoteHighlighter::EMailQuoteHighlighter( TextEdit *textEdit,
                                              const QColor &normalColor,
                                              const QColor &quoteDepth1,
                                              const QColor &quoteDepth2,
                                              const QColor &quoteDepth3,
                                              const QColor &misspelledColor )
    : Highlighter( textEdit ),
      d( new EMailQuoteHighlighterPrivate() )
{
    Q_UNUSED( normalColor );
    // Don't automatically disable the spell checker, for example because there
    // are too many misspelled words. That would also disable quote highlighting.
    // FIXME: disable this spell checking!
    setAutomatic( false );

    setActive( true );
    d->col1 = quoteDepth1;
    d->col2 = quoteDepth2;
    d->col3 = quoteDepth3;
    d->misspelledColor = misspelledColor;
    d->spellCheckingEnabled = false;
    d->parent = textEdit;
}

EMailQuoteHighlighter::~EMailQuoteHighlighter()
{
}

void EMailQuoteHighlighter::setQuoteColor( const QColor &normalColor,
                                           const QColor &quoteDepth1,
                                           const QColor &quoteDepth2,
                                           const QColor &quoteDepth3,
                                           const QColor &misspelledColor )
{
    Q_UNUSED( normalColor );
    d->col1 = quoteDepth1;
    d->col2 = quoteDepth2;
    d->col3 = quoteDepth3;
    d->misspelledColor = misspelledColor;
}

void EMailQuoteHighlighter::toggleSpellHighlighting( bool on )
{
    if ( on != d->spellCheckingEnabled ) {
        d->spellCheckingEnabled = on;
        rehighlight();
    }
}

void EMailQuoteHighlighter::highlightBlock( const QString & text )
{
    QString simplified = text;
    simplified = simplified.replace( QRegExp( QLatin1String( "\\s" ) ), QString() )
                           .replace( QLatin1Char( '|' ), QLatin1String(">") );
    while ( simplified.startsWith( QLatin1String(">>>>") ) )
        simplified = simplified.mid( 3 );
    if ( simplified.startsWith( QLatin1String(">>>") ) ||
         simplified.startsWith( QLatin1String("> > >") ) )
        setFormat( 0, text.length(), d->col1 );
    else if ( simplified.startsWith( QLatin1String(">>") ) ||
              simplified.startsWith( QLatin1String("> >") ) )
        setFormat( 0, text.length(), d->col2 );
    else if ( simplified.startsWith( QLatin1String(">") ) )
        setFormat( 0, text.length(), d->col3 );
    /*else if ( !d->parent->quotePrefixName().simplified().isEmpty() &&
              text.startsWith( d->parent->quotePrefixName() ) )
        setFormat( 0, text.length(), d->col1 );*/  //FIXME !!!!!
    else
    {
        if ( d->spellCheckingEnabled )
            Highlighter::highlightBlock( text );
    }
    setCurrentBlockState( 0 );
}

void EMailQuoteHighlighter::unsetMisspelled( int start,  int count )
{
    Q_UNUSED( start )
    Q_UNUSED( count )
}

void EMailQuoteHighlighter::setMisspelled( int start, int count )
{
    setMisspelledColor( d->misspelledColor );
    Sonnet::Highlighter::setMisspelled( start, count );
}

}
