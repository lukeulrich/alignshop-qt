/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEQBIOSTRINGITEM_H
#define SEQBIOSTRINGITEM_H

#include <QtGui/QGraphicsItemGroup>

#include "../../../core/global.h"

class QFont;

class AbstractSeqItem;
class BioStringItem;

/**
  * SeqBioStringItem composes an AbstractSeqItem and a BioStringItem representation of its complete source sequence.
  *
  * The BioStringItem may be enabled / disabled which will make it visible or hidden assuming that a sufficient pixels
  * per unit. Even if enabled, the BioStringItem will not be displayed if the pixels per unit is less than
  * bioStringVisiblePPUThreshold_. To alert others to changes in height, interested components may connect to the
  * heightChanged signal.
  *
  * For aesthetic purposes, the user may specify an amount of vertical space to use between the AbstractSeqItem and its
  * BioStringItem. This only applies if the BioStringItem is visible.
  */
class SeqBioStringItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    SeqBioStringItem(AbstractSeqItem *abstractSeqItem, const QFont &font, QGraphicsItem *parentItem = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    AbstractSeqItem *abstractSeqItem() const;
    bool bioStringEnabled() const;
    BioStringItem *bioStringItem() const;
    double height() const;
    void setBioStringEnabled(bool enabled = true);
    void setVerticalSpacing(const double newVerticalSpacing);
    double verticalSpacing() const;
    double width() const;


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setPixelsPerUnit(const double newPixelsPerUnit);


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void heightChanged(double height);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onSeqItemPixelsPerUnitChanged(const double newPixelsPerUnit);


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    AbstractSeqItem *abstractSeqItem_;
    BioStringItem *bioStringItem_;
    double verticalSpacing_;
    bool bioStringEnabled_;
    double bioStringVisiblePPUThreshold_;
};

#endif // SEQBIOSTRINGITEM_H
