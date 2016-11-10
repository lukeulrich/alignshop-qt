/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQFACTORY_H
#define AMINOSEQFACTORY_H

#include "AbstractSeqFactory.h"
#include "../Entities/Astring.h"
#include "../Repositories/AnonSeqRepository.h"
#include "../global.h"

class AminoSeqFactory : public AbstractSeqFactory
{
    Q_OBJECT

public:
    AminoSeqFactory(AnonSeqRepository<Astring> *astringRepository, QObject *parent = nullptr);

    AbstractSeqSPtr makeSeq(const QString &name, const QString &source, const BioString &aminoString) const;

private:
    AnonSeqRepository<Astring> *astringRepository_;
};

#endif // AMINOSEQFACTORY_H
