/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef VARYINGCOLUMNPROXYMODEL_H
#define VARYINGCOLUMNPROXYMODEL_H

#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtGui/QAbstractProxyModel>

#include <QtDebug>

/**
  * VaryingColumnProxyModel provides a mechanism for defining a varying number of columns external
  * to a source QAbstractItemModel.
  *
  * QSortFilterProxyModel enforces an upper limit on the number of columns available to those of its
  * source model. This presents a problem for those cases where we have a core model which contains
  * heterogeneous data with different number of columns depending upon the exact data type. This is
  * particularly cumbersome if a group item can immediately hold heterogeneous items. Moreover, setting
  * a columnCount() for one data type on the core/root model would cascade this change to all attached
  * views - an undesirable result.
  *
  * Thus, this class provides a 1:1 transparent proxy mapping to the source model, but provides a method
  * for setting the number of desired columns. The core data input/output is still deferred to the core
  * model although it could certainly be tweaked with derivatives of this class.
  *
  * An example usage might be the following:
  *
  * o Given: a core tree model with only one global column (its label)
  * VaryingColumnProxyModel aminoProxyModel;
  * aminoProxyModel.setColumnCount(9)
  * aminoProxyModel.setSourceModel(sourceModel)
  *
  * Sorting and filtering could be layered on with additional proxy models as necessary.
  *
  * Currently only provides support for varying the number of columns and not the number of rows.
  *
  * The trick to making this proxy function as expected involves storing the mapped parents in their parent
  * nodes.
  *
  * To ensure that drag and drop works as expected through this model, it is vital to reimplement the following
  * functions and redirect to the source model (see docs for full function signatures):
  * o dropMimeData()
  * o mimeData()
  * o mimeTypes()
  * o supportedDropActions()
  *
  * Shortcomings/future issues:
  * o Changing the number of columns after a view and model have been configured will not automatically
  *   trigger an update.
  * o ColumnsAboutToBeRemoved, ColumnsRemoved, ColumnsAboutToBeInserted, ColumnsInserted signals are not
  *   transferred to source model
  */
class VaryingColumnProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    VaryingColumnProxyModel(QObject *parent = 0, const int nColumns = 1);       //!< Construct an instance with columns set to nColumns
    virtual ~VaryingColumnProxyModel();                                         //!< Destructor; frees memory utilized by sourceIndexMap_

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;   //!< Always returns nColumns regardless of parent
    //! Returns the proxy index of the item in the model specified by row, column, and parent
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;    //!< Returns the proxy index that corresponds to sourceIndex
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;       //!< Returns the source index that corresponds to proxyIndex
    virtual QModelIndex parent(const QModelIndex &index) const;                 //!< Returns the parent of the model item with the given index; returns an invalid QModelIndex if index does not have a parent
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;      //!< Returns the number of rows under the given parent
    void setColumnCount(int nColumns);                                          //!< Set the number of columns to nColumns (minimum of zero)
    virtual void setSourceModel(QAbstractItemModel *sourceModel);               //!< Set the source model to sourceModel

    // Drag and drop methods - do not know how to test these pass through methods
    // UNTESTED
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual Qt::DropActions supportedDropActions() const;

private slots:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void sourceModelReset();
    void sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsInserted(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end);

private:
    // ------------------------------------------------------------------------------------------------
    // Private support structures
    //! Mapping structure
    struct ParentMap
    {
        QModelIndex sourceParent_;          //!< Stored parent index
        QVector<QModelIndex> mappedRows_;   //!< Array of child source rows (indexes) that have been mapped
    };

    // ------------------------------------------------------------------------------------------------
    // Private methods
    ParentMap *parentMapFromProxyIndex(const QModelIndex &proxyIndex) const;    //!< Return the ParentMap associated with proxyIndex
    ParentMap *parentMapFromSourceIndex(const QModelIndex &sourceIndex) const;  //!< Return the ParentMap associated with sourceIndex, creating it if necessary
    void recurseRemoveFromSourceIndexMap(const QModelIndex &sourceIndex) const; //!< Recursively removes all references stored in the source index map present beneath sourceIndex
    void clearSourceIndexMap();                                                 //!< Clears and deallocates all memory stored in the sourceIndexMap_

    // ------------------------------------------------------------------------------------------------
    // Private members
    mutable QHash<QModelIndex, ParentMap *> sourceIndexMap_;    //!< Associates a source model index with a ParentMap
    int nColumns_;

};

#endif // VARYINGCOLUMNPROXYMODEL_H
