/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef CHARPIXMAPPROVIDER_H
#define CHARPIXMAPPROVIDER_H

#include <QtCore/QHash>

#include <QtGui/QPixmap>

/**
  * AbstractCharPixmapProvider defines the abstract interface for providing glyph characters of a requested color.
  *
  * This base class implements basic caching for each character and color combination. Caching may be turned on/off with
  * the setCachingEnabled method and by default is on. Concrete implementations should define the renderGlyph method to
  * return a valid QPixmap.
  */
class AbstractCharPixmapProvider : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit AbstractCharPixmapProvider(QObject *parent = 0);       //!< Trivial constructor, caching defaults to true
    virtual ~AbstractCharPixmapProvider();                          //!< Trivial destructor, but virtual because this class is an interface

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QPixmap glyph(char character, const QColor &color);             //!< Returns a transparent pixmap containing the glyph for character in color; should be premultipled alpha format
    bool isCachingEnabled() const;                                  //!< Returns true if caching is enabled; false otherwise

public slots:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setCachingEnabled(bool cachingEnabled);                    //!< Sets the caching to cachingEnabled
    void clearCache();                                              //!< Clears all glyphs from the cache

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void cacheCleared();                                            //!< Emitted when the cache is cleared

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Pure virtual method that renders and returns the glyph of character in color
    virtual QPixmap renderGlyph(char character, const QColor &color) = 0;

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    QString glyphKey(char character, QColor color);                 //!< Returns a unique string key for any combination of character and color

    QHash<QString, QPixmap> cachedGlyphs_;
    bool cachingEnabled_;
};

#endif // CHARPIXMAPPROVIDER_H
