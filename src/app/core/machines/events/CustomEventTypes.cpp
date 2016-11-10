/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QEvent>

#include "CustomEventTypes.h"

namespace constants
{
    const int kCurrentRowChangedType = QEvent::User + 1;
    const int kBoolEventType = QEvent::User + 2;
}
