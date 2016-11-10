/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BOOLEVENT_H
#define BOOLEVENT_H

#include <QtCore/QEvent>

#include "CustomEventTypes.h"

struct BoolEvent : public QEvent
{
    bool value_;

    BoolEvent(const bool &value)
        : QEvent(QEvent::Type(constants::kBoolEventType)),
          value_(value)
    {
    }
};

#endif // BOOLEVENT_H
