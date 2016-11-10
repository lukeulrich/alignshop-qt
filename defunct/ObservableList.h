/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef OBSERVABLELIST_H
#define OBSERVABLELIST_H

#include <QtCore>
#include "ObservableListNotifier.h"

// Performs "op" operation on each item of type "T" in list "lst".
#define EACH(lst, T, op) for (int _i = 0; _i < lst.length(); ++_i) { T = lst[_i]; op; }
#define IS_VALID_INDEX(i) (i >= 0 && i < items_.length())

// Returns "false" if index "i" is invalid.
// If false, this will exit the function in which it is called.
#define VERIFY_INDEX(i) if (!IS_VALID_INDEX(i)) { return false; }

namespace Row
{
    namespace ObservableList
    {
        /// The value of the current index, if there is no current row.
        static const int NO_CURRENT = -1;
    }
}

using namespace Row::ObservableList;

/// The base observable list class, which contains item-type-agnostic methods and properties.  This class exists so
/// that the template class ObservableList can be generically consumed by type-agnostic methods and classes.
class ObservableListBase
{
public:
    /// Clears the list.
    virtual void clear() = 0;

    /// Gets the value of the current index.
    /// @return int
    virtual int currentIndex() const = 0;

    /// Gets the length of the list.
    /// @return int
    virtual int length() const = 0;

    /// Gets the object which raises events when the list is modified.
    /// @return ObservableListNotifier*
    virtual const ObservableListNotifier *notifier() const = 0;

    /// Removes the element at the specified index.
    /// @param i [int] The index of the item to be removed.
    /// @return bool False if the index was invalid.
    virtual bool removeAt(int i) = 0;

    /// Sets the current index.
    /// @param i [int] The current index.
    /// @return bool False if the index was invalid.
    virtual bool setCurrentIndex(int i) = 0;
};

/// An observable, templated list.
template <class T>
class ObservableList : public ObservableListBase
{
public:
    /// The default constructor.
    ObservableList()
    {
        currentIndex_ = NO_CURRENT;
    }

    /// The destructor.
    ~ObservableList()
    {
        notifier_.onListDeleting();
        EACH(items_, T* item, delete item);
    }

    /// Gets the value at index i.
    /// @param i [int]
    /// @return T*
    T * const operator[](int i) const
    {
        return at(i);
    }

    /// Adds an item to the list.
    /// @param item [T*]
    void add(T *item)
    {
        int index = items_.length();
        notifier_.onItemAdding(index);
        items_.append(item);
        notifier_.onItemAdded(index);
    }

    /// Gets the value at index i.
    /// @param i [int]
    /// @return T*
    T * const at(int i) const
    {
        VERIFY_INDEX(i);
        return items_[i];
    }

    void clear()
    {
        notifier_.onMajorChanging();
        setCurrentIndex(NO_CURRENT);
        items_.clear();
        notifier_.onMajorChanged();
    }

    /// Gets the current item or null if no current item exists.
    /// @return T*
    T * current() const
    {
        VERIFY_INDEX(currentIndex_);
        return items_[currentIndex_];
    }

    /// Gets the current index or Row::ObservableList::NO_CURRENT if no current item exists.
    /// @return int
    int currentIndex() const
    {
        return currentIndex_;
    }

    /// Gets the index of the specified item, starting at the specified position.
    /// @param item [T*]
    /// @param from [int]
    /// @return int
    int indexOf(T * const item, int from = 0) const
    {
        return items_.indexOf(item, from);
    }

    /// Gets the length of the list.
    /// @return int
    int length() const
    {
        return items_.length();
    }

    /// Gets the object which raises events when the list is modified.
    /// @return ObservableListNotifier*
    const ObservableListNotifier *notifier() const
    {
        return &notifier_;
    }

    /// Removes the element at the specified index.
    /// @param i [int] The index of the item to be removed.
    /// @return bool False if the index was invalid.
    bool removeAt(int i)
    {
        VERIFY_INDEX(i);
        notifier_.onItemRemoving(i);
        delete items_[i];
        items_.removeAt(i);
        if (currentIndex_ == i)
        {
            setCurrentIndex(NO_CURRENT);
        }

        notifier_.onItemRemoved(i);
        return true;
    }

    /// Sets the current index.
    /// @param i [int] The current index.
    /// @return bool False if the index was invalid.
    bool setCurrentIndex(int i)
    {
        if (i == NO_CURRENT || IS_VALID_INDEX(i))
        {
            notifier_.onCurrentChanging();
            currentIndex_ = i;
            notifier_.onCurrentChanged();
            return true;
        }

        return false;
    }

private:
    int currentIndex_;
    QList<T *> items_;
    ObservableListNotifier notifier_;
};

#endif // OBSERVABLELIST_H
