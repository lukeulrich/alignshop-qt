/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AminoSeqRecordMapper.h"
#include "../DataSources/IAdocSource.h"
#include "../Entities/AminoSeqRecord.h"
#include "../PODs/AminoSeqRecordPod.h"
#include "../Repositories/AminoAnonSeqRepository.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocSource [IAdocSource *]
  * @param aminoAnonSeqRepository [AminoAnonSeqRepository *]
  */
AminoSeqRecordMapper::AminoSeqRecordMapper(IAdocSource *adocSource, AminoAnonSeqRepository *aminoAnonSeqRepository) :
    AbstractDataMapper<AminoSeqRecord>(adocSource), aminoAnonSeqRepository_(aminoAnonSeqRepository)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param ids [const QVector<int> &]
  * @returns QVector<AminoSeqRecord *>
  */
QVector<AminoSeqRecord *> AminoSeqRecordMapper::find(const QVector<int> &ids) const
{
    QVector<AminoSeqRecord *> aminoSeqRecords(ids.size(), nullptr);
    try
    {
        // Read the pods
        QVector<AminoSeqRecordPod> pods = adocSource_->readAminoSeqRecords(ids);

        // Fetch the amino anon seqs
        QVector<int> aminoAnonSeqIds;
        aminoAnonSeqIds.reserve(ids.size());
        foreach (const AminoSeqRecordPod &pod, pods)
            aminoAnonSeqIds << pod.astringId_;
        QVector<AminoAnonSeq *> aminoAnonSeqs = aminoAnonSeqRepository_->find(aminoAnonSeqIds);
        ASSERT(pods.size() == aminoAnonSeqs.size());

        // Build the AminoSeqRecords
        for (int i=0, z=pods.size(); i<z; ++i)
        {
            const AminoSeqRecordPod &pod = pods.at(i);
            ASSERT(pod.id() != 0);
            aminoSeqRecords[i] = new AminoSeqRecord(pod.id_, pod.name_, pod.description_, aminoAnonSeqs.at(i));
        }
    }
    catch(...)
    {
        // Free any allocated memory
        for (int i=0, z=aminoSeqRecords.size(); i<z; ++i)
        {
            const AminoSeqRecord *record = aminoSeqRecords.at(i);
            if (record == nullptr)
                continue;

            aminoAnonSeqRepository_->unfindOne(record->anonSeq());
            delete record;
            record = nullptr;

            aminoSeqRecords[i] = nullptr;
        }
    }

    return aminoSeqRecords;
}

/**
  * @param aminoSeqRecords [const QVector<AminoSeqRecord *> &]
  * @returns bool
  */
bool AminoSeqRecordMapper::save(const QVector<AminoSeqRecord *> &aminoSeqRecords) const
{
    try
    {
        adocSource_->begin();
        QVector<AminoSeqRecordPod> updates;
        foreach (AminoSeqRecord *record, aminoSeqRecords)
        {
            ASSERT(record);
            ASSERT(record->id() != 0);

            if (record->isNew())
            {
                AminoSeqRecordPod pod(record->id(), record->anonSeq()->id(), 0, 0, record->name_, record->description_);
                adocSource_->insertAminoSeqRecord(pod);
                record->setId(pod.id_);
            }
            else
            {
                updates << AminoSeqRecordPod(record->id(), record->anonSeq()->id(), 0, 0, record->name_, record->description_);
            }
        }
        adocSource_->saveAminoSeqRecords(updates);
        adocSource_->end();
    }
    catch(...)
    {
        return false;
    }

    return true;
}

/**
  * @param aminoSeqRecords [const QVector<AminoSeqRecord *> &]
  */
void AminoSeqRecordMapper::teardown(const QVector<AminoSeqRecord *> &aminoSeqRecords) const
{
    foreach (const AminoSeqRecord *record, aminoSeqRecords)
        aminoAnonSeqRepository_->unfindOne(record->anonSeq());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param ids [const QVector<int> &]
  */
void AminoSeqRecordMapper::erase(const QVector<int> &ids) const
{
    adocSource_->eraseAminoSeqRecords(ids);
}
