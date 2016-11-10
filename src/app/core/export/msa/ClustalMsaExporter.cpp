/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "ClustalMsaExporter.h"
#include "../../Entities/AbstractMsa.h"
#include "../../Entities/AbstractSeq.h"
#include "../../ObservableMsa.h"
#include "../../macros.h"
#include "../../misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param abstractMsa [const AbstractMsa &]
  * @param device [QIODevice &]
  */
void ClustalMsaExporter::exportMsa(const AbstractMsa &abstractMsa, QIODevice &device)
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

    int targetLength = lengthOfLongestName(abstractMsa.msa());
    if (targetLength == 0)
        throw "No sequences were found with a valid name.";

    if (writeAll(device, "CLUSTALW\n\n") == -1)
        throw QString("Error writing to device: %1").arg(device.errorString());

    for (int i=1, z=msa->rowCount(); i<=z; ++i)
    {
        const Subseq *subseq = msa->at(i);
        if (!subseq->seqEntity_)
            throw QString("No sequence entity associated with sequence, %1").arg(i);

        if (writeAll(device, paddedName(subseq->seqEntity_->name(), targetLength)) == -1 ||
            writeAll(device, " ") == -1 ||
            writeAll(device, subseq->asByteArray()) == -1 ||
            writeAll(device, "\n") == -1)
        {
            throw QString("Error writing to device: %1").arg(device.errorString());
        }
    }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * If a subseq does not have an associated sequence entity, it is ignored.
  *
  * @param msa [ObservableMsa *]
  * @returns int
  */
int ClustalMsaExporter::lengthOfLongestName(ObservableMsa *msa) const
{
    ASSERT(msa != nullptr);

    int longest = 0;
    for (int i=1, z=msa->rowCount(); i<=z; ++i)
    {
        const Subseq *subseq = msa->at(i);
        if (!subseq->seqEntity_)
            continue;

        if (subseq->seqEntity_->name().length() > longest)
            longest = subseq->seqEntity_->name().length();
    }

    return longest;
}

/**
  * @param name [const QString &]
  * @param targetLength [int]
  * @returns QByteArray
  */
QByteArray ClustalMsaExporter::paddedName(const QString &name, int targetLength) const
{
    return name.leftJustified(targetLength, ' ').toAscii();
}
