/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SLICEPROXYMODEL_H
#define SLICEPROXYMODEL_H

#include <QtCore/QAbstractItemModel>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

class AdocTreeModel;
class AdocTreeNode;

/**
  * SliceProxyModel is an abstract base class for managing a "slice" of data associated with specific tree
  * nodes of a user-supplied AdocTreeModel.
  *
  * Motivation: While it possible to simply utilize the AdocTreeModel for all data storage and manipulation,
  * this approach is sub-optimal for several reasons:
  * - AdocTreeModel's primary function is organizing the individual items of a heterogeneous tree structure
  *   and only the item's simple data
  * - AdocTreeModel must always respond to data requests cell-by-cell. This complicates strategic
  *   caching and loading of entire data. In the case of SQLite data access, this is dramatically slower
  *   than requesting the entire array of data in one query.
  * - It centralizes all access and display code in a single class
  *
  * The primary motivation: speed and additional clarity when dealing with heterogeneous data. Tree structures
  * populated with heterogeneous nodes that may appear at virtually any location is very difficult to
  * model and display in a user-friendly manner. This mandates an approach that provides "windows" into
  * similar / equivalent entities. Originally, this was accomplished by chaining custom QSortFilterProxyModel
  * instances and ultimately mapping all data requests back to the AdocTreeModel. For improved speed,
  * a row-by-row cache manager was implemented; however, this is still less-desirable than loading all
  * relevant data from the data source in as few calls as possible.
  *
  * The first generation SliceModel created all data (including duplicated) values into memory and stored this
  * per source node. In October 2010, the overall data storage model was revamped to utilize the TableModel and
  * RelatedTableModel classes, which more closely parallel the actual database structure. In concert with this,
  * SliceProxyModel no longer stores this data, but refers to associated Table and RelatedTableModels, which
  * manage this data. Thus, each instance of a SliceProxyModel now only contains a single window of data, which
  * is set via the setSourceParent() method.
  *
  * The actual data structure is a flat table and each nodes parent is an invalid QModelIndex, although the
  * actual sub root parent, sourceParent_, may or may not refer to the tree model root.
  *
  * The internal slice data is split between two data structures:
  * 1) Slice slice_
  * 2) QHash<int, Slice> taggedSlices_
  *
  * Only data stored in slice_ is actually available to downstream components. The information in taggedSlices_
  * is not.
  *
  * Whenever new data is inserted to the tree model that returns true for the filtersAcceptsNode() method, it
  * is first added to taggedSlices_ with a unique tag generated via TagGenerator. This issues the virtual signal
  * taggedSliceCreated(), which by default (defined in this class) simply moves the newly created slice into
  * slice_. With synchronous models, the default implementation is usually sufficient; however, other situations
  * such as those requiring asynchronous data loading, it may not be desirable to immediately make all these
  * nodes available to connected components (e.g. views). Thus, derived classes may override the taggedSliceCreated()
  * method to manage this process of moving matching nodes in taggedSlices to the slice_ structure.
  *
  * An example of the above situation occurs when showing data from a database table that is loaded asynchronously.
  * The user may receive several nodes from a tree model for display; however, this data has not yet been loaded
  * from the data source. Thus, a subclass could override taggedSliceCreated(tag), issue a database request, and
  * once the data has been loaded, then move the taggedSlice nodes to slice_.
  *
  * By having this split functionality in this class, it avoid subclasses having to finagle with bookkeeping of
  * the source nodes and their mappings to the tree model. SliceProxyModel automatically keeps track and updates
  * slice_ and all taggedSlices_ with respect to changes in the source tree model.
  */
class SliceProxyModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit SliceProxyModel(QObject *parent = 0);                              //!< Basic constructor; sets pointers to zero
    virtual ~SliceProxyModel();                                                 //!< Standard destructor; releases any dynamically allocated memory

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //!< Returns the number of columns beneath parent
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual QVariant data(const QModelIndex &index, int role) const = 0;        //! Returns the data associated with index for the given role
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;                //!< Returns the item flags for the given index
    //! Default implementation specificed here simply returns the value returned by the source tree model's header data function or an invalid QVariant if the source tree model is null
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    //! Returns the index for the model item specified by row, column, and parent
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;                 //!< Returns the parent of the model item with the given index; returns an invalid QModelIndex if index does not have a parent
    //!< Remove count rows starting with row from the given parent (which must be a top level item)
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;      //!< Returns the number of rows under the given parent

    // ---------------------
    // Drag and drop methods
    //! Returns false if column is not the primaryColumn(); otherwise returns the source tree model result
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;          //!< Returns a QMimeData object containing a mapped set of indexes associated with the source tree model
    virtual QStringList mimeTypes() const;                                      //!< Returns the value of the source Calls the source tree model mimeTypes function
    virtual Qt::DropActions supportedDragActions() const;                       //!< Calls the source tree model's supportedDragActions method
    virtual Qt::DropActions supportedDropActions() const;                       //!< Calls the source tree model's supportedDropActions method

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void clear();                                                               //!< Clears the internal data structures and releases any dynamically allocated slices and issues a model reset
    int defaultSortColumn() const;                                              //!< Returns the integer position of the column to utilize for sorting purposes; default is 0
    virtual bool isEditableColumn(int column) const;                            //!< Virtual method that returns true if column is editable or false otherwise; default is false
    bool isGoodIndex(const QModelIndex &index) const;                           //!< Returns true if index belongs to this model, is valid, and is within the rowCount(), columnCount() limits
    bool isGroupItem(const QModelIndex &index) const;                           //!< Returns true if index belongs to this model as a slice item and represents an AdocTreeNode::GroupType or false otherwise
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;       //!< Returns the source tree model index that corresponds to the proxyIndex in this model
    int primaryColumn() const;                                                  //!< Returns the primary column for this model; default is 0; useful for drag and drop
    void setDefaultSortColumn(int column);                                      //!< Sets the default sort column to column
    void setPrimaryColumn(int column);                                          //!< Sets the primary column to column; user for drag and drop
    void setSourceParent(const QModelIndex &sourceParentIndex);                 //!< Sets the source parent for this slice to the node corresponding to sourceParentIndex
    void setTreeModel(AdocTreeModel *adocTreeModel);                            //!< Sets the underlying AdocTreeModel to adocTreeModel; does not take ownership
    const AdocTreeNode *sourceParentNode() const;                               //!< Returns the currently set parent node or zero if there is none
    AdocTreeModel *sourceTreeModel() const;                                     //!< Returns the currently associated AdocTreeModel
    //! Virtual stub for customizing the horizontal header
    virtual void tweakHorizontalHeader(QHeaderView *header) const               { Q_UNUSED(header); }

