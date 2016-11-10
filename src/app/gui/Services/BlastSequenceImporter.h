/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTSEQUENCEIMPORTER_H
#define BLASTSEQUENCEIMPORTER_H

#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include "../../core/Entities/IEntity.h"
#include "../../core/enums.h"
#include "../../core/global.h"

class QWidget;

class Adoc;
class AdocTreeModel;
class AdocTreeNode;
class BioString;
class BlastReportModel;
class BlastSequenceFetcher;

class BlastSequenceImporter : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit BlastSequenceImporter(QObject *parent = nullptr);

    struct BlastImportRequest
    {
        Grammar grammar_;
        QModelIndexList blastIndices_;
        QString blastDatabase_;
        QModelIndex destinationIndex_;
    };

    Adoc *adoc() const;
    void setAdoc(Adoc *adoc);
    void setAdocTreeModel(AdocTreeModel *adocTreeModel);


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void import(const BlastImportRequest &request);


private Q_SLOTS:
    void onBlastImportError(int id, const QString &message);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool addToRepository(const QVector<IEntitySPtr> &entities) const;
    bool containsEmptyBioString(const QVector<BioString> &bioStringVector) const;
    QVector<AdocTreeNode *> createAdocTreeNodes(const QVector<IEntitySPtr> &entities);
    QVector<IEntitySPtr> createAminoSeqEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const;
    QVector<IEntitySPtr> createDnaSeqEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const;
    QVector<IEntitySPtr> createEntities(const QModelIndexList &blastIndices_, const QVector<BioString> &bioStringVector) const;
    void displayWarning(const QString &message) const;
    QVector<BioString> fetchSequences(const QStringList &blastIds, const QString &blastDatabase_, const Grammar &grammar);
    QStringList getBlastIds(const QModelIndexList &blastIndices) const;
    QModelIndex getDestinationIndex(const QModelIndex &initialIndex);
    void initializeFetcher();
    bool isValidRequest(const BlastImportRequest &blastImportRequest) const;
    QWidget *parentWidget() const;


    // ------------------------------------------------------------------------------------------------
    // Private members
    Adoc *adoc_;
    AdocTreeModel *adocTreeModel_;
    BlastSequenceFetcher *blastSequenceFetcher_;
};

#endif // BLASTSEQUENCEIMPORTER_H
