/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COLLAPSEMSARECTRIGHTCOMMAND_H
#define COLLAPSEMSARECTRIGHTCOMMAND_H

#include "AbstractCollapseMsaRectCommand.h"

class ObservableMsa;
class PosiRect;
class QUndoCommand;

class CollapseMsaRectRightCommand : public AbstractCollapseMsaRectCommand
{
public:
    CollapseMsaRectRightCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand = nullptr);

    void redo();
    int subId() const;
};

#endif // COLLAPSEMSARECTRIGHTCOMMAND_H
