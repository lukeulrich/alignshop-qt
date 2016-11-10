/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PARSERESULTPOD_H
#define PARSERESULTPOD_H

#include <QtCore/QString>
#include "../enums.h"

struct ParseResultPod
{
    TriBool isParseSuccessful_;
    QString errorMessage_;

    ParseResultPod(TriBool parseSuccessful = eUnknown, const QString &errorMessage = QString())
        : isParseSuccessful_(parseSuccessful), errorMessage_(errorMessage)
    {
    }
};

#endif // PARSERESULTPOD_H
