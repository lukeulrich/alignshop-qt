/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ClosedIntRange.h"

static int qRegisterTypes()
{
    qRegisterMetaType<ClosedIntRange>("ClosedIntRange");
    return 0;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterTypes)


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const ClosedIntRange &range)
{
    debug.nospace() << "ClosedIntRange(";
    debug.nospace() << range.begin_ << ".." << range.end_ << " = " << range.length() << ")";
    return debug.space();
}
#endif
