/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef HSVA_H
#define HSVA_H

#include <QtCore/QtGlobal>
#include <QtGui/QColor>

/**
  * HSVA contains the raw data for representing a color with hue, saturation, value, and alpha components.
  */
struct HSVA
{
    qreal h_;
    qreal s_;
    qreal v_;
    qreal a_;

    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an HSVA from its corresponding QColor representation
    HSVA(const QColor &color = QColor())
    {
        color.getHsvF(&h_, &s_, &v_, &a_);
    }

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Helper method that returns the QColor equivalent of the hue, saturation, value, and alpha values
    QColor toColor() const
    {
        return QColor::fromHsvF(h_, s_, v_, a_);
    }
};

Q_DECLARE_TYPEINFO(HSVA, Q_PRIMITIVE_TYPE);

#endif // HSVA_H
