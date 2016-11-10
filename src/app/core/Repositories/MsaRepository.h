/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAREPOSITORY_H
#define MSAREPOSITORY_H

#include "GenericRepository.h"
#include "IMsaRepository.h"
#include "../DataMappers/IMsaMapper.h"
#include "../ObservableMsa.h"
#include "../Subseq.h"
#include "../global.h"

/**
  * Really a silly class because it simply acts as a proxy class to the end-user and forwards all requests onto the
  * the corresponding entity mapper.
  */
template<typename T, typename SeqT>
class MsaRepository : public GenericRepository<T>,
                      public IMsaRepository
{
public:
    MsaRepository(IMsaMapper<T> *msaMapper, GenericRepository<SeqT> *seqRepository)
        : GenericRepository<T>(msaMapper),
          msaMapper_(msaMapper),
          seqRepository_(seqRepository)
    {
        ASSERT(msaMapper_ != nullptr);
        ASSERT(seqRepository_ != nullptr);
    }

    virtual int beginLoadAlignment(const typename T::SPtr &msaEntity)
    {
        return msaMapper_->beginLoadAlignment(msaEntity.get());
    }

    virtual int beginLoadAlignment(const AbstractMsaSPtr &msaEntity)
    {
        ASSERT(boost::shared_dynamic_cast<T>(msaEntity));
        return beginLoadAlignment(boost::shared_static_cast<T>(msaEntity));
    }

    virtual void cancelLoadAlignment()
    {
        msaMapper_->cancelLoadAlignment();
    }
    virtual void endLoadAlignment()
    {
        msaMapper_->endLoadAlignment();
    }
    virtual int loadAlignmentStep(int stepsToTake)
    {
        return msaMapper_->loadAlignmentStep(stepsToTake);
    }
    virtual void unloadAlignment(const typename T::SPtr &msaEntity)
    {
        msaMapper_->unloadAlignment(msaEntity.get());
    }
    virtual void unloadAlignment(const AbstractMsaSPtr &msaEntity)
    {
        ASSERT(boost::shared_dynamic_cast<T>(msaEntity));
        unloadAlignment(boost::shared_static_cast<T>(msaEntity));
    }
    virtual bool expungeDeadSubseqs(T *msaEntity)
    {
        return msaMapper_->expungeDeadSubseqs(msaEntity);
    }
    virtual bool expungeDeadSubseqs(const AbstractMsaSPtr &msaEntity)
    {
        ASSERT(boost::shared_dynamic_cast<T>(msaEntity));
        return expungeDeadSubseqs(boost::shared_static_cast<T>(msaEntity));
    }
    virtual void unloadDeadSubseqs(const typename T::SPtr &msaEntity)
    {
        msaMapper_->unloadDeadSubseqs(msaEntity.get());
    }
    virtual void unloadDeadSubseqs(const AbstractMsaSPtr &msaEntity)
    {
        ASSERT(boost::shared_dynamic_cast<T>(msaEntity));
        unloadDeadSubseqs(boost::shared_static_cast<T>(msaEntity));
    }


    // ------------------------------------------------------------------------------------------------
    // When adding a MSA, add its associated subseq entities as well
    bool addGeneric(const IEntitySPtr &entity, bool ignoreNull)
    {
        ASSERT(boost::dynamic_pointer_cast<T>(entity));
        return add(boost::static_pointer_cast<T>(entity), ignoreNull);
    }

    // Important to remember that this method is also called by find and may in turn be added. Because it is desirable
    // when "find"ing a MSA to not immediately load all its subseqs, it is very possible that it has a null msa. On the
    // other hand, if it is being added to the repository, it should have a non-null msa and its corresponding subseqs
    // should be added to the repository as well.
    bool add(const typename T::SPtr &msaEntity, bool ignoreNull)
    {
        // A) Add the subseq-associated entities
        ObservableMsa *msa = msaEntity->msa();
        if (msa != nullptr)
        {
            // Must use 1-based indices!
            for (int i=1, z=msa->subseqCount(); i<=z; ++i)
            {
                ASSERT(msa->at(i)->seqEntity_ != nullptr);

                // ISSUE: What if addGeneric returns false?
                seqRepository_->addGeneric(msa->at(i)->seqEntity_, ignoreNull);
            }
        }

        // B) Add in the actual msa itself
        return GenericRepository<T>::addGeneric(msaEntity, ignoreNull);
    }

    bool addGeneric(const QVector<IEntitySPtr> &entities, bool ignoreNull)
    {
        foreach (const IEntitySPtr &entity, entities)
            if (!addGeneric(entity, ignoreNull))
                return false;

        return true;
    }

    bool addGeneric(const QVector<typename T::SPtr> &entities, bool ignoreNull)
    {
        foreach (const typename T::SPtr &entity, entities)
            if (!add(entity, ignoreNull))
                return false;

        return true;
    }

private:
    // Should be equivalent to the protected AbstractEntityMapper::entityMapper_ instance, but keeping a copy ourselves
    // avoids the need to make a dynamic cast in our methods.
    IMsaMapper<T> *msaMapper_;
    GenericRepository<SeqT> *seqRepository_;
};

#endif // MSAREPOSITORY_H
