/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <QtCore/QObject>
#include <QtCore/QPointF>

#include <QtGui/QFont>
#include <QtGui/QTextLayout>

#include "util/CharMetrics.h"
#include "util/CharMetricsF.h"
#include "TextColorStyle.h"
#include "global.h"

/**
  * TextRenderer renders arbitrarily scaled, individually aligned glyphs for a particular font without any
  * optimizations.
  *
  * This implementation directly renders the text using QPainter methods. Derived classes may override the drawChar
  * method and implement optimized drawing routines.
  *
  * It is important to distinguish between the size of a font and its scaled size. The font size is encapsulated in the
  * font argument passed to the constructor. The other mechanism for changing the final size of a character is by
  * arbitrarily scaling a font rendering. This is achieved by calling setScale, which performs a vector-based
  * scale transformation of the glyph rendering.
  *
  * Both integral and floating point char metrics are composed into this class and the actual metrics used may be
  * toggled via the setUseFloatMetrics() method.
  *
  * To optimize the rendering process, an array of QTextLayout's is pre-computed for each valid 7-bit character. These
  * are then painted in the drawChar method, rather than calling the QPainter::drawText method, which would otherwise
  * overly complicate things as well as creating a QTextLayout as well.
  */
class TextRenderer : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance with font, scale, and parent
//    TextRenderer(const QFont &font, qreal scale = 1.0, int resolution = constants::kDefaultResolution, QObject *parent = 0);
    TextRenderer(const QFont &font, qreal scale = 1.0, QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Core method responsible for drawing ch at pointF with textColorStyle using the supplied painter
    virtual void drawChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter) const;
    QFont font() const;                                         //!< Returns the currently defined font
    qreal height() const;                                       //!< Returns the scaled height in fractional pixels
//    int heightUnits() const;
    int resolution() const;                                     //!< Returns the current pixel resolution
    qreal scale() const;                                        //!< Returns the current scaling factor
    void setResolution(int resolution);                         //!< Sets the pixel resolution to resolution; does nothing if resolution < 1
    void setUseFloatMetrics(bool enable = true);                //!< If enable is true, then floating point metrics will be used when rendering characters
    QSizeF sizeF() const;                                       //!< Returns the scaled size per character in pixels (floating point, via CharMetricsF)
    QSize size() const;                                         //!< Returns the scaled size per character in pixels (integer units, via CharMetrics)
    qreal unscaledHeight() const;                               //!< Returns the unscaled height per character in pixels (via CharMetrics)
    qreal unscaledWidth() const;                                //!< Returns the unscaled width per character in pixels (via CharMetrics)
    QSizeF unscaledSize() const;                                //!< Returns the unscaled size per character in pixels (via CharMetrics)
    bool useFloatMetrics() const;                               //!< Returns true if floating point metrics are to be used when rendering; false otherwise
    qreal width() const;                                        //!< Returns the scaled width per character in pixels

public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setFont(const QFont &font);                            //!< Sets the font to font and emits fontChanged
    void setScale(qreal scale);                                 //!< Sets the scale to scale and emits scaleChanged

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void fontChanged();                                         //!< This signal is emitted when the font has changed; declared virtual for derived classes to intercept
    void scaleChanged();                                        //!< This signal is emitted when the scale has changed

private:
    QFont font_;
    qreal scale_;
    int resolution_;
    CharMetricsF charMetricsF_;
    CharMetrics charMetrics_;
    QTextLayout textLayouts_[94];

    bool useFloatMetrics_;
};

#endif // TEXTRENDERER_H
