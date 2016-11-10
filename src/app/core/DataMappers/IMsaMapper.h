/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IMSAMAPPER_H
#define IMSAMAPPER_H

#include "IEntityMapper.h"

template<typename T>
class IMsaMapper : public virtual IEntityMapper<T>
{
public:
    virtual int beginLoadAlignment(T *msaEntity) = 0;       // Returns the total number of steps to load this alignment
    virtual void cancelLoadAlignment() = 0;                 // Cancels the alignment
    virtual void endLoadAlignment() = 0;                    // Perform any necessary teardown after the load is complete
    virtual int loadAlignmentStep(int stepsToTake) = 0;     // Fetch stepsToTake steps of the load process

    virtual void unloadAlignment(T *msaEntity) = 0;         // De-allocates the alignment
    virtual void unloadDeadSubseqs(T *msaEntity) const = 0; // Unfinds all associated Seq entities and releases their associated Subseq pointer memory; does not erase the dead subseqs from persistent storage
    virtual bool expungeDeadSubseqs(T *msaEntity) const = 0;// Erases all dead subseqs from persistent storage, but does not free their memory
};

#endif // IMSAMAPPER_H
