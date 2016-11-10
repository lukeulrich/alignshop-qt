/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSAREPOSITORY_H
#define IMSAREPOSITORY_H

#include "../Entities/AbstractMsa.h"

/**
  * find(...) -> reads the annotation
  * save(...) -> saves the annotation and the alignment data if it is loaded
  * loadAlignment(T *) -> loads the alignment data for the given msa entity; if already loaded, simply returns
  * unloadAlignment(T *) -> releases the alignment data regardless of its current state. Any unsaved changes are not persisted
  */
class IMsaRepository
{
public:
    virtual int beginLoadAlignment(const AbstractMsaSPtr &msaEntity) = 0;       // Returns the total number of steps to load this alignment
    virtual void cancelLoadAlignment() = 0;                 // Cancels the alignment
    virtual void endLoadAlignment() = 0;                    // Perform any necessary teardown after the load is complete
    virtual int loadAlignmentStep(int stepsToTake) = 0;     // Fetch stepsToTake steps of the load process

    virtual void unloadAlignment(const AbstractMsaSPtr &msaEntity) = 0;         // Unloads the alignment data for msaEntity if it is loaded

    virtual void unloadDeadSubseqs(const AbstractMsaSPtr &msaEntity) = 0;  // Frees the memory occupied by dead Subseq pointers
    virtual bool expungeDeadSubseqs(const AbstractMsaSPtr &msaEntity) = 0;  // Persists the removal of dead seq entities from the msaEntity
};

#endif // IMSAREPOSITORY_H
