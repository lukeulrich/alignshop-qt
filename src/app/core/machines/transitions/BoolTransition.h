/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BOOLTRANSITION_H
#define BOOLTRANSITION_H

#include <QtCore/QEvent>
#include <QtCore/QAbstractTransition>
#include "../events/BoolEvent.h"

class BoolTransition : public QAbstractTransition
{
public:
    BoolTransition(const bool &value)
        : value_(value)
    {
    }

protected:
    virtual bool eventTest(QEvent *event)
    {
        if (event->type() != QEvent::Type(constants::kBoolEventType))
            return false;

        BoolEvent *boolEvent = static_cast<BoolEvent *>(event);
        return value_ == boolEvent->value_;
    }

    virtual void onTransition(QEvent * /* event */)
    {
    }

private:
    bool value_;
};

#endif // BOOLTRANSITION_H
