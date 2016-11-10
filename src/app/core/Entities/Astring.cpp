/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Astring.h"
#include "EntityFlags.h"

const int Astring::kType = eAstringEntity;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param id [int]
  * @param seq [const Seq &]
  */
Astring::Astring(int id, const Seq &seq)
    : AbstractAnonSeq(id, seq)
{
    ASSERT(seq.grammar() == eAminoGrammar);
}

/**
  * @param astringPod [const AStringPod &]
  */
Astring::Astring(const AstringPod &astringPod)
    : AbstractAnonSeq(astringPod.id_, astringPod.seq_),
      q3_(astringPod.q3_)
{
    ASSERT(astringPod.seq_.grammar() == eAminoGrammar);
    setCoils(astringPod.coils_);
    setSegs(astringPod.segs_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QVector<Coil>
  */
QVector<Coil> Astring::coils() const
{
    return coils_;
}

/**
  * @param coil [Coil]
  * @returns bool
  */
bool Astring::addCoil(Coil coil)
{
    ASSERT(coil.end() <= seq_.length());

    // Only add if unique
    if (coils_.indexOf(coil) != -1)
        return false;

    coil.clearId();
    coils_ << coil;

    setDirty(Ag::eCoilsFlag, true);

    return true;
}

/**
  * @param i [int]
  */
void Astring::removeCoil(int i)
{
    coils_.remove(i);

    setDirty(Ag::eCoilsFlag, true);
}

/**
  * @param coils [const QVector<Coil> &]
  */
void Astring::setCoils(const QVector<Coil> &coils)
{
    coils_.clear();
    foreach (const Coil &coil, coils)
        addCoil(coil);

    setDirty(Ag::eCoilsFlag, true);
}

/**
  * @returns QVector<Seg>
  */
QVector<Seg> Astring::segs() const
{
    return segs_;
}

/**
  * @param seg [Seg]
  * @returns bool
  */
bool Astring::addSeg(Seg seg)
{
    ASSERT(seg.end() <= seq_.length());

    // Only add if unique
    if (segs_.indexOf(seg) != -1)
        return false;

    seg.clearId();
    segs_ << seg;

    setDirty(Ag::eSegsFlag, true);

    return true;
}

/**
  * @param i [int]
  */
void Astring::removeSeg(int i)
{
    segs_.remove(i);

    setDirty(Ag::eCoilsFlag, true);
}

/**
  * @param segs [const QVector<Seg> &]
  */
void Astring::setSegs(const QVector<Seg> &segs)
{
    segs_.clear();

    foreach (const Seg &seg, segs)
        addSeg(seg);

    setDirty(Ag::eSegsFlag, true);
}

/**
  * @returns Q3Prediction
  */
Q3Prediction Astring::q3() const
{
    return q3_;
}

/**
  * @param q3 [const Q3Prediction &]
  */
void Astring::setQ3(const Q3Prediction &q3)
{
    using namespace Ag;

    q3_ = q3;
    setDirty(eQ3Flag, true);
}

/**
  * @returns int
  */
int Astring::type() const
{
    return eAstringEntity;
}

/**
  * Factory method for producing new Astring entities.
  *
  * @param seq [const Seq &]
  * @returns Astring *
  */
Astring *Astring::createEntity(const Seq &seq)
{
    ASSERT(seq.grammar() == eAminoGrammar);
    return new Astring(::newEntityId<Astring>(), seq);
}
