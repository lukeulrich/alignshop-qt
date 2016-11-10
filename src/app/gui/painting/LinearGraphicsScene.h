/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LINEARGRAPHICSSCENE_H
#define LINEARGRAPHICSSCENE_H

#include <QtGui/QGraphicsScene>
#include "../../core/global.h"

class AbstractLinearItem;
class SeqBioStringItem;

/**
  * LinearGraphicsScene extends QGraphicsScene by providing a common pixels per unit for AbstractLinearItems.
  *
  * AddLinearItem() will attach signals to update its pixels per unit when added to the scene. Additionally, it is
  * possible to update the pixels per unit via the scalePixelsPerUnit() method.
  */
class LinearGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    LinearGraphicsScene(QObject *parent = nullptr);                 //!< Simple constructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void addLinearItem(AbstractLinearItem *abstractLinearItem);     //!< Adds abstractLinearItem to the scene
    void addSeqBioStringItem(SeqBioStringItem *seqBioStringItem);   //!< Adds seqBioStringItem to the scene
    double maxPixelsPerUnit() const;                                //!< Returns the maximum pixels per unit
    QRectF paddedItemsBoundingRect(const double padding) const;     //!< Returns a bounding rect of all items with padding on the sides
    //!< Returns a bounding rect of all items with verticalPadding above and below and horizontalPadding left and right
    QRectF paddedItemsBoundingRect(const double verticalPadding, const double horizontalPadding) const;
    double pixelsPerUnit() const;                                   //!< Returns the pixels per unit
    double scaleFactor() const;                                     //!< Returns the current scale factor


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setMaxPixelsPerUnit(const double maxPixelsPerUnit);        //!< Sets the maximum pixels per unit to maxPixelsPerUnit
    void setPixelsPerUnit(const double newPixelsPerUnit);           //!< Sets the pixels per unit to newPixelsPerUnit
    void setScaleFactor(const double newScaleFactor);               //!< Sets the scale factor to newScaleFactor
    void scalePixelsPerUnit(const int direction);                   //!< Scales the pixels per unit by the set scale factor relative to direction


Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void pixelsPerUnitChanged(double pixelsPerUnit);


private:
    // ------------------------------------------------------------------------------------------------
    // Private
    double pixelsPerUnit_;      //!< Pixels per unit for all AbstractLinearItems within this scene
    double scaleFactor_;        //!< Amount that the pixel per unit should be changed
    double maxPixelsPerUnit_;   //!< Maximum allowed pixels per unit
};

#endif // LINEARGRAPHICSSCENE_H
