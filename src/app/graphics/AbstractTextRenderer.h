/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTTEXTRENDERER_H
#define ABSTRACTTEXTRENDERER_H

#include <QtGui/QTextLayout>
#include "../core/global.h"

class QPainter;
class QPointF;
class AbstractCharPixelMetrics;
struct TextColorStyle;

/**
  * AbstractTextRenderer defines and partially implements the interface for rendering arbitrarily scaled, individually
  * aligned 7-bit ASCII glyphs for a particular font.
  *
  * This implementation directly renders the text using QPainter methods. Derived classes may override the drawChar
  * method and implement optimized drawing routines.
  *
  * It is important to distinguish between the point size of a font and its scaled size. The font size is encapsulated
  * in the font argument passed to the constructor. The other mechanism for changing the final size of a character is by
  * arbitrarily scaling a font rendering. This is achieved by calling setScale, which performs a vector-based
  * scale transformation of the glyph rendering.
  *
  * A major differential factor between downstream classes is the specific CharPixelMetrics implementation to be used
  * for determining the rendering positions. Thus, it is the inherited classes responsibility to supply this via
  * the charPixelMetrics method. In this manner, the same drawing routines may be used for both integral and floating
  * point character metrics.
  *
  * To optimize the rendering process, an array of QTextLayout's is pre-computed for each valid 7-bit character. These
  * are then painted in the drawChar method, rather than calling the QPainter::drawText method, which would otherwise
  * overly complicate things in addition to creating a QTextLayout.
  */
class AbstractTextRenderer : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Constructs an instance with font and parent
    AbstractTextRenderer(const QFont &font, QObject *parent = nullptr);
    virtual ~AbstractTextRenderer();                            //!< Trivial destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Returns a const pointer to the specific CharPixelMetrics instance
    virtual const AbstractCharPixelMetrics *charPixelMetrics() const = 0;
    qreal baseline() const;                                     //!< Returns the scaled baseline
    //! Core method responsible for drawing ch at pointF with textColorStyle using the supplied painter
    virtual void drawChar(const QPointF &pointF, const char ch, const TextColorStyle &textColorStyle, QPainter *painter);
    QFont font() const;                                         //!< Returns the currently defined font
    qreal height() const;                                       //!< Returns the scaled height in fractional pixels
    qreal scale() const;                                        //!< Returns the current scaling factor
    QSizeF size() const;                                        //!< Returns the scaled size per character in pixels (floating point, via CharMetricsF)
    qreal width() const;                                        //!< Returns the scaled width per character in pixels


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setFont(const QFont &newFont);                         //!< Sets the font to newFont and emits fontChanged
    void setScale(qreal scale);                                 //!< Sets the scale to scale and emits scaleChanged


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void fontChanged();                                         //!< This signal is emitted when the font has changed; declared virtual for derived classes to intercept
    void scaleChanged();                                        //!< This signal is emitted when the scale has changed


protected:
    virtual AbstractCharPixelMetrics *charPixelMetrics() = 0;   //! Returns a pointer to the specific CharPixelMetrics instance


private:
    QTextLayout textLayouts_[94];
};

#endif // ABSTRACTTEXTRENDERER_H
