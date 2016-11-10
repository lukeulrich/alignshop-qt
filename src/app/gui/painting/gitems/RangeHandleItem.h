/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RANGEHANDLEITEM_H
#define RANGEHANDLEITEM_H

#include <QtGui/QGraphicsEllipseItem>

class QGraphicsLineItem;
class QGraphicsRectItem;

class AbstractRangeItem;

/**
  * RangeHandleItem provides a circular handle and stem (line) to indicate the start or stop position of a range item
  * and provides a facile means for interactively changing the range positions.
  */
class RangeHandleItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT

public:
    enum HandleType
    {
        eStartHandle = 0,
        eStopHandle
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    RangeHandleItem(AbstractRangeItem *targetRangeItem, HandleType handleType, double diameter);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    double bottomStemHeight() const;                            //!< Returns the bottom stem height
    double coreStemHeight() const;                              //!< Returns the core stem height
    double diameter() const;                                    //!< Returns the diameter of the ellipse handle
    double fadeRectOpacity() const;                             //!< Returns the opacity of the fade rect (default .75)
    double height() const;                                      //!< Returns diameter + total stem height
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setTargetRangeItem(AbstractRangeItem *targetRangeItem);//!< Sets the target range item to targetRangeItem
    double stemHeight() const;                                  //!< Returns the total stem height
    double topStemHeight() const;                               //!< Returns the top stem height


public Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Public slots
    void setBottomStemHeight(double newBottomStemHeight);       //!< Sets the bottom stem height to newBottomStemHeight
    void setCoreStemHeight(double newCoreStemHeight);           //!< Sets the core stem height to newCoreStemHeight
    void setFadeRectOpacity(double newFadeRectOpacity);         //!< Sets the fade rect opacity to newFadeRectOpacity
    void setHandleIndex(const int newHandleIndex);              //!< Sets the unit modeled by this handle item to newHandleIndex if possible
    void setTopStemHeight(double newTopStemHeight);             //!< Sets the top stem height to newTopStemHeight


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);      //!< Called when the mouse hovers over the ellipse
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);      //!< Called when the mouse leaves the ellipse
    //! Overrided method to constrain position changes to the horizontal axis and discrete range positions
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);      //!< Called when the mouse button has been pressed on the ellipse
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);    //!< Called when the mouse button has been released on the ellipse


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onTargetPixelsPerUnitChanged();                        //!< Updates the handle position


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    int clampIndex(const int index) const;                      //!< Helper method that clamps index into a valid range according to its handle type and target range item
    double handlePosition() const;                              //!< Helper method that returns the pixel position according to this handle type and target range item
    double handlePosition(const int index) const;               //!< Returns the pixel position for this handle type at index
    bool setTargetIndex(const int index) const;                 //!< Merely attempts to update the target range item index represnted by this handle to index and returns true on success, false otherwise
    void updateFadeRect();                                      //!< Updates the geometry of the fade rect relative to the handle type
    void updateStemLine();                                      //!< Helper method that resets the stemline endpoint based on the current stemHeight()

    // ------------------------------------------------------------------------------------------------
    // Private members
    AbstractRangeItem *targetRangeItem_;
    HandleType handleType_;
    double diameter_;
    double radius_;
    double topStemHeight_;
    double coreStemHeight_;
    double bottomStemHeight_;
    int handleIndex_;                       //!< Integer intex of handle (start for eStartHandle, stop for eStopHandle)
    int oldHandleIndex_;                    //!< In the event, that setting the handle index on a mouse release event does not work

    QGraphicsLineItem *stemItem_;
    QGraphicsRectItem *fadeRectItem_;
};

#endif // RANGEHANDLEITEM_H
