/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTCOLUMNADAPTER_H
#define ABSTRACTCOLUMNADAPTER_H

#include <QtGui/QUndoStack>
#include "../../Commands/SetEntityDataCommand.h"

#include "IColumnAdapter.h"

class AbstractColumnAdapter : public IColumnAdapter
{
    Q_OBJECT

public:
    virtual Qt::ItemFlags flags(int column) const
    {
        Q_UNUSED(column);

        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    virtual QVariant data(const IEntitySPtr &entity, int column, int role = Qt::DisplayRole) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(column);
        Q_UNUSED(role);

        return QVariant();
    }

    virtual QVariant headerData(int column) const
    {
        Q_UNUSED(column);

        return QVariant();
    }

    /**
      * Convenience method that always uses the undostack if one is defined.
      */
    virtual bool setData(const IEntitySPtr &entity, int column, const QVariant &value)
    {
        return setData(entity, column, value, true);
    }

    /**
      * If allowUndo is true, then this command will be pushed onto the undo stack.
      */
    virtual bool setData(const IEntitySPtr &entity, int column, const QVariant &value, bool allowUndo)
    {
        if (!entity)
            return false;

        // If the values are the same, do nothing
        if (value == data(entity, column))
            return true;

        if (allowUndo && undoStack_ != nullptr)
        {
            undoStack_->push(new SetEntityDataCommand(this, entity, column, value));
            return true;
        }

        return setDataPrivate(entity, column, value);
    }

    virtual void setUndoStack(QUndoStack *undoStack)
    {
        undoStack_ = undoStack;
    }

    QUndoStack *undoStack() const
    {
        return undoStack_;
    }


protected:
    AbstractColumnAdapter(QObject *parent = 0)
        : IColumnAdapter(parent)
    {
    }

    virtual bool setDataPrivate(const IEntitySPtr &entity, int column, const QVariant &value) = 0;

    QUndoStack *undoStack_;

    friend class SetEntityDataCommand;  // For access to setDataPrivate
};

#endif // ABSTRACTCOLUMNADAPTER_H
