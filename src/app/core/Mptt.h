/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MPTT_H
#define MPTT_H

#include <QtCore/QVector>

#include "MpttNode.h"
#include "TreeNode.h"

namespace Mptt
{
    //! Global function for completely freeing any allocated TreeNodes referenced by mpttNodeList
    template<typename Derived>
    void freeMpttTreeNodes(QVector<MpttNode<Derived> > &mpttNodeVector);

    //! Returns an equivalent representation of treeNode and its descendants as an Mptt node array
    template<typename Derived>
    QVector<MpttNode<Derived> > toMpttVector(Derived *treeNode);

    //! Transforms a set of MpttNodes into a tree structure. Returns the root node on success or mullptr otherwise
    template<typename Derived>
    Derived *fromMpttVector(QVector<MpttNode<Derived> > &mpttNodeVector);
}

#include "Mptt.tpp"


#endif // MPTT_H
