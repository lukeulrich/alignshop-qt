/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef BIOSYMBOL_H
#define BIOSYMBOL_H

#include <QtCore/QMetaType>
#include <QtCore/QSet>
#include <QtCore/QString>

/**
  * BioSymbol associates a single arbitrary biological symbol with a unique set of characters along with a threshold
  * (i.e. percentage) that these characters must surpass in quantity for the representative symbol to be deemed
  * significant.
  */
class BioSymbol
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    //! Construct an instance of this class with symbol, characters, and threshold; default arguments defined for default constructor purposes (e.g. hash)
    BioSymbol(char symbol = 0, const QByteArray &characters = QByteArray(), qreal threshold = 0);
    BioSymbol(const QString &label, char symbol = 0, const QByteArray &characters = QByteArray(), qreal threshold = 0);

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const BioSymbol &other) const;                      //!< Returns true if other is equal to this BioSymbol; false otherwise
    bool operator!=(const BioSymbol &other) const;                      //!< Returns true if other is not equal to this; false otherwise

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void addCharacters(const QByteArray &characters);                   //!< Associate each of the letters in characters with this symbol
    QByteArray characters() const;                                      //!< Returns the currently defined characters as a QByteArray
    QSet<char> characterSet() const;                                    //!< Returns the set of currently defined characters
    bool hasCharacter(char character) const;                            //!< Returns true if character is associated with this symbol; false otherwise
    QString label() const;
    void removeCharacters(const QByteArray &characters);                //!< Removes characters from the set associated with this symbol
    void setCharacters(const QByteArray &characters);                   //!< Replaces the currently associated set of internal characters with characters
    void setLabel(const QString &newLabel);
    void setSymbol(char symbol);                                        //!< Sets the internal symbol to symbol
    void setThreshold(qreal threshold);                                 //!< Sets the threshold value to threshold
    char symbol() const;                                                //!< Returns the symbol
    qreal threshold() const;                                            //!< Returns the threshold

private:
    QSet<char> stringToCharSet(const QByteArray &string) const;         //!< Returns the unique set of characters in string

    QString label_;
    QSet<char> characterSet_;
    qreal threshold_;
    char symbol_;

    friend QDataStream &operator<<(QDataStream &out, const BioSymbol &bioSymbol);
    friend QDataStream &operator>>(QDataStream &in, BioSymbol &bioSymbol);
};

Q_DECLARE_METATYPE(BioSymbol)

QDataStream &operator<<(QDataStream &out, const BioSymbol &bioSymbol);
QDataStream &operator>>(QDataStream &in, BioSymbol &bioSymbol);


#endif // BIOSYMBOL_H
