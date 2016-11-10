/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEQUENCEPARSERESULTPOD_H
#define SEQUENCEPARSERESULTPOD_H

#include <QtCore/QVector>

#include "SimpleSeqPod.h"

struct SequenceParseResultPod
{
    Grammar grammar_;
    TriBool isAlignment_;
    QVector<SimpleSeqPod> simpleSeqPods_;

    SequenceParseResultPod() : grammar_(eUnknownGrammar), isAlignment_(eUnknown)
    {
    }

    SequenceParseResultPod(QVector<SimpleSeqPod> simpleSeqPods)
        : grammar_(eUnknownGrammar), isAlignment_(eUnknown), simpleSeqPods_(simpleSeqPods)
    {
    }
};

Q_DECLARE_TYPEINFO(SequenceParseResultPod, Q_MOVABLE_TYPE);

#endif // SEQUENCEPARSERESULTPOD_H
