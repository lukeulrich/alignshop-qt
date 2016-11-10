/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include "DnaSequence.h"
#include "StrictDnaString.h"

using namespace PrimerDesign;

/**
  * Sets the name to a default string.
  */
DnaSequence::DnaSequence()
    : name_("Unknown Sequence"),
      sequence_(StrictDnaString(StrictDnaString::isNotEmpty))
{
}

/**
  * @return QString
  */
QString DnaSequence::name() const
{
    return name_;
}

/**
  * Name cannot be blank.
  *
  * @param name [QString]
  * @return bool
  */
bool DnaSequence::setName(const QString &name)
{
    QString trimmed = name.trimmed();
    if (trimmed.length() > 0)
    {
        name_ = trimmed;
        return true;
    }

    return false;
}

/**
  * @return QString
  */
QString DnaSequence::notes() const
{
    return notes_;
}

/**
  * @param notes [QString]
  * @return bool
  */
bool DnaSequence::setNotes(const QString &notes)
{
    notes_ = notes;
    return true;
}

QList<PrimerPair> DnaSequence::primers() const
{
    return primers_;
}

/**
  * @return QString
  */
QString DnaSequence::sequence() const
{
    return sequence_.value();
}

/**
  * @param sequence [QString]
  * @return bool
  */
bool DnaSequence::setSequence(const QString &sequence)
{
    return sequence_.setValue(sequence);
}
