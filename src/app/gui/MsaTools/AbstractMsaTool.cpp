/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractMsaTool.h"
#include "../../core/macros.h"
#include "../widgets/AbstractMsaView.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param msaView [AbstractMsaView *]
  * @param parent [QObject *]
  */
AbstractMsaTool::AbstractMsaTool(AbstractMsaView *msaView, QObject *parent)
    : IMsaTool(parent), msaView_(msaView)
{
    ASSERT(msaView != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AbstractMsaView *
  */
AbstractMsaView *AbstractMsaTool::msaView() const
{
    return msaView_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void AbstractMsaTool::activate()
{
    emit activated();
    msaView_->viewport()->update();
}

/**
  */
void AbstractMsaTool::deactivate()
{
    emit deactivated();
}
