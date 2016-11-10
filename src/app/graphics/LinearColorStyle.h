/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef LINEARCOLORSTYLE_H
#define LINEARCOLORSTYLE_H

#include <QtCore/QVector>
#include <QtGui/QColor>

#include "PODs/HSVA.h"
#include "../core/types.h"

/**
  * LinearColorStyle calculates interpolated colors from numerical values using a linear mapping between two arbitrary
  * colors.
  *
  * Many visualizations depend upon representing a range of values using a predictable and aesthetic mapping to some
  * color space. For instance, when visualizng secondary structure predictions it is desirable to shade the background
  * of each amino acid in the MSA according to the confidence of this prediction - darker shades indicating a higher
  * confidence and vice versa. To achieve this goal, LinearColorStyle takes a real number between two values (inclusive)
  * and two cognate colors, and determines the approximate color that most closely resembles this value using a linear
  * interpolation in the HSVA color space. By default this range is 0 to 1; however, any user-defined range may be used.
  *
  * LinearColorStyle may produce an continuous range of colors based; however, in some cases it is desirable to restrict
  * the available colors to a discrete set. This is accomplished by setting the number of increments to a positive
  * value that is also the desired number of discrete colors.
  *
  * All colors that are returned are in the Hsv color space, yet the start and stop input colors specification can be
  * any valid color specification.
  *
  * Constraints:
  * o The range is a positive (start < stop).
  *
  * Idea? Remove the colors_ member since we are storing the color in a different format (ie. HSVA)?
  */
class LinearColorStyle
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    LinearColorStyle();                                 //!< Construct default linear color style with the range 0 to 1 and the colors black to white
    //! Construct a style with the range start.first to start.second and corresponding colors start.first and start.second among increments discrete values (or infinite if increments is 0)
    LinearColorStyle(const QPair<QColor, double> &start, const QPair<QColor, double> &stop, const int increments = 0);
    //! Construct a style with valueRange and colors among increments discrete values (or infinite if increments is 0)
    LinearColorStyle(const PairQColor &colors, const int increments = 0, const PairDouble &valueRange = PairDouble(0.0, 1.0));

    // ------------------------------------------------------------------------------------------------
    // Public methods
    PairQColor colors() const;                          //!< Returns the start and stop colors being mapped
    int increments() const;                             //!< Returns the number of discrete color values; 0 indicates a continuous color space
    bool isContinuous() const;                          //!< Returns true if the calculated colors returned by linearColor is from a continuous space; false otherwise
    bool isDiscrete() const;                            //!< Returns true if the calculated colors returned by linearColor is from a discrete space; false otherwise
    QColor linearColor(const double value) const;       //!< Returns the linearly interpolated color for value (uses a discete color set if increments is positive)
    PairDouble range() const;                           //!< Returns the value range being mapped
    void setIncrements(const int increments);           //!< Sets the number of increments to increments
    void setRange(const PairDouble &valueRange);        //!< Sets the mapping value range to valueRange

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    double boundValue(const double value) const;        //!< Simply clamps value to the current value range
    void buildColorTable();                             //!< Builds the set of discrete colors if using discrete colors
    //!< Returns the interpolated HSVF color between startColor and stopColor for value
    QColor interpolate(const double value, const HSVA &startColor, const HSVA &stopColor) const;
    double normalizeValue(const double value) const;    //!< Bounds values and then linearly maps to the value range

    // Private members
    int increments_;
    PairDouble valueRange_;
    HSVA startHSVA_;
    HSVA stopHSVA_;
    QVector<QColor> interpolatedColors_;
};

Q_DECLARE_TYPEINFO(LinearColorStyle, Q_MOVABLE_TYPE);

#endif // LINEARCOLORSTYLE_H
