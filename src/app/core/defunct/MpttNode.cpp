/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "MpttNode.h"

#include "TreeNode.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Global functions
/**
  * @param mpttNodeList [const QVector<MpttNode> &]
  */
void freeMpttTreeNodes(QVector<MpttNode> &mpttNodeVector)
{
    QVector<MpttNode>::Iterator it = mpttNodeVector.begin();
    while (it != mpttNodeVector.end())
    {
        MpttNode &mpttNode = *it;
        if (mpttNode.treeNode_ != nullptr)
        {
            delete mpttNode.treeNode_;
            mpttNode.treeNode_ = nullptr;
        }

        ++it;
    }
}
