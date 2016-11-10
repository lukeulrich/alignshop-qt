/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSYMBOL_H
#define BIOSYMBOL_H

#include <QtCore/QSet>

/**
  * BioSymbol associates a single biological symbol with a unique set of characters along with a threshold (i.e.
  * percentage) that these characters must surpass in quantity for the representative symbol to be deemed significant.
  */
class BioSymbol
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance of this class with symbol, characters, and threshold; default arguments defined for default constructor purposes (e.g. hash)
    BioSymbol(char symbol = 0, const QString &characters = QString(), qreal threshold = 0);

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const BioSymbol &other) const;                      //!< Returns true if all private members are equivalent; false otherwise
    bool operator!=(const BioSymbol &other) const;                      //!< Returns true if one or more private members are not equivalent; false otherwise

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void addCharacters(const QString &characters);                      //!< Associate each of the letters in characters with this symbol
    QString characters() const;                                         //!< Returns the currently defined characters as a QString
    QSet<char> characterSet() const;                                    //!< Returns the set of currently defined characters
    bool hasCharacter(char character) const;                            //!< Returns true if character is associated with this symbol; false otherwise
    void removeCharacters(const QString &characters);                   //!< Removes characters from the set associated with this symbol
    void setCharacters(const QString &characters);                      //!< Replaces the currently associated set of internal characters with characters
    void setSymbol(char symbol);                                        //!< Sets the internal symbol to symbol
    void setThreshold(qreal threshold);                                 //!< Sets the threshold value to threshold
    char symbol() const;                                                //!< Returns the symbol
    qreal threshold() const;                                            //!< Returns the threshold

private:
    QSet<char> stringToCharSet(const QString &string) const;            //!< Returns the unique set of characters in string

    QSet<char> characterSet_;
    qreal threshold_;
    char symbol_;
};

#endif // BIOSYMBOL_H
