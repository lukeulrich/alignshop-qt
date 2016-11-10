/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QtCore>
#include <QtGui>
#include <ObservableList.h>

namespace PrimerDesign
{
    /// The list model is a QT table model which binds to an observable list.
    class ListModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        /// Initializes the ListModel.
        /// @param editableColumnMask int
        /// @param columns const QList<QString> &
        ListModel(int editableColumnMask, const QList<QString> &columns);

        /// Binds the model to an observable list.
        /// @param list ObservableListBase *
        /// @return void
        virtual void bind(ObservableListBase *list);

        /// Gets the column count.
        /// @param parent const QModelIndex &
        /// @return int
        int columnCount(const QModelIndex &parent) const;

        /// Gets the data for the specified index and role.
        /// @param index const QModelIndex &
        /// @param role int (Defaults to Qt::DisplayRole.)
        /// @return QVariant
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const = 0;

        /// Gets the flags for the specified index, these flags indicate things like whether this index is editable...
        /// @param index const QModelIndex &
        /// @return Qt::ItemFlags
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;

        /// Gets the header data for the specified section, orientation, and role.
        /// @param section int
        /// @param orientation Qt::Orientation
        /// @param role int
        /// @return QVariant
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;

        /// Gets the underlying observable list.
        /// @return ObservableListBase *
        ObservableListBase *list() const;

        /// Gets teh number of rows.
        /// @param parent const QModelIndex & (Defaults to QModelIndex().)
        /// @return int
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

        /// Sets the data.
        /// @param index const QModelIndex &
        /// @param value const QVariant &
        /// @param role int (Defaults to Qt::EditRole.)
        /// @return bool
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) = 0;

    protected:
        /// Unbinds from the underlying list.
        /// @return void
        virtual void unbind();

    private:
        QList<QString> columns_;
        int editableColumnMask_;
        ObservableListBase *list_;

    private slots:
        /// Triggered when there is a major change.
        /// @return void
        void majorChanged();

        /// Triggered before an item is added at the specified index.
        /// @param i int
        /// @return void
        void itemAdding(int i);

        /// Triggered after an item was added at the specified index.
        /// @param i int
        /// @return void
        void itemAdded(int i);

        /// Triggered before an item is removed at the specified index.
        /// @param i int
        /// @return void
        void itemRemoving(int i);

        /// Triggered after an item was removed at the specified index.
        /// @param i int
        /// @return void
        void itemRemoved(int i);

        /// Triggered when the list which this model represents is being removed completely.
        /// @return void
        void listDeleting();
    };
}

#endif // LISTMODEL_H
