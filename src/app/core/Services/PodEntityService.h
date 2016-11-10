/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PODENTITYSERVICE_H
#define PODENTITYSERVICE_H

#include <QtCore/QVector>
#include "../Entities/IEntity.h"
#include "../enums.h"

class Adoc;
// class ObservableMsa;
struct SimpleSeqPod;

/**
  * Converts POD data (e.g. SimpleSeq data from an import operation) to entities
  */
class PodEntityService
{
public:
    PodEntityService(Adoc *adoc);

    QVector<IEntitySPtr> convertToEntities(const QVector<SimpleSeqPod> &simpleSeqPods, bool isAlignment, Grammar grammar);
//    ObservableMsa *convertToObservableMsa(const QVector<SimpleSeqPod> &simpleSeqPods, Grammar grammar);

private:
    QVector<IEntitySPtr> convertToAminoSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const;
    QVector<IEntitySPtr> convertToDnaSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const;
    QVector<IEntitySPtr> convertToRnaSeqEntities(const QVector<SimpleSeqPod> &simpleSeqPods) const;

    IEntitySPtr convertToAminoMsaEntity(const QVector<SimpleSeqPod> &simpleSeqPods) const;
    IEntitySPtr convertToDnaMsaEntity(const QVector<SimpleSeqPod> &simpleSeqPods) const;

    Adoc *adoc_;
};

#endif // PODENTITYSERVICE_H
