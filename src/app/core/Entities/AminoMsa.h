/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef AMINOMSA_H
#define AMINOMSA_H

#include "AbstractMsa.h"

struct AminoMsaPod;

class AminoMsa : public AbstractMsa
{
public:
    typedef boost::shared_ptr<AminoMsa> SPtr;
    static const int kType;

    AminoMsa(int id, const QString &name, const QString &description, const QString &notes);
    AminoMsa(const AminoMsaPod &abstractMsaPod);

    Grammar grammar() const;
    int type() const;

    static AminoMsa *createEntity(const QString &name,
                                  const QString &description = QString(),
                                  const QString &notes = QString());
};
Q_DECLARE_TYPEINFO(AminoMsa, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<AminoMsa> AminoMsaSPtr;

struct AminoMsaPod : public AbstractMsaPod
{
    AminoMsaPod(int id = 0) : AbstractMsaPod(id)
    {
    }
};
Q_DECLARE_TYPEINFO(AminoMsaPod, Q_MOVABLE_TYPE);

#endif // AMINOMSA_H
