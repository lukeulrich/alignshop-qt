/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef INFOCONTENTDISTRIBUTION_H
#define INFOCONTENTDISTRIBUTION_H

#include "CharCountDistribution.h"
#include "PODs/InfoUnit.h"
#include "macros.h"
#include "types.h"

/**
  */
class InfoContentDistribution
{
public:
    InfoContentDistribution(const CharCountDistribution &charCountDistribution,
                            int possibleLetters,
                            bool smallSampleErrorCorrection = true);

    VectorVectorInfoUnit infoContent() const;               //!< Returns the raw information content data with each column of InfoUnit's sorted by decreasing info
    double columnInfo(const int column) const;              //!< Returns the total content for column
    int length() const;                                     //!< Returns the length of this distribution
    double maxInfo() const;                                 //!< Returns the maximum information possible for a column
    int possibleLetters() const;                            //!< Returns the number of possible letters
    bool smallSampleErrorCorrection() const;                //!< Returns true if small sample error correction is enabled; false otherwise
    double totalInfo() const;                               //!< Returns the total information for the entire char count distribution

protected:
    //! Determines the information content of charCountDistribution between range and returns a VectorVectorInfoUnit
    VectorVectorInfoUnit computeInfoContent(const CharCountDistribution &charCountDistribution,
                                            const ClosedIntRange &range = ClosedIntRange()) const;

    VectorVectorInfoUnit infoContent_;          // The raw information content in doubles
    bool smallSampleErrorCorrection_;           // Flag denoting whether to use small error correction

private:
    int possibleLetters_;                       // Total number of possible letters in this distribution

    double maxInfo_;
    double smallSampleErrorFactor_;             //!< Small sample correction error factor
};

#endif // INFOCONTENTDISTRIBUTION_H
