/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef DNAMSA_H
#define DNAMSA_H

#include "AbstractMsa.h"

struct DnaMsaPod;

class DnaMsa : public AbstractMsa
{
public:
    typedef boost::shared_ptr<DnaMsa> SPtr;
    static const int kType;

    DnaMsa(int id, const QString &name, const QString &description, const QString &notes);
    DnaMsa(const DnaMsaPod &abstractMsaPod);

    Grammar grammar() const;
    int type() const;

    static DnaMsa *createEntity(const QString &name,
                                const QString &description = QString(),
                                const QString &notes = QString());
};
Q_DECLARE_TYPEINFO(DnaMsa, Q_MOVABLE_TYPE);

typedef boost::shared_ptr<DnaMsa> DnaMsaSPtr;

struct DnaMsaPod : public AbstractMsaPod
{
    DnaMsaPod(int id = 0) : AbstractMsaPod(id)
    {
    }
};
Q_DECLARE_TYPEINFO(DnaMsaPod, Q_MOVABLE_TYPE);

#endif // DNAMSA_H
