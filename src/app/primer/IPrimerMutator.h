/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IPRIMERMUTATOR_H
#define IPRIMERMUTATOR_H

#include <QtCore/QObject>

class QString;

class Primer;

/**
  * IPrimerMutator defines the common interface methods for setting the primer pair data members.
  */
class IPrimerMutator : public QObject
{
    Q_OBJECT

public:
    IPrimerMutator(QObject *parent = 0);
    virtual ~IPrimerMutator();

    virtual bool setPrimerName(Primer &primer, const QString &newName) = 0;


Q_SIGNALS:
    void primerNameChanged(int primerId);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
inline
IPrimerMutator::IPrimerMutator(QObject *parent)
    : QObject(parent)
{
}

inline
IPrimerMutator::~IPrimerMutator()
{
}

#endif // IPRIMERPAIRMUTATOR_H
