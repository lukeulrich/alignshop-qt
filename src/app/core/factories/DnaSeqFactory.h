/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQFACTORY_H
#define DNASEQFACTORY_H

#include "AbstractSeqFactory.h"
#include "../Entities/Dstring.h"
#include "../Repositories/AnonSeqRepository.h"
#include "../global.h"

class DnaSeqFactory : public AbstractSeqFactory
{
    Q_OBJECT

public:
    DnaSeqFactory(AnonSeqRepository<Dstring> *dstringRepository, QObject *parent = nullptr);

    AbstractSeqSPtr makeSeq(const QString &name, const QString &source, const BioString &dnaString) const;

private:
    AnonSeqRepository<Dstring> *dstringRepository_;
};

#endif // DNASEQFACTORY_H
