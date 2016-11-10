/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SignalSequenceParser.h"
#include "../macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param sequenceParser [ISequenceParser *]
  * @param parent [QObject *]
  */
SignalSequenceParser::SignalSequenceParser(ISequenceParser *sequenceParser, QObject *parent)
    : QObject(parent), sequenceParser_(sequenceParser)
{
    ASSERT(sequenceParser != nullptr);
    sequenceParser_->setParent(this);
    connect(sequenceParser_, SIGNAL(progressChanged(int,int)), this, SIGNAL(progressChanged(int,int)));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void SignalSequenceParser::cancel()
{
    sequenceParser_->cancel();
}

/**
  * @param fileName [const QString &]
  */
void SignalSequenceParser::parseFile(const QString &fileName)
{
    try
    {
        emit parseSuccess(sequenceParser_->parseFile(fileName));
    }
    catch (const char *errorMessage)
    {
        emit parseError(errorMessage);
    }
    catch (...)
    {
        // TODO
        emit parseCanceled();
    }

    emit parseOver();
}
