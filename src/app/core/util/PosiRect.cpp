/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PosiRect.h"

/**
  */
static int registerMetatypes()
{
    qRegisterMetaType<PosiRect>("PosiRect");

    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetatypes);

#ifndef QT_NO_DEBUG_STREAM
#include <QtDebug>
QDebug operator<<(QDebug debug, const PosiRect &rectangle)
{
    debug.nospace() << "PosiRect([" << rectangle.x() << ',' << rectangle.y() << "] --> ["
                    << rectangle.right() << ',' << rectangle.bottom() << "] :: "
                    << rectangle.width() << 'x' << rectangle.height() << ')';
    return debug.maybeSpace();
}
#endif
