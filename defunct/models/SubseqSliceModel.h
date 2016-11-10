/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SUBSEQSLICEMODEL_H
#define SUBSEQSLICEMODEL_H

#include "SliceProxyModel.h"

#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AdocTreeNode;
class TableModel;
class RelatedTableModel;

/**
  * SubseqSliceModel abstracts the annotation bookkeeping duties of a Subseq slice and delegates all
  * data loading to a related subseq table and a seq table.
  *
  * Specifically, it dynamically augments the tree model to view a specific slice of data corresponding
  * to group nodes and other nodes with a similar alphabet/type.
  *
  * Automatic filtering is carried out as follows (filterAcceptsNode):
  * o All groups are immediately accepted
  * o Subseq nodes are accepted based upon the alphabet_ member
  *   - eUnknownAlphabet: no subseq nodes are selected
  *   - eAminoAlphabet: true for SubseqAminoType
  *   - eDnaAlphabet: true for SubseqDnaType
  *   - eRnaAlphabet: true for SubseqRnaType
  * o Similar logic applies for Msa nodes
  * o All other nodes are rejected
  *
  * The annotation data is externally stored in the subseqTable_ and seqTable_ models and retained in
  * the SliceProxyModel::taggedSlice_ member until it has been loaded (or an error occurred while
  * attempting to load this data). This provides a lightweight implementation requiring only the identifiers
  * to be stored here and the annotation data only stored once in the external table model.
  *
  * Once the data has been loaded, either a loadDone or loadError signal is emitted from the subseqTable_,
  * which is connected to the subseqsLoadDone and subseqsLoadError slots, respectively, of this class. If
  * the subseqs have been loaded successfully, the slice is appended to the official data store; otherwise,
  * it is simply ignored.
  *
  * TODO: Error reporting for unsuccessful data load requests.
  */
class SubseqSliceModel : public SliceProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    SubseqSliceModel(QObject *parent = 0);                  //!< Trivial constructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    Alphabet alphabet() const;                              //!< Returns the currently defined alphabet
    //! Returns the number of fields contained in both seqTable and subseqTable or zero if uninitialized; parent parameter is ignored
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    //! Reimplemented public method that simply calls data(index, role, handled) and permits subclasses to avoid having to define a data function
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    //! Returns the data for the given index and role and sets handled to true if processed or false otherwise. Provides for subclasses to utilize this method to perform boilerplate functionality
    QVariant data(const QModelIndex &index, int role, bool &handled) const;
    //!< Returns the data for the given role and section in the header with the specified orientation by requesting field names from the relevant subseq and seq tables
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool isInitialized() const;                             //!< Returns true if alphabet is not eUnknownAlphabet and subseqTable_ and seqTable_ are non-zero
    //! Sets the data identified by index to value for role (only EditRole currently supported) and returns whether this operation was successful
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    //! Sets the subseq and seq annotation data source tables to subseqTable and seqTable, respectively, for the given alphabet; resets the model
    void setSourceTables(Alphabet alphabet, RelatedTableModel *subseqTable, TableModel *seqTable);

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void taggedSliceCreated(int tag);                       //!< Reimplemented virtual signal for when a tagged slice has been created

    // ------------------------------------------------------------------------------------------------
    // Protected members
    RelatedTableModel *subseqTable_;                        //!< The table source for subseq annotation data
    TableModel *seqTable_;                                  //!< The table source for seq annotation data

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void seqDataChanged(int id, int column);                //!< The value in column for seq in seqTable_ identified by id has changed
    void seqFriendlyFieldNamesChanged(int first, int last); //!< The friendly names between first and last of seqTable have changed; emit headerDataChanged for this model with appropriately transformed index ranges
    void subseqDataChanged(int id, int column);             //!< The value in column for subseq in subseqTable_ identified by id has changed
    //! The friendly names between first and last of subseqTable have changed; emit headerDataChanged for this model with appropriately transformed index ranges
    void subseqFriendlyFieldNamesChanged(int first, int last);
    void subseqsLoadDone(int tag);                          //!< Called when the subseqs in the slice associated with tag have been successfully loaded
    void subseqsLoadError(const QString &error, int tag);   //!< Called when the subseqs in the slice associated with tag were unable to be loaded

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool filterAcceptsNode(AdocTreeNode *node) const;       //!< Reimplemented function that returns true if node is a group node or the relevant sequence type based on alphabet; false otherwise
    bool isGroupOrMsa(AdocTreeNode *node) const;            //!< Simply tests node->nodeType_ if it is a group or Msa

    Alphabet alphabet_;
};

#endif // SUBSEQSLICEMODEL_H
