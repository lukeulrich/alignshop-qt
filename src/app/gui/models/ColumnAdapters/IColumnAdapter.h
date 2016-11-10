/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ICOLUMNADAPTER_H
#define ICOLUMNADAPTER_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "../../../core/Entities/IEntity.h"

class QUndoStack;

class IColumnAdapter : public QObject
{
    Q_OBJECT

public:
    virtual ~IColumnAdapter() {}

    virtual int columnCount() const = 0;
    virtual QVariant data(const IEntitySPtr &entity, int column, int role = Qt::DisplayRole) const = 0;
    virtual Qt::ItemFlags flags(int column) const = 0;
    virtual QVariant headerData(int column) const = 0;
    virtual bool setData(const IEntitySPtr &entity, int column, const QVariant &value) = 0;
    virtual bool setData(const IEntitySPtr &entity, int column, const QVariant &value, bool allowUndo) = 0;

    virtual void setUndoStack(QUndoStack *undoStack) = 0;
    virtual QUndoStack *undoStack() const = 0;


Q_SIGNALS:
    void dataChanged(const IEntitySPtr &entity, int column);


protected:
    IColumnAdapter(QObject *parent = 0) : QObject(parent) {}
};

#endif // ICOLUMNADAPTER_H
