/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AbstractMsa.h"
#include "../ObservableMsa.h"
#include "../global.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
AbstractMsa::~AbstractMsa()
{
    delete msa_;

    clearDeadSubseqs();
}

/**
  * @param subseqs [const QVector<Subseq *>]
  */
void AbstractMsa::addDeadSubseqs(const QVector<Subseq *> subseqs)
{
#ifdef QT_DEBUG
    for (int i=0; i<subseqs.size(); ++i)
        ASSERT(subseqs.at(i)->grammar() == grammar());
#endif

    deadSubseqs_ << subseqs;
}

/**
  */
void AbstractMsa::clearDeadSubseqs()
{
    qDeleteAll(deadSubseqs_);
    deadSubseqs_.clear();
}

/**
  * @returns QVector<Subseq *>
  */
QVector<Subseq *> AbstractMsa::deadSubseqs() const
{
    return deadSubseqs_;
}


/**
  * @returns ObservableMsa *
  */
ObservableMsa *AbstractMsa::msa() const
{
    return msa_;
}

/**
  * @param msa [ObservableMsa *]
  */
void AbstractMsa::setMsa(ObservableMsa *msa)
{
    // Free any msa_
    delete msa_;

    msa_ = msa;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected constructor
/**
  * @param id [int]
  * @param name [const QString &]
  * @param description [const QString &]
  * @param notes [const QString &]
  */
AbstractMsa::AbstractMsa(int id,
                         const QString &name,
                         const QString &description,
                         const QString &notes)
    : AbstractBasicEntity(id, name, description, notes), msa_(nullptr)
{
}
