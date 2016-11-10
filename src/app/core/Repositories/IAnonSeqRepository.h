/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IANONSEQREPOSITORY_H
#define IANONSEQREPOSITORY_H

#include "IRepository.h"

class Seq;

template<typename T>
class IAnonSeqRepository : public virtual IRepository
{
public:
    virtual typename T::SPtr findBySeq(const Seq &seq) = 0;
    virtual typename T::SPtr findBySeqOrCreate(const Seq &seq) = 0;
};

#endif // IANONSEQREPOSITORY_H
