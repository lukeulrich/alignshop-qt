/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FASTAMSAEXPORTER_H
#define FASTAMSAEXPORTER_H

#include "IMsaExporter.h"

/**
  * FastaMsaExplorer exports multiple sequence alignments into the aligned FASTA format.
  */
class FastaMsaExporter : public IMsaExporter
{
public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    //!< Export abstractMsa to device in the aligned FASTA format
    void exportMsa(const AbstractMsa &abstractMsa, QIODevice &device);
};

#endif // FASTAMSAEXPORTER_H
