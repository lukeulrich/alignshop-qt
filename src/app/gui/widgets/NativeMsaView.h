/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef NATIVEMSAVIEW_H
#define NATIVEMSAVIEW_H

#include "AbstractMsaView.h"
#include "../../core/global.h"

class QPainter;

class NativeRenderEngine;

/**
  * NativeMsaView encapsulates the software rendering (raster) implementation of a MsaView.
  */
class NativeMsaView : public AbstractMsaView
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    NativeMsaView(QWidget *parent = nullptr);                    //!< Trivial constructor

    // ------------------------------------------------------------------------------------------------
    // Reimplemented methods
    virtual IRenderEngine *renderEngine() const;

private:
    NativeRenderEngine *renderEngine_;
};

#endif // NATIVEMSAVIEW_H
