/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include <PrimerDesign/SequenceListModel.h>

using namespace PrimerDesign;

namespace Col
{
    enum Sequence
    {
        NAME,
        MAX
    };
}

SequenceListModel::SequenceListModel()
    : ListModel(1, QList<QString>() << "Name")
{
}

QVariant SequenceListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && list())
    {
        ObservablePrimerPairGroupList *groups = (ObservablePrimerPairGroupList*)list();
        PrimerPairGroup *group = groups->at(index.row());
        switch (index.column())
        {
            case Col::NAME:
                return QVariant(group->sequence.name());
            default:
                QVariant();
        }
    }

    return QVariant();
}

bool SequenceListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && list())
    {
        ObservablePrimerPairGroupList *groups = (ObservablePrimerPairGroupList*)list();
        PrimerPairGroup *group = groups->at(index.row());
        switch (index.column())
        {
            case Col::NAME:
                return group->sequence.setName(value.toString());
            default:
                return false;
        }
    }
    else
    {
        return false;
    }
}

void SequenceListModel::unbindCurrent()
{
    ObservablePrimerPairGroupList *groups = (ObservablePrimerPairGroupList*)list();
    if (groups && groups->current())
    {
        QObject::disconnect(
            &groups->current()->sequence,
            SIGNAL(nameChanged(ObservableSequence*)),
            this,
            SLOT(onNameChanged(ObservableSequence*)));
    }
}

void SequenceListModel::bindCurrent()
{
    ObservablePrimerPairGroupList *groups = (ObservablePrimerPairGroupList*)list();
    if (groups && groups->current())
    {
        QObject::connect(
            &groups->current()->sequence,
            SIGNAL(nameChanged(ObservableSequence*)),
            this,
            SLOT(onNameChanged(ObservableSequence *)));
    }
}

void SequenceListModel::onNameChanged(ObservableSequence *sequence)
{
    ObservablePrimerPairGroupList *groups = (ObservablePrimerPairGroupList*)list();
    if (groups)
    {
        for (int i = 0; i < groups->length(); ++i)
        {
            if (&groups->at(i)->sequence == sequence)
            {
                QModelIndex itemIndex = index(i, 0);
                emit dataChanged(itemIndex, itemIndex);
                return;
            }
        }
    }
}

void SequenceListModel::bind(ObservableListBase *list)
{
    ListModel::bind(list);
    if (list)
    {
        QObject::connect(list->notifier(), SIGNAL(currentChanging()), this, SLOT(unbindCurrent()));
        QObject::connect(list->notifier(), SIGNAL(currentChanged()), this, SLOT(bindCurrent()));
    }
}
