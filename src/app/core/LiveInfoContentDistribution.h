/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LIVEINFOCONTENTDISTRIBUTION_H
#define LIVEINFOCONTENTDISTRIBUTION_H

#include <QtCore/QObject>

#include "InfoContentDistribution.h"
#include "global.h"

class AbstractLiveCharCountDistribution;

class LiveInfoContentDistribution : public QObject,
                                    public InfoContentDistribution
{
    Q_OBJECT

public:
    LiveInfoContentDistribution(AbstractLiveCharCountDistribution *liveCharCountDistribution,
                                int possibleLetters,
                                bool smallSampleErrorCorrection = true,
                                QObject *parent = nullptr);

    AbstractLiveCharCountDistribution *liveCharCountDistribution() const;       //!< Returns the live char count distribution being observed
    void setSmallSampleErrorCorrection(bool enabled);                           //!< Enables or disables small sample error correction

Q_SIGNALS:
    void columnsInserted(const ClosedIntRange &range);                          //!< Emitted after the columns in range (inclusive, 1-based) have been inserted
    void columnsRemoved(const ClosedIntRange &range);                           //!< Emitted after the columns in range (inclusive, 1-based) have been removed)
    void dataChanged(const ClosedIntRange &range);                              //!< Emitted when any of the character counts in range (inclusive, 1-baed) have changed

private Q_SLOTS:
    void onSourceColumnsInserted(const ClosedIntRange &range);                  //!< Emitted after the columns in range (inclusive, 1-based) have been inserted
    void onSourceColumnsRemoved(const ClosedIntRange &range);                   //!< Emitted after the columns in range (inclusive, 1-based) have been removed)
    void onSourceDataChanged(const ClosedIntRange &range);                      //!< Emitted when any of the character counts in range (inclusive, 1-baed) have changed

private:
    AbstractLiveCharCountDistribution *liveCharCountDistribution_;
};

#endif // LIVEINFOCONTENTDISTRIBUTION_H
