/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TARCREATOR_H
#define TARCREATOR_H

#include "../util/Pipe.h"
#include "../global.h"

class QIODevice;

/**
  * TarCreator is a source pipe for archiving files in the TAR format.
  */
class TarCreator : public Pipe
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    TarCreator(QIODevice *target, QObject *parent = nullptr);   //!<

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool archive(const QString &fileName);
    void writeEofMarker();                  // Writes two 512-byte blocks of binary zeros to the stream

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    qint64 writeData(const char *data, qint64 len);
};

#endif // TARCREATOR_H
