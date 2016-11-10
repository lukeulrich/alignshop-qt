/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MSASUBSEQMODEL_H
#define MSASUBSEQMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDate>
#include <QtCore/QDateTime>

#include "Msa.h"
#include "TableModel.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class Msa;
class RelatedTableModel;

/**
  * MsaSubseqModel manages the annotation data for the set of Subseqs contained in an Msa.
  *
  * At a minimum, MsaSubseqModel requires a valid Msa object before any data will be available. To display
  * associated annotation data, the subseqTable_ and/or seqTable_ TableModel's must be defined with the
  * setAnnotationTables method; however, these are not necessary for the model class to function.
  *
  * The data columns to be displayed are as follows:
  * o Valid Msa object -> subseq id, start, stop
  * o Valid SubseqTable object  -> msa columns, all subseq columns
  * o Valid SeqTable object -> msa columns, all subseq columns, all seq columns (note, this will only be accepted if subseqTable is defined)
  *
  * TODO:
  * >> React to the following Msa signals: removal/insertion of subseqs
  * >> setData
  * >> sort
  */
class MsaSubseqModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    explicit MsaSubseqModel(QObject *parent = 0);
    ~MsaSubseqModel();

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;           //!< Returns the number of columns in subseqTable and seqTable
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;  //!< Returns the data for the given model index and role
    Qt::ItemFlags flags(const QModelIndex &index) const;                        //!< Returns the item flags for the given index
    //! Returns the header data for section and orientation and role
    QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;              //!< Returns the number of subseqs contained in the source Msa
    //! Sets the annotation data for the given subseq identified by index to value for the given role; returns true on success or false otherwise
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);            //!< Sorts the Msa subseqs based on the data contained in column in order direction

    // Drag and drop methods
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isGoodIndex(const QModelIndex &index) const;                           //!< Returns true if index belongs to this model and is within the valid row/column bounds
    bool isReady() const;                                                       //!< Returns true if msa_ has been defined
    const Msa *msa() const;                                                     //!< Returns the source Msa
    void setMsa(Msa *msa);                                                      //!< Sets the source msa to msa, connecting any signals needed to keep the annotation model in sync
    //! Sets the data tables for CRUD access to subseqTable and seqTable
    void setAnnotationTables(RelatedTableModel *subseqTable, TableModel *seqTable = 0);
    const TableModel *seqTable() const;                                         //!< Returns the associated TableModel for displaying Seq data
    const RelatedTableModel *subseqTable() const;                               //!< Returns the RelatedTableModel for displaying Subseq data

    static const QStringList coreFields_;
    static const QStringList friendlyCoreFieldNames_;
    static const int kCoreSubseqIdColumn_;
    static const int kCoreSubseqStartColumn_;
    static const int kCoreSubseqStopColumn_;

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onMsaRegionSlid(int left, int top, int right, int bottom);     //!< Unused!!
    void onMsaSubseqsAboutToBeInserted(int start, int end);
    void onMsaSubseqsAboutToBeMoved(int start, int end, int target);
    void onMsaSubseqsAboutToBeRemoved(int start, int end);
    void onMsaSubseqAboutToBeSwapped(int first, int second);
    void onMsaSubseqsInserted(int start, int end);
    void onMsaSubseqsMoved(int start, int end, int target);
    void onMsaSubseqsRemoved(int start, int end);
    void onMsaSubseqSwapped(int first, int second);

    void seqDataChanged(int id, int column);                //!< The value in column for seq in seqTable_ identified by id has changed
    void seqFriendlyFieldNamesChanged(int first, int last); //!< The friendly names between first and last of seqTable have changed; emit headerDataChanged for this model with appropriately transformed index ranges
    void seqTableReset();                                   //!< Called when the associated seqTable_ has been reset
    // seqsOnlyLoad{Done,Error} slots are utilized when the source seq table has been reset and the seq data for those
    //   seqs associated with the msa->subseqs_ need to be reloaded.
    void seqsOnlyLoadDone(int tag);                         //!< Called when the seqs associated with tag have been successfully loaded
    void seqsOnlyLoadError(const QString &error, int tag);  //!< Called when the seqs associated with tag were unable to be loaded
    void subseqDataChanged(int id, int column);             //!< The value in column for subseq in subseqTable_ identified by id has changed
    //! The friendly names between first and last of subseqTable have changed; emit headerDataChanged for this model with appropriately transformed index ranges
    void subseqFriendlyFieldNamesChanged(int first, int last);
    void subseqTableReset();                                //!< Called when the associated subseqTable_ has been reset
    // Similarly, subseqsLoad{Done,Error} pertains to when the subseqTable_ has been reset/cleared and this annotation data needs reloaded.
    void subseqsLoadDone(int tag);                          //!< Called when the subseqs (and possibly related seqs) associated with tag have been successfully loaded
    void subseqsLoadError(const QString &error, int tag);   //!< Called when the subseqs (and/or possibly related seqs) associated with tag were unable to be loaded

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    Msa *msa_;                          //!< Pointer to source Msa, not owned by this class
    RelatedTableModel *subseqTable_;    //!< Pointer to table containing Subseq level annotation data; not owned by this class
    TableModel *seqTable_;              //!< Pointer to table containing Seq level annotation data; not owned by this class

    int subseqLoadTag_;                 //!< Tag of request denoting when the requested subseq data has been loaded
    int seqLoadTag_;                    //!< Tag of request denoting when the requested seq data has been loaded


    /**
      * SubseqLessThanPrivate provides the comparison routine for sorting a Msa's members in ascending order
      * via their associated TableModel data.
      */
    class SubseqLessThanPrivate : public SubseqLessThan
    {
    public:
        //! Construct SubseqLessThan comparison object with self, tableModel, and column
        SubseqLessThanPrivate(const MsaSubseqModel *self, const TableModel *tableModel, int column);
        bool operator()(const Subseq *a, const Subseq *b) const;                //!< Returns true if the data in tableModel_ for a is less than the corresponding data value in b; false otherwise

    private:
        const MsaSubseqModel *self_;
        const TableModel *tableModel_;
        int column_;
    };

    /**
      * SubseqGreaterThanPrivate provides the comparison routine for sorting a Msa's members in descending order
      * via their associated TableModel data.
      */
    class SubseqGreaterThanPrivate : public SubseqGreaterThan
    {
    public:
        //! Construct SubseqGreaterThan comparison object with self, tableModel, and column
        SubseqGreaterThanPrivate(const MsaSubseqModel *self, const TableModel *tableModel, int column);
        bool operator()(const Subseq *a, const Subseq *b) const;                //!< Returns true if the data in tableModel_ for a is greater than the corresponding data value in b; false otherwise

    private:
        const MsaSubseqModel *self_;
        const TableModel *tableModel_;
        int column_;
    };

