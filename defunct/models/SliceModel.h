/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SLICEMODEL_H
#define SLICEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPair>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

class AdocTreeModel;
class AdocTreeNode;
class DataRow;

/**
  * Slice represents a two-dimensional array of QVariant data intended for fast data access to tabular
  * sequence data displayed in an ItemView.
  *
  * It contains three list variables which should always contain the same number of elements.
  */
struct Slice
{
    QList<DataRow> rows_;               //!< Actual data rows comprising the slice
    QList<AdocTreeNode *> sourceNodes_; //!< List of AdocTreeNode's contained in this slice
    QList<int> sourceRowIndices_;       //!< Row number from source model of the corresponding AdocTreeNode pointer
    int refCount_;                      //!< External reference count

    // ------------------------------------------------------------------------------------------------
    // Constructor
    Slice();                            //!< Trivial constructor
};



/**
  * SliceModel is an abstract base class for managing "slices" of data associated with specific tree
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
  * SliceModel addressses these issues by allowing a subclass to decide which rows
  *
  * SliceModel addresses these issues by lazily creating "slices" for a particular AdocTreeNode parent.
  * This 2D array then becomes the basis for all data requests that map to this parent, which dramatically
  * improves speed by providing direct access. Because this could potentially utilize large amounts of
  * memory, the user is required to release and load slices as desired. A reference count of each slice
  * is maintained in accordance with these commands and slices are automatically freed when their
  * reference count reaches zero.
  *
  * The underlying SliceModel structure is a simple, two-layered tree structure:
  *
  * o Root
  * |___ AdocTreeNode *A --> Slice *A
  *      |___ Slice item A0.0, Slice item A0.1, ... Slice item A0.n
  *      |___ Slice item A1.0, Slice item A1.1, ... Slice item A1.n
  *      |___ ...
  *      |___ Slice item Am.0, Slice item Am.1, ... Slice item Am.n
  * |___ AdocTreeNode *B --> Slice *B
  *      |___ Slice item 0
  *      |___ Slice item 1
  *      |___ ...
  *      |___ Slice item n
  * ...
  * |___ AdocTreeNode *m --> Slice *n
  *
  *
  * Just beneath the root are AdocTreeNode pointers corresponding to a particular AdocTreeNode parent and
  * that map to a Slice instance. At the second-most layer are slice rows and their columns.
  *
  * To utilize a SliceModel, it is first necessary to call loadSlice on an appropriate AdocTreeNode
  * parent (e.g. selected from an ItemView). This will dynamically load the relevant data and prepare
  * the model for further data access. To attach to this data via an ItemView, it is necessary to set
  * the root index to one of the top-level indices. It is not intended for any attached ItemViews to
  * set the root to the tree root or slice items, but rather only the AdoctreeNode first level items.
  * If it does set the root to an invalid location, 0 rows and columns will be returned, which will
  * result in an empty view.
  *
  * This class will only function if it has been sourced to an appropriate AdocTreeModel. Upon attaching
  * to an AdocTreeModel it will disconnect/connect the appropriate signals and slots to keep the slice
  * model synchronized with any AdocTreeModel changes.
  *
  * This class should not be directly instantiated, but rather it defines several pure virtual functions
  * that delegate necessary responsibility to other client classes.
  *
  * The mappedNodes list contains the top-level items and sliceHash_ contains the individual slices whose
  * list items comprise the second-level of the tree.
  *
  * >> Sorting:
  * SliceModel is intended to purely work with AdocTreeModel source models. Consequently, all sorting is
  * done at two levels - groups (folders) and items. Groups always appear at the top of the list and then
  * the items below the groups. These are then sorted by themselves. Group sorting is always done lexically
  * because it purely contains lexical names; however, items contain several columns some of which are
  * purely numerical. Since only subclasses will know the data types of any given column, subclasses must
  * specify which columns are numerical by overriding the virtual isNumericalColumn method. In the base
  * class implementation here, it always returns false.
  *
  * Thus, this class provides both lexical (default) and numerical sorting. This actually sorts the internal
  * slice data structure QLists and issues a reset. To enable numerical sorting on a specific column, subclasses
  * simply have to override isNumericalColumn and return true for the relevant columns.
  *
  * >> Drag and drop:
  * Selection of items may only be done on a single column which is designated by subclasses via the pure
  * virtual primaryColumn method. Similarly, the columns which may be edited are determined by subclasses
  * with the pure virtual isEditableColumn method.
  *   Given this setup, it is now possible to completely implement the flags method in this base class for
  * all slice models. Indices corresponding to group nodes for the primaryColumn will be handled via the
  * source AdocTreeModel. Otherwise all selectable items are draggable.
  *   The specific drag and drop methods are all passed to the source AdocTreeModel. Ultimately, the flags
  * determines which items may do what and the ItemView's utilize these to determine which actions are
  * appropriate.
  *
  * >> Other notes:
  * An individual slice typically contains at least one column, but may contain multiple types of data.
  * Therefore, it is not possible to uniformly assign a specific column count and indices to all data
  * cells. To minimize the effort needed for subclasses, a maxColumn function is called that allows
  * the subclass to specify the maximium column possible for a given slice and row combination.
  *
  * The rowsAboutToBeInserted signal for the source tree model cannot be processed and re-emitted because
  * slice model performs row filtering. When this signal is received, no rows have been inserted in the
  * source model and without this information it is impossible to know which rows will be inserted. Rather
  * these beginInsertRows() and endInsertRows() are called from within the sourceRowsInserted slot.
  */
class SliceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit SliceModel(QObject *parent = 0);                                   //!< Basic constructor; sets pointers to zero
    virtual ~SliceModel();                                                      //!< Standard destructor; releases any dynamically allocated memory

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    virtual int columnCount(const QModelIndex &parent) const = 0;               //!< Returns the number of columns beneath parent
    //! Returns the data associated with index for the given role
    virtual QVariant data(const QModelIndex &index, int role) const = 0;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;                //!< Returns the item flags for the given index
    //! Returns the index for the model item specified by row, column, and parent
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;                 //!< Returns the parent of the model item with the given index; returns an invalid QModelIndex if index does not have a parent
    virtual bool removeRows(int row, int count, const QModelIndex &parent);     //!< Remove count rows starting with row from the given parent (which must be a top level item)
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
    AdocTreeModel *adocTreeModel() const;                                       //!< Returns the currently associated AdocTreeModel
    void clear();                                                               //!< Clears the internal data structures and releases any dynamically allocated slices and issues a model reset
    virtual int defaultSortColumn() const;                                      //!< Returns the integer position of the column to utilize for sorting purposes; this base class implementation returns 0
    QModelIndex indexFromNode(AdocTreeNode *parent) const;                      //!< Returns the QModelIndex associated with the top-level parent
    virtual bool isEditableColumn(int column) const = 0;                        //!< Pure virtual method that returns true if column is editable or false otherwise
    bool isGroupItem(const QModelIndex &index) const;                           //!< Returns true if index belongs to this model as a slice item and represents an AdocTreeNode::GroupType or false otherwise
    //! Virtual method that returns true if the data in column solely contains numbers; always returns false in the base class implementation
    void loadSlice(const QModelIndex &sourceParentIndex);                       //!< Dynamically loads the slice for sourceParentIndex if it is not already loaded
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;               //!< Returns the source tree model index that corresponds to the proxyIndex in this model
    virtual int primaryColumn() const = 0;                                      //!< Returns the primary column for this model; useful for drag and drop
    void releaseSlice(const QModelIndex &sourceParentIndex);                    //!< Decrements the reference count of any slice associated with sourceParentIndex and releases its memory if the reference count is zero
    void setTreeModel(AdocTreeModel *adocTreeModel);                            //!< Sets the underlying AdocTreeModel to adocTreeModel; does not take ownership
    //! Virtual stub for customizing the horizontal header
    virtual void tweakHorizontalHeader(QHeaderView *header) const               { Q_UNUSED(header); }

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    virtual Slice *createSlice(AdocTreeNode *parent) const;                     //!< Factory method for creating and returning the relevant slice from among all children of parent
    //! Factory method responsible for creating and returning a Slice for parent from start to end inclusive; if start and end are both -1, then retrieve the contents of the entire row
    virtual Slice *createSlice(AdocTreeNode *parent, int start, int end) const = 0;
    Slice *getParentSlice(const QModelIndex &index) const;                      //!< Returns the parent Slice containing index or zero if there is none or index does not reference a valid slice row
    QModelIndex indexFromSlice(Slice *slice) const;                             //!< Returns the QModelIndex associated with slice or an invalid QModelIndex if slice is not found

    // ------------------------------------------------------------------------------------------------
    // Internal virtual "signals" called whenever a specific slice change event occurs
    // The base class implementation does nothing. These are not intended to be called outside of this class
    // interface or undefined behavior may result.
    //! Called when slice is about to be removed from the SliceModel
    virtual void sliceAboutToBeRemoved(Slice *slice)                            { Q_UNUSED(slice); }
    //! Called with slice has been inserted
    virtual void sliceInserted(Slice *slice)                                    { Q_UNUSED(slice); }
    //! Called when the model is reset and all slices are removed
    virtual void slicesReset()                                                  {}
    //! Called when the rows between start and end (inclusive) of slice are about to be removed
    virtual void sliceRowsAboutToBeRemoved(Slice *slice, int start, int end)    { Q_UNUSED(slice); Q_UNUSED(start); Q_UNUSED(end); }
    //! Called when the rows between start and end (inclusive) of slice are about to be inserted
    virtual void sliceRowsInserted(Slice *slice, int start, int end)            { Q_UNUSED(slice); Q_UNUSED(start); Q_UNUSED(end); }
    //! Called when the rows between start and end (inclusive) of slice have been removed
    virtual void sliceRowsRemoved(Slice *slice, int start, int end)             { Q_UNUSED(slice); Q_UNUSED(start); Q_UNUSED(end); }

    AdocTreeModel *adocTreeModel_;                  //!< Underlying AdocTreeModel
    QList<AdocTreeNode *> mappedNodes_;             //!< All currently mapped AdocTreeNodes with data slices
    QHash<AdocTreeNode *, Slice *> sliceHash_;      //!< Hash linking a mapped AdocTreeNode to its cognate slice

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
    void clearStructures();                         //!< Internal function that frees all slice data and clears the internal data structures
    void lexicalSort(int column, Qt::SortOrder order);
    void numericalSort(int column, Qt::SortOrder order);

    QHash<Slice *, AdocTreeNode *> nodeHash_;       //!< Reverse of sliceHash_ mapping a Slice pointer to its cognate AdocTreeNode; used for retrieving a QModelIndex from a slice pointer (indexFromSlice())

    // Special storage variables solely for handling the removal of items via the sourceRowsAboutToBeRemoved() and sourceRowsRemoved() slots
    QList<QPair<int, int> > sliceRowsToRemove_;     //!< Array of slice item ranges (first = start, second = end) to be removed
    QList<int> mappedNodesToRemove_;                //!< Array of top-level row positions to remove

#ifdef TESTING
    // Provide access to the internal state of this class if we are testing
    friend class TestSliceModel;
#endif
};

#endif // SLICEMODEL_H
