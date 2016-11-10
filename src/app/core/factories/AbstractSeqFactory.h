/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTSEQFACTORY_H
#define ABSTRACTSEQFACTORY_H

#include <QtCore/QObject>
#include "../Entities/AbstractSeq.h"
#include "../global.h"

class AbstractSeqFactory : public QObject
{
    Q_OBJECT

public:
    AbstractSeqFactory(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    virtual ~AbstractSeqFactory()
    {
    }

    virtual AbstractSeqSPtr makeSeq(const QString &name, const QString &source, const BioString &bioString) const = 0;
};


#endif // ABSTRACTSEQFACTORY_H
