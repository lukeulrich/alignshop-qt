/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTSEQITEM_H
#define ABSTRACTSEQITEM_H

#include <QtCore/QHash>
#include <QtCore/QMultiHash>

#include "AbstractRangeItem.h"
#include "../../../core/Entities/AbstractSeq.h"
#include "../../../core/global.h"


// ------------------------------------------------------------------------------------------------
// Forward declarations
class QGraphicsRectItem;

class AbstractFeatureItem;
class IColumnAdapter;

/**
  * AbstractSeqItem provides a partial, basic implementation of a sequence item (e.g. AminoSeq).
  *
  * It currently encapsulates the two-way logic for 1) updating the start and stop values of an AbstractSeq and 2)
  * notifying downstream components (e.g. RangeHandleItem) when the start/stop values have been changed externally (e.g.
  * via a relevant IColumnAdapter).
  */
class AbstractSeqItem : public AbstractRangeItem
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    AbstractSeqSPtr abstractSeq() const;            //!< Returns the source abstractSeq
    QGraphicsRectItem *baseLine() const;            //!< Returns the base line graphical item
    double height() const;                          //!< Returns the visual height of this item

    void addFeature(AbstractFeatureItem *featureItem);
    void removeFeature(const int featureType, const int featureId);
    void clearFeatures(const int featureType);
    QVector<AbstractFeatureItem *> findFeatures(const int featureType, const int featureId);


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected constructors
    //! Construct an instance with abstractSeq and parentItem
    AbstractSeqItem(const AbstractSeqSPtr &abstractSeq, QGraphicsItem *parentItem = nullptr);
    //! Advanced constructor that provides for using MVC approach to updating/responding to start/stop changes
    AbstractSeqItem(const AbstractSeqSPtr &abstractSeq, IColumnAdapter *columnAdapter, int startColumn, int stopColumn, QGraphicsItem *parentItem = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void onPixelsPerUnitChanged();                  //!< Called whenever the base pixels per unit changes; updates the baseline width
    virtual bool setStartImpl(int newStart);        //!< If a valid column adapter is present, uses it to update the start to newStart; othewrise, updates the abstractSeq object directly
    virtual bool setStopImpl(int newStop);          //!< If a valid column adapter is present, uses it to update the stop to newStop; othewrise, updates the abstractSeq object directly


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    //!< Called whenever the data has changed for entity in column; used to check for external updates
    void onColumnAdapterDataChanged(const IEntitySPtr &entity, int column);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void constructor_();                    //!< Special method to consolidate multiple common constructor initialization; only to be called from constructors
    void updateFeaturesPPU();         //!< Iterates through all associated features and sets their pixels per unit and positions accordingly


    // ------------------------------------------------------------------------------------------------
    // Private members
    AbstractSeqSPtr abstractSeq_;           //!< Underlying sequence object being modeled
    IColumnAdapter *columnAdapter_;         //!< Optional column adapter
    int startColumn_;                       //!< Column of column adapter that corresponds to the start column
    int stopColumn_;                        //!< Column of column adapter that corresponds to the stop column

    // Graphical parameters
    double baseLineHeight_;                 //!< Height of the baseLine
    QGraphicsRectItem *baseLine_;           //!< Rectangle item representing the baseline

    // { feature type -> feature id -> AbstractFeatureItem * }
    QHash<int, QMultiHash<int, AbstractFeatureItem *> > features_;
};

#endif // ABSTRACTSEQITEM_H
