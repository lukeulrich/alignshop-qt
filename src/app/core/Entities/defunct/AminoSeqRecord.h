/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQRECORD_H
#define AMINOSEQRECORD_H

#include "AbstractSeqRecord.h"
#include "AminoAnonSeq.h"
#include "../DataMappers/AbstractDataMapper.h"
#include "../util/DecrementNumberGenerator.h"

class AminoSeqRecord : public AbstractSeqRecord
{
public:
    AminoSeqRecord(int id, int start, int stop, const QString &name, const QString &description, AminoAnonSeq *aminoAnonSeq)
        : AbstractSeqRecord(id, name, description), aminoAnonSeq_(aminoAnonSeq)
    {
        ASSERT(aminoAnonSeq_);
    }

    AminoAnonSeq *anonSeq() const
    {
        return aminoAnonSeq_;
    }

    static AminoSeqRecord *create(const QString &name, const QString &description, AminoAnonSeq *aminoAnonSeq)
    {
        return new AminoSeqRecord(decrementor_.nextValue(), name, description, aminoAnonSeq);
    }

private:
    static DecrementNumberGenerator decrementor_;

    AminoAnonSeq *aminoAnonSeq_;
};

#endif // AMINOSEQRECORD_H
