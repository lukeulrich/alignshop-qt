/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IDOMAIN_H
#define IDOMAIN_H

#include <QtCore/QString>

#include "IFeature.h"

class IDomain : public IFeature
{
public:
    IDomain(ClosedIntRange location, const QString &name) : IFeature(location), name_(name)
    {
    }

    const QString name_;
};

#endif // IDOMAIN_H
