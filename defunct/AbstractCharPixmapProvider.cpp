/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AbstractCharPixmapProvider.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
AbstractCharPixmapProvider::AbstractCharPixmapProvider(QObject *parent) : QObject(parent)
{
    cachingEnabled_ = true;
}

/**
  */
AbstractCharPixmapProvider::~AbstractCharPixmapProvider()
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * If caching is disabled, then returns the pixmap returned by the renderGlyph method. If caching is enabled, render
  * the glyph for this character and color combination if it does not exist in the cache; otherwise, return the cached
  * glyph.
  *
  * @param character [char]
  * @param color [const QColor &]
  * @returns QPixmap
  */
QPixmap AbstractCharPixmapProvider::glyph(char character, const QColor &color)
{
    if (cachingEnabled_)
    {
        QString key = glyphKey(character, color);
        if (!cachedGlyphs_.contains(key))
            cachedGlyphs_.insert(key, renderGlyph(character, color));

        return cachedGlyphs_.value(key);
    }

    // Caching is disabled
    return renderGlyph(character, color);
}

/**
  * @returns bool
  */
bool AbstractCharPixmapProvider::isCachingEnabled() const
{
    return cachingEnabled_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * @param cachingEnabled [bool]
  */
void AbstractCharPixmapProvider::setCachingEnabled(bool cachingEnabled)
{
    cachingEnabled_ = cachingEnabled;
}

/**
  * Removes all the glyphs from the cache.
  */
void AbstractCharPixmapProvider::clearCache()
{
    cachedGlyphs_.clear();

    emit cacheCleared();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Currently, each character + color combination is converted to a string by combining its character and color name.
  *
  * @param character [char]
  * @param color [QColor]
  * @returns QString
  */
QString AbstractCharPixmapProvider::glyphKey(char character, QColor color)
{
    return character + color.name();
}
