/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef OBSERVABLELISTNOTIFIER_H
#define OBSERVABLELISTNOTIFIER_H

#include <QtCore>

/// This class is responsible for raising events to notify of changes to an observable list.
class ObservableListNotifier : public QObject
{
    Q_OBJECT

public:
    /// Raises the current changed event.
    void onCurrentChanged();

    /// Raises the current changing event.
    void onCurrentChanging();

    /// Raises the item added event.
    /// @param i [int] The index of the added item.
    void onItemAdded(int i);

    /// Raises the item adding event.
    /// @param i [int] The index of the added item.
    void onItemAdding(int i);

    /// Raises the item removed event.
    /// @param i [int] The index of the removed item.
    void onItemRemoved(int i);

    /// Raises the item removing event.
    /// @param i [int] The index of the removed item.
    void onItemRemoving(int i);

    /// Raises the list deleting event, indicating that the entire list is being destroyed.
    void onListDeleting();

    /// Raises the on major changing event, indicating that a major change is about to occur.
    void onMajorChanging();

    /// Raises the on major changed event, indicating that a major change occurred.
    void onMajorChanged();

signals:
    /// The current changed event.
    void currentChanging();

    /// The current changing event.
    void currentChanged();

    /// The item added event.
    /// @param i [int] The index of the added item.
    void itemAdded(int i);

    /// The item adding event.
    /// @param i [int] The index of the added item.
    void itemAdding(int i);

    /// The item removed event.
    /// @param i [int] The index of the removed item.
    void itemRemoved(int i);

    /// The item removing event.
    /// @param i [int] The index of the removed item.
    void itemRemoving(int i);

    /// The list deleting event, indicating that the entire list is being destroyed.
    void listDeleting();

    /// The on major changing event, indicating that a major change is about to occur.
    void majorChanging();

    /// The on major changed event, indicating that a major change occurred.
    void majorChanged();
};

#endif // OBSERVABLELISTNOTIFIER_H
