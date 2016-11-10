/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BASICTEXTRENDERER_H
#define BASICTEXTRENDERER_H

#include "AbstractTextRenderer.h"
#include "CharPixelMetrics.h"
#include "../core/global.h"

/**
  * BasicTextRenderer implements the AbstractTextRenderer interface and provides integer accurate rendering.
  */
class BasicTextRenderer : public AbstractTextRenderer
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance with font, scale, and parent
    BasicTextRenderer(const QFont &font, qreal scale = 1., QObject *parent = nullptr)
        : AbstractTextRenderer(font, parent), charPixelMetrics_(font, scale)
    {
    }

    /**
      * @returns AbstractCharPixelMetrics *
      */
    virtual const AbstractCharPixelMetrics *charPixelMetrics() const
    {
        return &charPixelMetrics_;
    }


protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented pure virtual methods
    /**
      * @returns AbstractCharPixelMetrics *
      */
    virtual AbstractCharPixelMetrics *charPixelMetrics()
    {
        return &charPixelMetrics_;
    }

private:
    CharPixelMetrics charPixelMetrics_;
};

#endif // BASICTEXTRENDERER_H
