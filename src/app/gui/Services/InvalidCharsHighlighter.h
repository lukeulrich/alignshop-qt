/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INVALIDCHARSHIGHLIGHTER_H
#define INVALIDCHARSHIGHLIGHTER_H

#include <QtCore/QRegExp>
#include <QtGui/QSyntaxHighlighter>

#include "../../core/global.h"

/**
  * Highlights all characters that do not match a user-specified set of valid characters. All characters are accepted
  * if the valid characters has not been set.
  *
  * Matching is case-insensitive.
  */
class InvalidCharsHighligher : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    InvalidCharsHighligher(QTextEdit *textEditParent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool stringIsValid(const QString &string) const;
    QString validCharacters() const;
    QRegExp regExpForInvalidCharacters() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setValidCharacters(const QString &validCharacters);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void highlightBlock(const QString &text);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void rebuildInvalidCharactersRegExp();
    QTextCharFormat invalidFormat() const;

    // ------------------------------------------------------------------------------------------------
    // Private members
    QString validCharacters_;
    QRegExp invalidRegExp_;
};

#endif // INVALIDCHARSHIGHLIGHTER_H
