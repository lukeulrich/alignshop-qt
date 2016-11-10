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
  * @param mpttNodeList [QList<MpttNode *> &]
  */
void freeMpttList(QList<MpttNode *> &mpttNodeList)
{
    foreach (const MpttNode *mpttNode, mpttNodeList)
    {
        if (mpttNode->treeNode_)
            delete mpttNode->treeNode_;
    }
    qDeleteAll(mpttNodeList);
    mpttNodeList.clear();
}
