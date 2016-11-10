/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef METATYPES_H
#define METATYPES_H

#include <QtCore/QVector>
#include <QtCore/QMetaType>

#include "enums.h"
#include "types.h"

#include "util/ClosedIntRange.h"
#include "util/PosiRect.h"
#include "PODs/Q3Prediction.h"
#include "PODs/BlastDatabaseMetaPod.h"
#include "PODs/SubseqChangePod.h"
#include "PODs/SequenceParseResultPod.h"

Q_DECLARE_METATYPE(PosiRect)
Q_DECLARE_METATYPE(Q3Prediction)
Q_DECLARE_METATYPE(QVector<SubseqChangePod>)
Q_DECLARE_METATYPE(SequenceParseResultPod)
Q_DECLARE_METATYPE(BlastDatabaseMetaPodVector)

#endif // METATYPES_H
