/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOSEQRECORDMAPPER_H
#define AMINOSEQRECORDMAPPER_H

#include "AbstractDataMapper.h"

class AminoAnonSeqRepository;
class AminoSeqRecord;

class AminoSeqRecordMapper : public AbstractDataMapper<AminoSeqRecord>
{
public:
    AminoSeqRecordMapper(IAdocSource *adocSource, AminoAnonSeqRepository *aminoAnonSeqRepository);

    QVector<AminoSeqRecord *> find(const QVector<int> &ids) const;
    bool save(const QVector<AminoSeqRecord *> &aminoSeqRecords) const;
    void teardown(const QVector<AminoSeqRecord *> &aminoSeqRecords) const;

private:
    void erase(const QVector<int> &ids) const;

    AminoAnonSeqRepository *aminoAnonSeqRepository_;
};

#endif // AMINOSEQRECORDMAPPER_H
