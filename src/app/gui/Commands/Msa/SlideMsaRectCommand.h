/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SLIDEMSARECTCOMMAND_H
#define SLIDEMSARECTCOMMAND_H

#include "../AbstractSkipFirstRedoCommand.h"
#include "../../../core/util/PosiRect.h"
#include "../../../core/global.h"

class AbstractMsaView;
class ObservableMsa;

class SlideMsaRectCommand : public AbstractSkipFirstRedoCommand
{
public:
    SlideMsaRectCommand(AbstractMsaView *msaView,
                        ObservableMsa *msa,
                        const PosiRect &msaRect,
                        int delta,
                        QUndoCommand *parentCommand = nullptr);

    void redoDelegate();
    void undo();

private:
    AbstractMsaView *msaView_;
    ObservableMsa *msa_;
    PosiRect msaRect_;
    int delta_;
};

#endif // SLIDEMSARECTCOMMAND_H
