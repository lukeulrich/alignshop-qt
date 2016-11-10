/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef POINTRECTMAPPER_H
#define POINTRECTMAPPER_H

#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include "../../core/util/PosiRect.h"

class AbstractMsaView;

/**
  * PointRectMapperPrivate provides the necessary methods for converting between the msa, canvas, and view spaces
  *
  * It is a friend class to AbstractMsaView and therefore has access to its data members for computing the appropriate
  * values. If abstractMsaView is not defined, then all conversion methods return default constructed values.
  *
  * >> Note the update following this section!!
  * Key to the mapping process is understanding how pixels/units and fractional pixels/units are interconverted. With
  * integer units this is relatively straightforward:
  *
  * Given a block width and height of 10 pixels, then pixels 0-9 belong to the first block, 10-19 belong to the second
  * block and so forth.
  *
  * However, these calculations are complicated with the introduction of fractional units. For instance, given the same
  * block sizes (but in floating point), what block does 10. belong to? A related question, what is the largest value
  * that belongs in the first block? 9.99 or 9.9999? This strictly depends upon the precision of the underlying
  * architecture. Also, it is complicated by the rectangle process. What happens if we request the rectangle for the
  * first block in floating point? Should it be (0., 0.) -> (10., 10.) or (0., 0.) -> (9.9999*, 9.9999*)? Techincally,
  * this should be 10., 10.; however, this point shares blocks.
  *
  * Because of these complications, the following approach will be followed with fractional coordinates. Fractional
  * **points** occuring on the boundary of a given block, will be assigned to the next largest block. In other words, the
  * coordinate (10., 10.) will be mapped to block (1, 1) and not (0., 0.); however, when considering a normalized
  * rectangle, the bottom right most point will be mapped to the next smallest block. For example, the rect [(0., 0.) ->
  * (10., 10.)] will be mapped to block (0, 0). A rectangle must at least partially enclose a block for it to be
  * included in the analysis.
  *
  * For easing the calculation of offsets and avoiding floating point rounding errors, have decided to utilize a
  * resolution-dependent grid. There are four different spaces:
  * Msa: 1-based
  * MsaGrid: a resolution expanded of Msa
  * Canvas: resolution expanded view of pixel data
  * View: pixel based window into canvas
  */
class PointRectMapper
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //!< Construct a mapper object with abstractMsaView and grid resolution (default 64 units per pixel)
    PointRectMapper(AbstractMsaView *abstractMsaView);

    // ------------------------------------------------------------------------------------------------
    // Public methods (general)
    AbstractMsaView *abstractMsaView() const;                           //!< Returns the current msa view being modeled
    void setAbstractMsaView(AbstractMsaView *abstractMsaView);          //!< Sets the abstract msa view to abstractMsaView

    // ------------------------------------------------------------------------------------------------
    // Public methods (conversion)
    // Msa <-> Canvas
    QPoint canvasPointToMsaPoint(const QPointF &canvasPointF) const;    //!< Returns the QPoint in msa space that corresponds to canvasPointF in canvas space
    QPointF canvasPointToMsaPointF(const QPointF &canvasPointF) const;  //!< Returns the QPointF in fractional msa space that corresponds to canvasPointF in canvas space
    PosiRect canvasRectToMsaRect(const Rect &canvasRect) const;         //!< Returns the PosiRect in msa space that corresponds to canvasRect in canvas space
    PosiRect canvasRectFToMsaRect(const QRectF &canvasRectF) const;     //!< Returns the PosiRect in msa space that corresponds to canvasRectF in canvas space
    QPointF msaPointToCanvasPoint(const QPointF &msaPointF) const;      //!< Returns the QPointF in canvas space that corresponds to msaPointF in msa space
    QRectF msaPointToCanvasRect(const QPoint &msaPoint) const;          //!< Returns the QRectF in canvas space that corresponds to msaPoint in msa space
    QRectF msaRectToCanvasRect(const Rect &msaRect) const;              //!< Returns the QRectF in canvas space that corresponds to msaRect in msa space

    // Canvas <-> View
    QPointF canvasPointToViewPoint(const QPointF &canvasPointF) const;  //!< Returns the QPointF in fractional view space that corresponds to canvasPointF in canvas space
    QRectF canvasRectToViewRect(const QRectF &canvasRectF) const;       //!< Returns the QRectF in fractional view space that corresponds to canvasRectF in canvas space
    QPointF viewPointToCanvasPoint(const QPointF &viewPointF) const;    //!< Returns the QPointF in canvas space that corresponds to viewPointF in view space
    QRectF viewRectToCanvasRect(const QRectF &viewRectF) const;         //!< Returns the QRectF in canvas space that corresponds to viewRectF in view space

    // ---------------------
    // Convenience functions
    // View <-> Msa
    QPoint viewPointToMsaPoint(const QPointF &viewPointF) const;        //!< Returns the QPoint in msa space that corresponds to viewPointF in view space
    QPointF viewPointToMsaPointF(const QPointF &viewPointF) const;      //!< Returns the QPointF in msa space that corresponds to viewPointF in view space
    QPointF msaPointToViewPoint(const QPointF &msaPointF) const;        //!< Returns the QPointF in view space that corresponds to msaPointF in msa space
    QRectF msaPointToViewRect(const QPoint &msaPoint) const;            //!< Returns the QRectF in view space that corresponds to msaPoint in msa space
    PosiRect viewRectToMsaRect(const QRectF &viewRectF) const;          //!< Returns the PosiRect in msa space that corresponds to viewRectF in view space
    QRectF msaRectToViewRect(const PosiRect &msaRect) const;            //!< Returns the QRectF in view space that corresponds to msaRect in msa space

private:
    QPointF unboundedCanvasPointToMsaPointF(const QPointF &canvasPointF) const;
    QPoint unboundedCanvasPointToMsaPoint(const QPointF &canvasPointF) const;

    AbstractMsaView *abstractMsaView_;
};

#endif // POINTRECTMAPPER_H
