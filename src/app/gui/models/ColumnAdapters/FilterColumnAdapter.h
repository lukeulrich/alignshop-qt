/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FILTERCOLUMNADAPTER_H
#define FILTERCOLUMNADAPTER_H

#include <QtCore/QVector>
#include <QtCore/QSet>

#include "IFilterColumnAdapter.h"
#include "../../../core/global.h"

class IEntity;

class FilterColumnAdapter : public IFilterColumnAdapter
{
    Q_OBJECT

public:
    FilterColumnAdapter(QObject *parent = nullptr);

    int columnCount() const;
    QVariant data(const IEntitySPtr &entity, int proxyColumn, int role) const;
    void exclude(int sourceColumn);
    Qt::ItemFlags flags(int proxyColumn) const;
    QVariant headerData(int proxyColumn) const;
    void include(int sourceColumn);
    int mapFromSource(int sourceColumn) const;
    int mapToSource(int proxyColumn) const;
    bool setData(const IEntitySPtr &entity, int proxyColumn, const QVariant &value);
    bool setData(const IEntitySPtr &entity, int proxyColumn, const QVariant &value, bool allowUndo);
    void setSourceAdapter(IColumnAdapter *sourceAdapter);
    IColumnAdapter *sourceAdapter() const;

    QUndoStack *undoStack() const { return nullptr; }

protected:
    void setUndoStack(QUndoStack * /* undoStack */) {}


private Q_SLOTS:
    void onSourceDataChanged(const IEntitySPtr &entity, int sourceColumn);


private:
    bool isValidProxyColumn(int proxyColumn) const;
    bool isValidSourceColumn(int sourceColumn) const;
    void updateMapping();                               //!< Updates the filterSourceMapping_

    IColumnAdapter *sourceAdapter_;
    QVector<int> filterSourceMapping_;                  //!< Maps a filter column to its source columns
    QSet<int> excludedColumns_;
};

#endif // FILTERCOLUMNADAPTER_H
