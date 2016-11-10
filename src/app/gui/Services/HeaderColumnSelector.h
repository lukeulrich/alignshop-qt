/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef HEADERCOLUMNSELECTOR_H
#define HEADERCOLUMNSELECTOR_H

#include <QtCore/QObject>
#include <QtCore/QVector>
#include "../../core/global.h"

class QHeaderView;

/**
  * HeaderColumnSelector provides a context menu activated, popup menu of checkboxes for hiding/showing the columns in
  * an item view.
  *
  * No popup menu is displayed if there is no model or no columns in the model.
  */
class HeaderColumnSelector : public QObject
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    HeaderColumnSelector(QObject *parent = nullptr);
    HeaderColumnSelector(QHeaderView *targetHeader, QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QVector<int> defaultColumns() const;
    void setDefaultColumns(const QVector<int> &defaultColumns);
    void setTargetHeader(QHeaderView *targetHeader);
    QHeaderView *targetHeader() const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onContextMenuRequested();


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QHeaderView *targetHeader_;
    QVector<int> defaultColumns_;
};

#endif // HEADERCOLUMNSELECTOR_H
