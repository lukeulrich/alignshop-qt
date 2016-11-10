/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TRANSIENTTASKCOLUMNADAPTER_H
#define TRANSIENTTASKCOLUMNADAPTER_H

#include "AbstractColumnAdapter.h"
#include "../../../core/Entities/TransientTask.h"
#include "../../Services/Tasks/ITask.h"

class TransientTaskColumnAdapter : public AbstractColumnAdapter
{
    Q_OBJECT

public:
    enum Columns
    {
        eIdColumn = 0,
        eTypeColumn,
        eNameColumn,
        eProgressColumn,
        eStatusColumn,

        eNumberOfColumns
    };

    TransientTaskColumnAdapter(QObject *parent = 0) : AbstractColumnAdapter(parent)
    {
    }

    int columnCount() const
    {
        return eNumberOfColumns;
    }

    Qt::ItemFlags flags(int column) const
    {
        Qt::ItemFlags defaultFlags = AbstractColumnAdapter::flags(column);

        switch (column)
        {
        case eNameColumn:
            defaultFlags |= Qt::ItemIsEditable;
            break;

        default:
            break;
        }

        return defaultFlags;
    }

    QVariant data(const IEntitySPtr &entity, int column, int role) const
    {
        ASSERT(boost::shared_dynamic_cast<const TransientTask>(entity));
        const TransientTaskSPtr &transientTask = boost::shared_static_cast<TransientTask>(entity);
        ASSERT(transientTask);

        if (role == Qt::DisplayRole)
        {
            switch (column)
            {
            case eIdColumn:
                return transientTask->id();
            case eTypeColumn:
                return "Task";
            case eNameColumn:
                return transientTask->name();
            case eProgressColumn:
                return transientTask->progress();
            case eStatusColumn:
                switch (transientTask->status())
                {
                case Ag::NotApplicable:
                    return "-";
                case Ag::NotStarted:
                    return "Not started";
                case Ag::Starting:
                    return "Starting";
                case Ag::Running:
                    return "Running";
                case Ag::Paused:
                    return "Paused";
                case Ag::Finished:
                    return "Done";
                case Ag::Error:
                    return "Error";
                case Ag::Killed:
                    return "Killed";

                default:
                    break;
                }

            default:
                break;
            }
        }

        return QVariant();
    }

    QVariant headerData(int column) const
    {
        switch (column)
        {
        case eIdColumn:
            return "ID";
        case eTypeColumn:
            return "Type";
        case eNameColumn:
            return "Name";
        case eProgressColumn:
            return "Progress";
        case eStatusColumn:
            return "Status";

        default:
            return QVariant();
        }
    }


protected:
    bool setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value)
    {
        ASSERT(entity);
        ASSERT(boost::shared_dynamic_cast<const TransientTask>(entity));
        TransientTaskSPtr transientTask = boost::shared_static_cast<TransientTask>(entity);

        switch (column)
        {
        case eNameColumn:
            transientTask->setName(value.toString());
            break;
        case eProgressColumn:
            {
                bool ok = false;
                double newProgress = value.toDouble(&ok);
                if (!ok)
                    return false;
                transientTask->setProgress(newProgress);
            }
            break;
        case eStatusColumn:
            {
                bool ok = false;
                int newStatus = value.toInt(&ok);
                if (!ok)
                    return false;

                transientTask->setStatus(static_cast<Ag::TaskStatus>(newStatus));
            }
            break;

        default:
            return false;
        }

        emit dataChanged(entity, column);

        return true;
    }
};

#endif // TRANSIENTTASKCOLUMNADAPTER_H
