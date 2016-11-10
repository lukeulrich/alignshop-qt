/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "metatypes.h"

/**
  */
static int registerMetatypes()
{
    qRegisterMetaType<SequenceParseResultPod>("SequenceParseResultPod");

    return 0;
}
Q_CONSTRUCTOR_FUNCTION(registerMetatypes);

