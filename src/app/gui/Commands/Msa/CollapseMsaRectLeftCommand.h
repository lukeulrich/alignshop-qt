/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COLLAPSEMSARECTLEFTCOMMAND_H
#define COLLAPSEMSARECTLEFTCOMMAND_H

#include "AbstractCollapseMsaRectCommand.h"

class ObservableMsa;
class PosiRect;
class QUndoCommand;

class CollapseMsaRectLeftCommand : public AbstractCollapseMsaRectCommand
{
public:
    CollapseMsaRectLeftCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand = nullptr);

    void redo();
    int subId() const;
};

#endif // COLLAPSEMSARECTLEFTCOMMAND_H
