/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CURRENTROWCHANGEDEVENT_H
#define CURRENTROWCHANGEDEVENT_H

#include <QtCore/QEvent>
#include <QtCore/QPersistentModelIndex>

#include "CustomEventTypes.h"

struct CurrentRowChangedEvent : public QEvent
{
    CurrentRowChangedEvent(const QModelIndex &current, const QModelIndex &previous)
        : QEvent(QEvent::Type(constants::kCurrentRowChangedType)), current_(current), previous_(previous)
    {
    }

    QPersistentModelIndex current_;
    QPersistentModelIndex previous_;
};

#endif // CURRENTROWCHANGEDEVENT_H
