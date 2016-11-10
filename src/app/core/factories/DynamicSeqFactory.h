/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DYNAMICSEQFACTORY_H
#define DYNAMICSEQFACTORY_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include "../Entities/AbstractSeq.h"
#include "../global.h"
#include "../enums.h"

class AbstractSeqFactory;

/**
  * Creates the appropriate Seq object based upon the biostring's grammar.
  */
class DynamicSeqFactory : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    DynamicSeqFactory(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    AbstractSeqSPtr makeSeq(const QString &name, const QString &source, const BioString &bioString) const;
    void setSeqFactoryForGrammar(const Grammar grammar, AbstractSeqFactory *seqFactory);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QHash<Grammar, AbstractSeqFactory *> grammarSeqFactoryHash_;
};

#endif // DYNAMICSEQFACTORY_H
