/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ADOCSORTFILTERPROXYMODEL_H
#define ADOCSORTFILTERPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

/**
  * AdocSortFilterProxyModel simply enhances the default QSortFilterProxyModel with additional methods
  * and a common polymorphic base class for these Adoc specific methods.
  */
class AdocSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit AdocSortFilterProxyModel(QObject *parent = 0);         //!< Trivial constructor

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool lessThan(const QModelIndex &a, const QModelIndex &b) const;
    virtual void tweakHorizontalHeader(QHeaderView *header) const;  //!< Tweak the appearance of the horizontal header header; this base class version does nothing
};

#endif // ADOCSORTFILTERPROXYMODEL_H
