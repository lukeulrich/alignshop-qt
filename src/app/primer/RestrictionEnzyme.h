/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RESTRICTIONENZYME_H
#define RESTRICTIONENZYME_H

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include "../core/BioString.h"
#include "../core/macros.h"

static const char kCutDelimiter = ',';

/**
  * RestrictionEnzyme captures the basic details of a restriction enzyme.
  *
  * Except for the name it is a value object and all its relevant properties must be initialized at construction.
  * Because restriction enzymes may cut at arbitrary sites relative to the recognition site, there is little data
  * verification in place. Verification includes the following: grammar is DNA, ungapped recognition site, that all cuts
  * are non-zero, and that the recognition site is non-empty if at least one cut is provided.
  *
  * Cut positions have the following properties:
  * 1) 1-based coordinates
  * 2) Relative to the 5' direction on the sense / forward / + strand
  * 3) Cut position is to the right of the value
  * 4) Negative numbers are permitted and indicate a position prior to the first base on the 5' direction
  * 5) Zero values are not allowed and are undefined
  *
  * To even potentially have a blunt or sticky end there must be at least one cut on both strands.
  *
  * It is the user's responsibility to make sure no duplicate cut positions are provided.
  */
class RestrictionEnzyme
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    RestrictionEnzyme()                                         //!< Construct an empty restriction enzyme
        : recognitionSite_(eDnaGrammar)
    {
    }

    //! Construct a restriction enzyme with name, recognitionSite, forwardCuts, and reverseCuts
    RestrictionEnzyme(const QString &name, const BioString &recognitionSite, const QVector<int> &forwardCuts, const QVector<int> &reverseCuts)
        : name_(name),
          recognitionSite_(recognitionSite),
          forwardCuts_(forwardCuts),
          reverseCuts_(reverseCuts)
    {
        common_constructor();
    }

    //! Construct a restriction enzyme with name, recognitionSite, forwardCuts, and reverseCuts
    RestrictionEnzyme(const QString &name, const char *recognitionSite, const QVector<int> &forwardCuts, const QVector<int> &reverseCuts)
        : name_(name),
          recognitionSite_(BioString(recognitionSite, eDnaGrammar)),
          forwardCuts_(forwardCuts),
          reverseCuts_(reverseCuts)
    {
        common_constructor();
    }


    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const RestrictionEnzyme &other) const       //!< Returns true if other is equivalent to this instance; false otherwise
    {
        return name_ == other.name_ &&
               recognitionSite_ == other.recognitionSite_ &&
               forwardCuts_ == other.forwardCuts_ &&
               reverseCuts_ == other.reverseCuts_;
    }

    bool operator!=(const RestrictionEnzyme &other) const       //!< Returns true if other is not equivalent to this instance; false otherwise
    {
        return !operator==(other);
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool cutsOnlyOneStrand() const                              //!< Returns true if only one strand is cut; false otherwise
    {
        return (forwardCuts_.size() == 0 && reverseCuts_.size() > 0) ||
                (forwardCuts_.size() > 0 && reverseCuts().size() == 0);
    }

    QVector<int> forwardCuts() const                            //!< Returns all forward cuts
    {
        return forwardCuts_;
    }

    bool isBlunt() const                                        //!< Returns true if there are is an even number of cuts and they produce a blunt end after cleavage; false otherwise
    {
        return forwardCuts_.size() > 0 && forwardCuts_ == reverseCuts_;
    }

    bool isEmpty() const                                        //!< Returns true if no recognition site has been defined; false otherwise
    {
        return recognitionSite_.isEmpty();
    }

    bool isSticky() const                                       //!< Returns true if cleavage produces sticky ends; false otherwise
    {
        return forwardCuts_.size() > 0 &&
               reverseCuts_.size() > 0 &&
               forwardCuts_ != reverseCuts_;
    }

    QString name() const                                        //!< Returns the enzyme name
    {
        return name_;
    }

    int numCuts() const                                         //!< Returns the sum of the forward and reverse cuts
    {
        return forwardCuts_.size() + reverseCuts_.size();
    }

    BioString recognitionSite() const                           //!< Returns the recognition site
    {
        return recognitionSite_;
    }

    QVector<int> reverseCuts() const                            //!< Returns all reverse cuts
    {
        return reverseCuts_;
    }

    void setName(const QString &newName)                        //!< Sets the name to newName
    {
        name_ = newName;
    }


    // ------------------------------------------------------------------------------------------------
    // Public static methods
    static QVector<int> deserializeCutString(const QString &cutString);
    static QString serializeCuts(const QVector<int> &cuts);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void common_constructor()                                   //!< Performs common constructor instructions
    {
        ASSERT(recognitionSite_.grammar() == eDnaGrammar);
        ASSERT(!recognitionSite_.isEmpty() || numCuts() == 0);
        ASSERT(!recognitionSite_.hasGaps());
        ASSERT(!forwardCuts_.contains(0));
        ASSERT(!reverseCuts_.contains(0));

        qSort(forwardCuts_);
        qSort(reverseCuts_);
    }


    // ------------------------------------------------------------------------------------------------
    // Private members
    QString name_;
    BioString recognitionSite_;

    QVector<int> forwardCuts_;
    QVector<int> reverseCuts_;
};

Q_DECLARE_METATYPE(RestrictionEnzyme)

// Tell Qt how to handle this data structure so it can optimize it's container usage and copying performance
Q_DECLARE_TYPEINFO(RestrictionEnzyme, Q_MOVABLE_TYPE);


// ------------------------------------------------------------------------------------------------
// Static inline methods
inline
QVector<int> RestrictionEnzyme::deserializeCutString(const QString &cutString)
{
    QStringList splitString = cutString.split(kCutDelimiter, QString::SkipEmptyParts);
    QVector<int> cuts;
    foreach (const QString &cutAsString, splitString)
    {
        bool ok = false;
        int cut = cutAsString.toInt(&ok);
        if (ok)
            cuts << cut;
    }

    return cuts;
}

inline
QString RestrictionEnzyme::serializeCuts(const QVector<int> &cuts)
{
    QStringList cutStringList;
    foreach (const int cut, cuts)
        cutStringList << QString::number(cut);

    char joinStr[2] = {kCutDelimiter};
    return cutStringList.join(joinStr);
}

#endif // RESTRICTIONENZYME_H
