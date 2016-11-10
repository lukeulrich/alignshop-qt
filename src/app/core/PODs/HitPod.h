/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef HITPOD_H
#define HITPOD_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "HspPod.h"

struct HitPod
{
    QString id_;
    QString accession_;
    QString definition_;
    int length_;

    QVector<HspPod> hsps_;
};


#endif // HITPOD_H
