/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "NativeMsaView.h"
#include "../../graphics/TextPixmapRenderer.h"
#include "../painting/NativeRenderEngine.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
NativeMsaView::NativeMsaView(QWidget *parent) : AbstractMsaView(parent)
{
    renderEngine_ = new NativeRenderEngine(new TextPixmapRenderer(font(), 1., this), this);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * @returns IRenderEngine *
  */
IRenderEngine *NativeMsaView::renderEngine() const
{
    return renderEngine_;
}
