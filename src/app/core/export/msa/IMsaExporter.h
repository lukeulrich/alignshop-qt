/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSAEXPORTER_H
#define IMSAEXPORTER_H

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QIODevice;

class AbstractMsa;

/**
  * IMsaExplorer provides a common interface for the msa exporter interface.
  */
class IMsaExporter
{
public:
    virtual ~IMsaExporter()    {}

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //!< Export abstractMsa to device in the aligned FASTA format
    virtual void exportMsa(const AbstractMsa &abstractMsa, QIODevice &device) = 0;
};

#endif // IMSAEXPORTER_H
