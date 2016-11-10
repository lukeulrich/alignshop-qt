/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CLUSTALMSAEXPORTER_H
#define CLUSTALMSAEXPORTER_H

#include "IMsaExporter.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QByteArray;
class QString;

class ObservableMsa;

/**
  * ClustalMsaExplorer exports multiple sequence alignments into the CLUSTAL format.
  *
  * While there are several options that may be used to customize the final CLUSTAL output, currently implemented the
  * simplest options here:
  * o CLUSTALW header without any version information
  * o One empty line between CLUSTAL header and beginning of alignment section
  * o Full length of name is used
  * o Full aligned sequence per line
  * o Only 1 space padding between longest sequence name and sequence alignment
  * o No consensus data is displayed
  */
class ClustalMsaExporter : public IMsaExporter
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    //!< Export abstractMsa to device in the aligned Clustal format
    void exportMsa(const AbstractMsa &abstractMsa, QIODevice &device);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int lengthOfLongestName(ObservableMsa *msa) const;      //!< Returns the length of the longest sequence entity name in msa
    //! Pads name with space characters up to targetLength and returns the resulting QByteArray
    QByteArray paddedName(const QString &name, int targetLength) const;
};

#endif // CLUSTALMSAEXPORTER_H
