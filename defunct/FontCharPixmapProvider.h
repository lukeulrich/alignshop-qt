/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef FONTCHARPIXMAPPROVIDER_H
#define FONTCHARPIXMAPPROVIDER_H

#include "AbstractCharPixmapProvider.h"

#include <QtGui/QFont>
#include <QtGui/QFontMetricsF>

/**
  * FontCharPixmapProvider is a concrete implementation of the AbstractCharPixmapProvider interface capable of
  * rendering arbitrarily scaled glyphs for a particular font.
  *
  * It is important to distinguish between the size of a font and its scaled size. The font size is encapsulated in the
  * font argument passed to the constructor. The other mechanism for changing the final size of a character is by
  * arbitrarily scaling a font rendering. This is achieved by calling setScale, which performs a vector-based
  * scale transformation of the glyph rendering.
  */
class FontCharPixmapProvider : public AbstractCharPixmapProvider
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance with font, scale, and parent
    FontCharPixmapProvider(const QFont &font, qreal scale = 1.0, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QFont font() const;                                         //!< Returns the currently defined font
    qreal height() const;                                       //!< Returns the scaled height in pixels
    qreal scale() const;                                        //!< Returns the current scaling factor
    qreal unscaledHeight() const;                               //!< Returns the unscaled height in pixels
    qreal unscaledWidth(char character) const;                  //!< Returns the unscaled width of character in pixels
    qreal width(char character) const;                          //!< Returns the scaled width of character in pixels

public slots:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setFont(const QFont &font);                            //!< Sets the font to font and emits fontChanged
    void setScale(qreal scale);                                 //!< Sets the scale to scale and emits scaleChanged

signals:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void fontChanged();                                         //!< This signal is emitted when the font has changed
    void scaleChanged();                                        //!< This signal is emitted when the scale has changed

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    QPixmap renderGlyph(char character, const QColor &color);   //!< Renders and returns the glyph of character in color

private:
    QFont font_;
    qreal scale_;
    QFontMetricsF fontMetricsF_;
};

#endif // FONTCHARPIXMAPPROVIDER_H
