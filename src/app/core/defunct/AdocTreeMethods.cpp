/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeMethods.h"
#include "AdocTreeNode.h"

#include "Entities/TransientTask.h"

#include "enums.h"
#include "macros.h"

/**
  */
bool AdocTreeMethods::containsNotOverTask(AdocTreeNode *adocTreeNode)
{
    AdocTreeNode::ConstIterator it = adocTreeNode;
    for (AdocTreeNode::ConstIterator end = it->nextAscendant(); it != end; ++it)
    {
        if ((*it).nodeType_ != eTransientTaskNode)
            continue;

        ASSERT((*it).entity() != nullptr);
        TransientTask *transientTask = static_cast<TransientTask *>((*it).entity());
        if (transientTask->task() != nullptr && !transientTask->task()->isOver())
            return true;
    }

    return false;
}
