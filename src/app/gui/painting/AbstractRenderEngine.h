/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTPAINTENGINE_H
#define ABSTRACTPAINTENGINE_H

#include "IRenderEngine.h"
#include "../../core/macros.h"

class AbstractRenderEngine : public IRenderEngine
{
protected:
    AbstractRenderEngine(QObject *parent = nullptr) : IRenderEngine(parent), lineWidth_(1)
    {
    }

    virtual int lineWidth() const
    {
        return lineWidth_;
    }

    virtual void setLineWidth(int lineWidth)
    {
        ASSERT_X(lineWidth > 0, "lineWidth must be positive");
        lineWidth_ = lineWidth;
    }

    int lineWidth_;
};

#endif // ABSTRACTPAINTENGINE_H
