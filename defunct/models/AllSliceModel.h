/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ALLSLICEMODEL_H
#define ALLSLICEMODEL_H

#include "SliceProxyModel.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QHeaderView;

/**
  * AllSliceModel functions as a simple, pass-through slice model for displaying all the immediate
  * items beneath a given node of an AdocTreeModel.
  *
  * This class positions AllSliceModel on the same model chain level as {Amino,Dna,Rna}SliceModel
  * and makes it possible to polymorphically handle all these from a single interface.
  */
class AllSliceModel : public SliceProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit AllSliceModel(QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    int columnCount(const QModelIndex &parent = QModelIndex()) const;                       //!< Returns the number of columns for the children items beneath parent
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;              //!< Returns the data associated for the given index and role
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole); //!< Sets the data stored by the item referred to by index to value for role

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void tweakHorizontalHeader(QHeaderView *header) const;                                  //!< Tweaks the appearance of the horizontal headerview
};

#endif // ALLSLICEMODEL_H
