/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MsaRect.h"

/**
  */
static int registerMetatypes()
{
    qRegisterMetaType<MsaRect>("MsaRect");
//    qRegisterMetaTypeStreamOperators<MsaRect>("MsaRect");

    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetatypes);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const MsaRect &rectangle)
{
    debug.nospace() << "MsaRect([" << rectangle.x() << ',' << rectangle.y() << "] --> ["
                    << rectangle.right() << ',' << rectangle.bottom() << "] :: "
                    << rectangle.width() << 'x' << rectangle.height() << ')';
    return debug.maybeSpace();
}
#endif
