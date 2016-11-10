/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QTextCharFormat>

#include "InvalidCharsHighlighter.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
InvalidCharsHighligher::InvalidCharsHighligher(QTextEdit *textEditParent)
    : QSyntaxHighlighter(textEditParent)
{
    invalidRegExp_.setCaseSensitivity(Qt::CaseInsensitive);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
bool InvalidCharsHighligher::stringIsValid(const QString &string) const
{
    if (invalidRegExp_.isEmpty())
        return true;

    return invalidRegExp_.indexIn(string) == -1;
}

QString InvalidCharsHighligher::validCharacters() const
{
    return validCharacters_;
}

QRegExp InvalidCharsHighligher::regExpForInvalidCharacters() const
{
    return invalidRegExp_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
void InvalidCharsHighligher::setValidCharacters(const QString &validCharacters)
{
    validCharacters_ = validCharacters;
    rebuildInvalidCharactersRegExp();
    rehighlight();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
void InvalidCharsHighligher::highlightBlock(const QString &text)
{
    if (invalidRegExp_.isEmpty())
        return;

    QTextCharFormat format = invalidFormat();

    int index = text.indexOf(invalidRegExp_);
    while (index >= 0)
    {
        int invalidLength = invalidRegExp_.matchedLength();
        setFormat(index, invalidLength, format);
        index = text.indexOf(invalidRegExp_, index + invalidLength);
    }
}


// ------------------------------------------------------------------------------------------------
// Private methods
void InvalidCharsHighligher::rebuildInvalidCharactersRegExp()
{
    if (validCharacters_.isEmpty())
    {
        invalidRegExp_.setPattern("");
        return;
    }

    QString regexpChars = validCharacters_;
    regexpChars.replace('-', "\\-");

    invalidRegExp_.setPattern(QString("[^%1]").arg(regexpChars));
}

QTextCharFormat InvalidCharsHighligher::invalidFormat() const
{
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setForeground(QColor(196, 0, 0));
    return format;
}
