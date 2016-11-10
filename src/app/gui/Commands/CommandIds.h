/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COMMANDIDS_H
#define COMMANDIDS_H

namespace Ag
{
    enum CommandIds
    {
        eCollapseMsaRectCommandId = 0,
        eSetSubseqStartCommandId,
        eSetSubseqStopCommandId
    };

    enum SubCommandIds
    {
        eCollapseMsaRectLeftCommandId = 0,
        eCollapseMsaRectRightCommandId
    };
}

#endif // COMMANDIDS_H
