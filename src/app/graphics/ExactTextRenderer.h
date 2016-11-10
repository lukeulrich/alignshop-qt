/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef EXACTTEXTRENDERER_H
#define EXACTTEXTRENDERER_H

#include "AbstractTextRenderer.h"
#include "CharPixelMetricsF.h"

/**
  * ExactTextRenderer implements the AbstractTextRenderer interface and provides floating point accurate rendering.
  */
class ExactTextRenderer : public AbstractTextRenderer
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Constructs an instance with font, scale, and parent
    ExactTextRenderer(const QFont &font, qreal scale = 1., QObject *parent = 0)
        : AbstractTextRenderer(font, parent),
          charPixelMetricsF_(font, scale)
    {
    }

    /**
      * @returns AbstractCharPixelMetrics *
      */
    virtual const AbstractCharPixelMetrics *charPixelMetrics() const
    {
        return &charPixelMetricsF_;
    }


protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented pure virtual methods
    /**
      * @returns AbstractCharPixelMetrics *
      */
    virtual AbstractCharPixelMetrics *charPixelMetrics()
    {
        return &charPixelMetricsF_;
    }


private:
    CharPixelMetricsF charPixelMetricsF_;
};

#endif // EXACTTEXTRENDERER_H
