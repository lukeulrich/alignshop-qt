/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSASUBSEQTABLEMODEL_H
#define MSASUBSEQTABLEMODEL_H

#include <QtCore/QAbstractTableModel>
#include "../../core/Entities/IEntity.h"
#include "../../core/global.h"
#include "../../core/util/ClosedIntRange.h"
#include "../../core/PODs/SubseqChangePod.h"

class QUndoStack;

class IColumnAdapter;
class ObservableMsa;
class Subseq;

class MsaSubseqTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum SelfColumn
    {
        eStartColumn = 0,
        eStopColumn,

        eNumberOfSelfColumns
    };

    explicit MsaSubseqTableModel(QObject *parent = nullptr);
    explicit MsaSubseqTableModel(ObservableMsa *msa, IColumnAdapter *columnAdapter = nullptr, QObject *parent = nullptr);

    // -------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);


    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indices) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;


    // -------------------------------------------------------------------------------------------------
    // Public methods
    IColumnAdapter *columnAdapter() const;
    int mapFromColumnAdapter(int column) const;                                     //!< Maps column (corresponding to a column adapter column) to the corresponding column in this model; returns -1 if a column adapter is not defined or column is outside of the column adapters range
    ObservableMsa *msa() const;
    void setMsa(ObservableMsa *msa, IColumnAdapter *columnAdapter);
    void setUndoStack(QUndoStack *undoStack);



private Q_SLOTS:
    void onMsaRowsAboutToBeInserted(const ClosedIntRange &rows);                     //!< Emitted just before subseqs are inserted; new subseqs will be placed at rows
    void onMsaRowsAboutToBeMoved(const ClosedIntRange &rows, int finalRow);          //!< Emitted just before subseqs are moved; the subseqs in rows will be placed at finalRow
    void onMsaRowsAboutToBeRemoved(const ClosedIntRange &rows);                      //!< Emitted just before subseqs are removed; subseqs will be removed from rows
    void onMsaRowsInserted(const ClosedIntRange &rows);                              //!< Emitted after subseqs have been inserted in rows
    void onMsaRowsMoved(const ClosedIntRange &rows, int finalRow);                   //!< Emitted after subseqs have been moved; the subseqs previously in rows are now located at finalRow
    void onMsaRowsRemoved(const ClosedIntRange &rows);                               //!< Emitted after subseqs have been removed; subseqs were previously located at rows
    void onMsaSubseqsChanged(const SubseqChangePodVector &subseqChangePods);         //!< Emitted after the subseqs in subseqChangePods have had their borders modified via either an extension, trim, level, collapse, setSubseqStart, or setSubseqStop


    void onSubseqEntityDataChanged(const IEntitySPtr &entity, int column);

private:
    QVariant data(const Subseq *subseq, int column, int role = Qt::DisplayRole) const;

    ObservableMsa *msa_;
    IColumnAdapter *columnAdapter_;
    QUndoStack *undoStack_;

    friend class SubseqLessThanPrivate;         // To access our private data method
};

#endif // MSASUBSEQTABLEMODEL_H
