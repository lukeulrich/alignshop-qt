/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNASEQFEATUREITEMSYNCHRONIZER_H
#define DNASEQFEATUREITEMSYNCHRONIZER_H

#include <QtCore/QObject>

#include "../../core/Entities/DnaSeq.h"

class QModelIndex;

class DnaSeqItem;
class DnaSeqPrimerModel;

/**
  * Synchronizes modifications to a DnaSeq with its state in a corresponding QGraphicsScene.
  *
  * For instance, when primers are added or removed, adds or removes the corresponding feature item, respectively.
  */
class DnaSeqFeatureItemSynchronizer : public QObject
{
    Q_OBJECT

public:
    DnaSeqFeatureItemSynchronizer(QObject *parent = nullptr);

    void setDnaSeqItem(DnaSeqItem *dnaSeqItem);
    void setDnaSeqPrimerModel(DnaSeqPrimerModel *dnaSeqPrimerModel);


private Q_SLOTS:
    void onDnaSeqItemAboutToBeDestroyed();
    void onPrimerDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onPrimerRowsAboutToBeRemoved(const QModelIndex &parent, int startRow, int endRow);
    void onPrimerRowsInserted(const QModelIndex &parent, int startRow, int endRow);
    void initializePrimerItemsFromModel();


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void generatePrimerItems(int startRow, int endRow);


    DnaSeqItem *dnaSeqItem_;
    DnaSeqPrimerModel *dnaSeqPrimerModel_;
};

#endif // DNASEQFEATUREITEMSYNCHRONIZER_H
