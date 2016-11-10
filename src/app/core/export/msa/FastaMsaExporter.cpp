/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QIODevice>

#include "FastaMsaExporter.h"

#include "../../Entities/AbstractMsa.h"
#include "../../Entities/AbstractSeq.h"
#include "../../ObservableMsa.h"
#include "../../misc.h"


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Throws an exception if the following conditions are not met:
  * 1) abstractMsa has a non-null ObservableMsa
  * 2) the ObservableMsa contains at least one sequence
  * 3) Every Subseq has a non-null sequence entity
  *
  * @param abstractMsa [const AbstractMsa &]
  * @param device [QIODevice &]
  */
void FastaMsaExporter::exportMsa(const AbstractMsa &abstractMsa, QIODevice &device)
{
    ObservableMsa *msa = abstractMsa.msa();
    if (msa == nullptr)
        throw "Msa entity does not have a valid msa instance.";

    if (msa->rowCount() == 0)
        throw "Msa does not contain any sequences.";

    if (!device.isOpen())
        throw "Output device is not open.";

    if (!(device.openMode() & QIODevice::WriteOnly))
        throw "Unable to write to output device.";

    for (int i=1, z=msa->rowCount(); i<=z; ++i)
    {
        const Subseq *subseq = msa->at(i);
        if (!subseq->seqEntity_)
            throw QString("No sequence entity associated with sequence, %1").arg(i);

        if (writeAll(device, ">") == -1 ||
            writeAll(device, subseq->seqEntity_->name().toAscii()) == -1 ||
            writeAll(device, "\n") == -1 ||
            writeAll(device, subseq->asByteArray()) == -1 ||
            writeAll(device, "\n") == -1)
        {
            throw QString("Error writing to device: %1").arg(device.errorString());
        }
    }
}
