/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "ObservableListNotifier.h"

void ObservableListNotifier::onCurrentChanged()
{
    emit currentChanged();
}

void ObservableListNotifier::onCurrentChanging()
{
    emit currentChanging();
}

void ObservableListNotifier::onItemAdded(int i)
{
    emit itemAdded(i);
}

void ObservableListNotifier::onItemAdding(int i)
{
    emit itemAdding(i);
}

void ObservableListNotifier::onItemRemoved(int i)
{
    emit itemRemoved(i);
}

void ObservableListNotifier::onItemRemoving(int i)
{
    emit itemRemoving(i);
}

void ObservableListNotifier::onListDeleting()
{
    emit listDeleting();
}

void ObservableListNotifier::onMajorChanged()
{
    emit majorChanged();
}

void ObservableListNotifier::onMajorChanging()
{
    emit majorChanging();
}