#ifdef TESTING
    friend class TestMsaSubseqModel
#endif
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// SubseqLessThanPrivate implementation
/**
  * @param self [const MsaSubseqModel *]
  * @param tableModel [const TableModel *]
  * @param column [int]
  */
inline
MsaSubseqModel::SubseqLessThanPrivate::SubseqLessThanPrivate(const MsaSubseqModel *self, const TableModel *tableModel, int column)
    : SubseqLessThan(), self_(self), tableModel_(tableModel), column_(column)
{
}

/**
  * Looks at the data type returned by the TableModel and uses that to perform the "less than" comparison.
  *
  * OPTIMIZATION: Since TableModel contains database records in which each field has a defined type,
  *               it is feasible to store those alongside the fieldnames. When this class is constructed,
  *               a variable with the exact data type could then be utilized here and avoid the need to
  *               query the actual data type each time this method is called.
  *
  * @param a [const Subseq *]
  * @param b [const Subseq *]
  * @returns bool
  */
inline
bool MsaSubseqModel::SubseqLessThanPrivate::operator()(const Subseq *a, const Subseq *b) const
{
    ASSERT(a);
    ASSERT(b);
    ASSERT(tableModel_);
    QVariant aValue = tableModel_->data(a->id(), column_);
    QVariant bValue = tableModel_->data(b->id(), column_);

    switch (aValue.userType())
    {
    case QVariant::Invalid:
        return bValue.type() != QVariant::Invalid;

    case QVariant::Bool:
        return aValue.toBool() < bValue.toBool();
    case QVariant::Char:
        return aValue.toChar() < bValue.toChar();
    case QVariant::Date:
        return aValue.toDate() < bValue.toDate();
    case QVariant::DateTime:
        return aValue.toDateTime() < bValue.toDateTime();
    case QVariant::Double:
        return aValue.toDouble() < bValue.toDouble();
    case QVariant::Int:
        return aValue.toInt() < bValue.toInt();
    case QVariant::LongLong:
        return aValue.toLongLong() < bValue.toLongLong();
    case QVariant::Time:
        return aValue.toTime() < bValue.toTime();
    case QVariant::UInt:
        return aValue.toUInt() < bValue.toUInt();
    case QVariant::ULongLong:
        return aValue.toULongLong() < bValue.toULongLong();
    case QVariant::String:
    default:
        return aValue.toString().localeAwareCompare(bValue.toString()) < 0;
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// SubseqGreaterThanPrivate implementation
/**
  * @param self [const MsaSubseqModel *]
  * @param tableModel [const TableModel *]
  * @param column [int]
  */
inline
MsaSubseqModel::SubseqGreaterThanPrivate::SubseqGreaterThanPrivate(const MsaSubseqModel *self, const TableModel *tableModel, int column)
    : SubseqGreaterThan(), self_(self), tableModel_(tableModel), column_(column)
{
}

/**
  * Looks at the data type returned by the TableModel and uses that to perform the "greater than" comparison.
  *
  * OPTIMIZATION: Since TableModel contains database records in which each field has a defined type,
  *               it is feasible to store those alongside the fieldnames. When this class is constructed,
  *               a variable with the exact data type could then be utilized here and avoid the need to
  *               query the actual data type each time this method is called.
  *
  * @param a [const Subseq *]
  * @param b [const Subseq *]
  * @returns bool
  */
inline
bool MsaSubseqModel::SubseqGreaterThanPrivate::operator()(const Subseq *a, const Subseq *b) const
{
    ASSERT(a);
    ASSERT(b);
    ASSERT(tableModel_);
    QVariant aValue = tableModel_->data(a->id(), column_);
    QVariant bValue = tableModel_->data(b->id(), column_);

    switch (aValue.userType())
    {
    case QVariant::Invalid:
        return bValue.type() == QVariant::Invalid;

    case QVariant::Bool:
        return aValue.toBool() > bValue.toBool();
    case QVariant::Char:
        return aValue.toChar() > bValue.toChar();
    case QVariant::Date:
        return aValue.toDate() > bValue.toDate();
    case QVariant::DateTime:
        return aValue.toDateTime() > bValue.toDateTime();
    case QVariant::Double:
        return aValue.toDouble() > bValue.toDouble();
    case QVariant::Int:
        return aValue.toInt() > bValue.toInt();
    case QVariant::LongLong:
        return aValue.toLongLong() > bValue.toLongLong();
    case QVariant::Time:
        return aValue.toTime() > bValue.toTime();
    case QVariant::UInt:
        return aValue.toUInt() > bValue.toUInt();
    case QVariant::ULongLong:
        return aValue.toULongLong() > bValue.toULongLong();
    case QVariant::String:
    default:
        return aValue.toString().localeAwareCompare(bValue.toString()) > 0;
    }
}


#endif // MSASUBSEQMODEL_H
