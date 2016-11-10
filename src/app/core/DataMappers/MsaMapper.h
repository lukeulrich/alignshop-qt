/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAMAPPER_H
#define MSAMAPPER_H

#include <QtCore/QScopedPointer>

#include "GenericEntityMapper.h"
#include "IMsaMapper.h"
#include "../DataSources/IAdocSource.h"
#include "../Repositories/GenericRepository.h"
#include "../Entities/AbstractMsa.h"    // For MsaMembersPod, see loadAlignmentStep
#include "../ObservableMsa.h"
#include "../Subseq.h"
#include "../global.h"
#include "../macros.h"

// Disable the virtual inheritance warning about dominant functions
#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

template<typename T, typename PodT, typename SeqT>
class MsaMapper : public GenericEntityMapper<T, PodT>,
                  public IMsaMapper<T>
{
public:
    MsaMapper(IAdocSource *adocSource, GenericRepository<SeqT> *seqRepository)
        : GenericEntityMapper<T, PodT>(adocSource), seqRepository_(seqRepository)
    {
        ASSERT(seqRepository_ != nullptr);
    }

    virtual int beginLoadAlignment(T *msaEntity)
    {
        ASSERT(msaEntity != nullptr);

        if (loadRequest_.msa_ != nullptr)
            clearLoadData();

        loadRequest_.msaEntity_ = msaEntity;
        loadRequest_.msa_ = new ObservableMsa(msaEntity->grammar());
        loadRequest_.memberCount_ = GenericEntityMapper<T, PodT>::adocSource_->crud(msaEntity)->countMembers(msaEntity->id());

        // Return total number of sequences to load
        return loadRequest_.memberCount_;
    }

    virtual void cancelLoadAlignment()
    {
        clearLoadData();
    }

    virtual void endLoadAlignment()
    {
        ASSERT(loadRequest_.msaEntity_ != nullptr);
        ASSERT(loadRequest_.msa_ != nullptr);
        ASSERT(loadRequest_.msa_->subseqCount() == loadRequest_.memberCount_);

        // Update the entities msa pointer to the one we have been loading
        loadRequest_.msaEntity_->setMsa(loadRequest_.msa_);

        // Clear the load request members
        loadRequest_.msaEntity_ = nullptr;
        loadRequest_.msa_ = nullptr;
        loadRequest_.memberCount_ = 0;
    }

    virtual int loadAlignmentStep(int stepsToTake)
    {
        // Must either take a negative number of steps which indicates to load the entire alignment in one go
        // or a positive number of steps which indicates the number of sequences to load in this step
        ASSERT(stepsToTake != 0);
        ASSERT(loadRequest_.msa_ != nullptr);

        // TODO: Wrap in a try/catch block
        int currentStep = loadRequest_.msa_->subseqCount();
        MsaMembersPod pod = GenericEntityMapper<T, PodT>::adocSource_->crud(static_cast<T *>(nullptr))->readMsaMembers(
                    loadRequest_.msaEntity_->id(),
                    currentStep,
                    stepsToTake);

        QVector<typename SeqT::SPtr> seqEntities = seqRepository_->find(pod.seqIds_);
        ASSERT(seqEntities.size() == pod.gappedSequences_.size());

        for (int i=0, z=seqEntities.size(); i<z; ++i)
        {
            ASSERT(seqEntities.at(i) != nullptr);
            QScopedPointer<Subseq> subseq(new Subseq(seqEntities.at(i)->abstractAnonSeq()->seq_));
            if (!subseq->setBioString(pod.gappedSequences_.at(i)) ||
                !loadRequest_.msa_->append(subseq.data()))
            {
                clearLoadData();        // De-allocates the msa and its subseqs; unfinds the subseq entities

                throw 0;
            }

            // Associate the entity with the subseq
            subseq->seqEntity_ = seqEntities.at(i);

            // To prevent the scoped pointer from auto-deleting the subseq when it goes out of scope
            subseq.take();
        }

        return currentStep + seqEntities.size();
    }

    virtual void unloadAlignment(T *msaEntity)
    {
        ASSERT(loadRequest_.isNull());

        ObservableMsa *msa = msaEntity->msa();
        if (msa != nullptr)
            msaEntity->setMsa(nullptr);     // This method de-allocates the Msa
    }

    // Erases associated Seq entities from the repository (could/should be a hard and immediate erase), but currently
    // only a soft erase. Of course, if they are hard-erased, the pointer members should be set to null.
    //
    // !! Note: The Subseq * will still have non-null and valid Seq entity pointers
    virtual bool expungeDeadSubseqs(T *msaEntity) const
    {
        ASSERT(msaEntity != nullptr);
        seqRepository_->erase(::seqEntityIdVector<SeqT>(msaEntity->deadSubseqs()));
        return true;
    }

    // 1) De-allocates the memory occupied by dead Subseq *
    // 2) Unfinds the Seq entities from the repository
    virtual void unloadDeadSubseqs(T *msaEntity) const
    {
        ASSERT(msaEntity != nullptr);

        msaEntity->clearDeadSubseqs();
    }

    // Override the save method so that all modifications to the Seq entities are saved as well
    virtual bool save(const QVector<T *> &msaEntities) const
    {
        QVector<typename SeqT::SPtr> seqEntities;
        foreach (T *msaEntity, msaEntities)
        {
            ObservableMsa *msa = msaEntity->msa();
            if (msa != nullptr)
            {
                // Save all the associated seq entities
                for (int i=0, z=msa->rowCount(); i<z; ++i)
                {
                    // ISSUE? Should we do a dynamic cast here instead of a static_cast? Or possibly move this function
                    // into a specific MsaMapper (e.g. AminoMsaMapper), which has a concrete defined SeqT
                    seqEntities << boost::shared_static_cast<SeqT>(msa->at(i+1)->seqEntity_);
                }

                if (!this->expungeDeadSubseqs(msaEntity))
                    return false;

                this->unloadDeadSubseqs(msaEntity);
            }
        }

        return seqRepository_->save(seqEntities) && GenericEntityMapper<T, PodT>::save(msaEntities);
    }

private:
    void clearLoadData()
    {
        if (loadRequest_.msa_ == nullptr)
            return;

        loadRequest_.reset();
    }

    GenericRepository<SeqT> *seqRepository_;

    struct LoadAlignmentRequest
    {
        ObservableMsa *msa_;
        T *msaEntity_;
        int memberCount_;

        LoadAlignmentRequest() : msa_(nullptr), msaEntity_(nullptr), memberCount_(0)
        {
        }

        bool isNull() const
        {
            return msa_ == nullptr;
        }

        void reset()
        {
            delete msa_;
            msa_ = nullptr;

            msaEntity_ = nullptr;
            memberCount_ = 0;
        }
    };
    LoadAlignmentRequest loadRequest_;
};

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#endif // MSAMAPPER_H
