/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QDataStream>

#include "Rect.h"

/**
  */
static int registerMetatypes()
{
    qRegisterMetaType<Rect>("Rect");
    qRegisterMetaTypeStreamOperators<Rect>("Rect");

    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetatypes);

/**
  * Serializes rect to the given stream and returns a reference to the stream
  *
  * @param stream [const QDataStream &]
  * @param rect [const Rect &]
  * @returns QDataStream &
  *
  * UNTESTED
  */
QDataStream &operator<<(QDataStream &stream, const Rect &rect)
{
    stream << (qint32)rect.x1_ << (qint32)rect.y1_ << (qint32)rect.x2_ << (qint32)rect.y2_;

    return stream;
}

/**
  * Unserializes rect from the given stream and returns a reference to the stream
  *
  * @param stream [const QDataStream &]
  * @param rect [const Rect &]
  * @returns QDataStream &
  */
QDataStream &operator>>(QDataStream &stream, Rect &rect)
{
    qint32 x1, y1, x2, y2;
    stream >> x1 >> y1 >> x2 >> y2;
    rect.x1_ = x1;
    rect.y1_ = y1;
    rect.x2_ = x2;
    rect.y2_ = y2;

    return stream;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const Rect &rectangle)
{
    debug.nospace() << "Rect([" << rectangle.x() << ',' << rectangle.y() << "] --> ["
                    << rectangle.right() << ',' << rectangle.bottom() << "] :: "
                    << rectangle.width() << 'x' << rectangle.height() << ')';
    return debug.maybeSpace();
}
#endif