public slots:
    void refreshSourceParent();                                                 //!< Clears the model data and calls setSourceParent to reload the nodes beneath sourceParent_

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected data structures
    struct Slice {
        QList<AdocTreeNode *> sourceNodes_;         //!< List of AdocTreeNode's contained in this slice
        QList<int> sourceRowIndices_;               //!< Row number from source model of the corresponding AdocTreeNode pointer

        Slice& operator<<(const Slice &other);      //!< Appends the contents of other to the current instance if other is not the same slice
        int count() const;                          //!< Returns the number of nodes currently in this slice
        bool isEmpty() const;                       //!< Returns true if the slice is empty or false otherwise
    };

    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual bool filterAcceptsNode(AdocTreeNode *node) const;                   //!< Returns true if node should be included in the model or false otherwise; the default implementation always returns true
    void appendSlice(const Slice &slice);                                       //!< Appends slice to the current slice_ emitting the proper signals

    // ------------------------------------------------------------------------------------------------
    // Internal virtual "signals" called whenever a specific slice change event occurs
    // The base class implementation of all except taggedSliceCreated does nothing. These are not intended to be
    // called outside of this class interface or undefined behavior may result.
    // sliceCleared(), sliceRowsAboutToBeRemoved(), sliceRowsInserted(), and sliceRowsRemoved() are called solely
    // for any changes made to slice_, not taggedSlices_.
    //! Called when the slice_ field is cleared
    virtual void sliceCleared()                                   {}
    //! Called when the rows between start and end (inclusive) are about to be removed
    virtual void sliceRowsAboutToBeRemoved(int start, int end)    { Q_UNUSED(start); Q_UNUSED(end); }
    //! Called when the rows between start and end (inclusive) are about to be inserted
    virtual void sliceRowsInserted(int start, int end)            { Q_UNUSED(start); Q_UNUSED(end); }
    //! Called when the rows between start and end (inclusive) have been removed
    virtual void sliceRowsRemoved(int start, int end)             { Q_UNUSED(start); Q_UNUSED(end); }
    //! Called when a non-empty tagged slice has been created and inserted into taggedSlices_ with tag
    virtual void taggedSliceCreated(int tag);

    Slice slice_;                                   //!< Slice currently being modeled
    QHash<int, Slice> taggedSlices_;                //!< Tagged slices that have been created but not yet added to slice_

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots - must respond to these events to keep the model in sync with changes to the AdocTreeModel
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void sourceModelReset();
    void sourceRowsInserted(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void clearStructures();                                                      //!< Internal function that frees all slice data and clears the internal data structures
    virtual Slice createSlice(AdocTreeNode *parent) const;                       //!< Factory method for creating and returning the relevant slice from among all children of parent
    virtual Slice createSlice(AdocTreeNode *parent, int start, int end) const;   //!< Factory method responsible for creating and returning a Slice for parent from start to end inclusive

    AdocTreeModel *adocTreeModel_;                  //!< Underlying AdocTreeModel
    AdocTreeNode *sourceParent_;                    //!< Source parent node for this model
    int defaultSortColumn_;
    int primaryColumn_;

    // Special storage variables solely for handling the removal of items via the sourceRowsAboutToBeRemoved() and sourceRowsRemoved() slots
    QList<QPair<int, int> > sliceRowsToRemove_;     //!< Array of slice item ranges (first = start, second = end) to be removed
    QHash<int, QList<QPair<int, int> > > taggedSliceRowsToRemove_;  //!< Tagged array of slice item ranges (first = start, second = end) to be removed
    bool removeMappedParent_;                       //!< If the parent node has been selected for removal

#ifdef TESTING
    // Provide access to the internal state of this class if we are testing
    friend class TestSliceProxyModel;
#endif
};



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Inline slice functions
/**
  * @param other [const Slice &]
  * @returns Slice &
  */
inline
SliceProxyModel::Slice& SliceProxyModel::Slice::operator<<(const Slice &other)
{
    if (this != &other)
    {
        sourceNodes_ << other.sourceNodes_;
        sourceRowIndices_ << other.sourceRowIndices_;
    }

    return *this;
}

/**
  * @returns int
  */
inline
int SliceProxyModel::Slice::count() const
{
    return sourceNodes_.count();
}

/**
  * @returns bool
  */
inline
bool SliceProxyModel::Slice::isEmpty() const
{
    return sourceNodes_.isEmpty();
}


#endif // SLICEPROXYMODEL_H
