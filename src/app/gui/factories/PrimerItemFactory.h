/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PRIMERITEMFACTORY_H
#define PRIMERITEMFACTORY_H

#include <QtCore/QObject>
#include "../../core/Entities/DnaSeq.h"
#include "../../core/global.h"

class AbstractPrimerItem;
class Primer;

class PrimerItemFactory : public QObject
{
    Q_OBJECT

public:
    PrimerItemFactory(QObject *parent = nullptr);

    void setDnaSeq(const DnaSeqSPtr &dnaSeq);
    QVector<AbstractPrimerItem *> makePrimerItemsFromPrimer(const Primer &primer) const;

private:
    QVector<AbstractPrimerItem *> makeForwardPrimerItemsFromPrimer(const Primer &primer) const;
    QVector<AbstractPrimerItem *> makeReversePrimerItemsFromPrimer(const Primer &primer) const;

    DnaSeqSPtr dnaSeq_;
};

#endif // PRIMERITEMFACTORY_H
