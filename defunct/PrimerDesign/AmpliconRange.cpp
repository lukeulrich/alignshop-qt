/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "AmpliconRange.h"

using namespace PrimerDesign;

AmpliconRange::AmpliconRange()
{
}

AmpliconRange::AmpliconRange(int minBound, int maxBound)
{
    start_ = minBound;
    stop_ = maxBound;
    minBound_ = minBound;
    maxBound_ = maxBound;
}

int AmpliconRange::getStart()
{
    return start_;
}

int AmpliconRange::getStop()
{
    return stop_;
}

bool AmpliconRange::setRange(int start, int stop)
{
    start_ = std::max(minBound_, std::min(start, stop));
    stop_ = std::min(maxBound_, std::max(start, stop));

    return start_ == start && stop_ == stop;
}
