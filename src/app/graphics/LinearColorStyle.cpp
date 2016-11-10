/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "LinearColorStyle.h"
#include "../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
LinearColorStyle::LinearColorStyle()
    : increments_(0),
      valueRange_(qMakePair(0.0, 1.0)),
      startHSVA_(QColor::fromHsvF(0., 0., 0.)),     // Black
      stopHSVA_(QColor::fromHsvF(0., 0., 1.))       // White
{
}

/**
  * @param start [const QPair<QColor, double> &]
  * @param stop [const QPair<QColor, double> &]
  * @param increments [const int]
  */
LinearColorStyle::LinearColorStyle(const QPair<QColor, double> &start,
                                   const QPair<QColor, double> &stop,
                                   const int increments)
    : increments_(increments),
      valueRange_(PairDouble(start.second, stop.second)),
      startHSVA_(start.first),
      stopHSVA_(stop.first)
{
    ASSERT(increments >= 0);
    ASSERT(valueRange_.first <= valueRange_.second);
    buildColorTable();
}

/**
  * @param colors [const PairQColor &]
  * @param increments [const int]
  * @param valueRange [const PairDouble &]
  */
LinearColorStyle::LinearColorStyle(const PairQColor &colors, const int increments, const PairDouble &valueRange)
    : increments_(increments),
      valueRange_(valueRange),
      startHSVA_(colors.first),
      stopHSVA_(colors.second)
{
    ASSERT(increments >= 0);
    ASSERT(valueRange_.first <= valueRange_.second);
    buildColorTable();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns PairQColor
  */
PairQColor LinearColorStyle::colors() const
{
    return qMakePair(startHSVA_.toColor(), stopHSVA_.toColor());
}

/**
  * @returns int
  */
int LinearColorStyle::increments() const
{
    return increments_;
}

/**
  * @returns bool
  */
bool LinearColorStyle::isContinuous() const
{
    return increments_ == 0;
}

/**
  * @returns bool
  */
bool LinearColorStyle::isDiscrete() const
{
    return increments_ > 0;
}

/**
  * @param value [const double]
  * @returns QColor
  */
QColor LinearColorStyle::linearColor(const double value) const
{
    double normValue = normalizeValue(value);
    if (isContinuous())
        return interpolate(normValue, startHSVA_, stopHSVA_);

    // Otherwise, using a discrete number of interpolated colors
    int index = static_cast<int>(normValue * increments_);
    ASSERT(index <= interpolatedColors_.size());

    // Special case: when the normalized value is 1, then the preceding calculation returns an out of range index. Thus,
    //               manually set it to the last interpolated color.
    if (index == interpolatedColors_.size())
        --index;

    return interpolatedColors_[index];
}

/**
  * @returns PairDouble
  */
PairDouble LinearColorStyle::range() const
{
    return valueRange_;
}

/**
  * @param increments [const int]
  */
void LinearColorStyle::setIncrements(const int increments)
{
    ASSERT(increments >= 0);
    increments_ = increments;
    buildColorTable();
}

/**
  * @param valueRange [const PairDouble &]
  */
void LinearColorStyle::setRange(const PairDouble &valueRange)
{
    ASSERT(valueRange_.first <= valueRange_.second);
    valueRange_ = valueRange;
    buildColorTable();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param value [const double]
  * @returns double
  */
double LinearColorStyle::boundValue(const double value) const
{
    return qBound(valueRange_.first, value, valueRange_.second);
}

/**
  * The color table is only relevant for mapping values to a set of uniformly distributed, discrete colors. Depending on
  * the number of increments, a few special cases arise. In particular, the discrete color set contains
  *
  * If increments == 1: start color
  * If increments == 2: start color and stop color
  * If increments > 2: start color, (increments - 2) uniformly distributed colors, stop color
  *
  * This color vector corresponds to the normal range of values between 0 and 1 and it is helpful to consider it
  * similarly to the numbering scheme for string positions. For instance, given 3 increments between white and black,
  * this would look like the following:
  *
  *  Input values: 0         .33        .66         1
  *                |----------|----------|----------|
  * Output colors: <- white  -><- gray  -><- black ->
  *
  * f(x) <= .33 -> white
  * f(x) > .33 && <= .66 -> gray
  * f(x) > .66 && <= 1 -> black
  *
  * Thus, even though the input values are 0 and 1, the normalized values used to determine the value thresholds for
  * each color are 0, .33, and .66. This is slightly unintuitive because one might naturally think that the three
  * values should be 0, .5, and 1.
  */
void LinearColorStyle::buildColorTable()
{
    // In smooth mode, all colors are always dynamically calculated
    if (isContinuous())
        return;

    interpolatedColors_.resize(increments_);

    // Always have the start color
    interpolatedColors_[0] = startHSVA_.toColor();

    for (int i=1; i< increments_ - 1; ++i)
    {
        double p = static_cast<double>(i) / static_cast<double>(increments_ - 1);
        interpolatedColors_[i] = interpolate(p, startHSVA_, stopHSVA_);
    }

    // Include the stop color if there is room
    if (increments_ > 1)
        interpolatedColors_[increments_ - 1] = stopHSVA_.toColor();
}

/**
  * @param value [const double]
  * @param startColor [const HSVA &]
  * @param stopColor [const HSVA &]
  * @returns QColor
  */
QColor LinearColorStyle::interpolate(const double value, const HSVA &startColor, const HSVA &stopColor) const
{
    ASSERT(value >= 0 && value <= 1.);

    double hue = 0;
    if (startColor.h_ != -1 && stopColor.h_ != -1)
        hue = startColor.h_ + value * (stopColor.h_ - startColor.h_);
    // Deal with achromatic colors
    else if (startColor.h_ == -1)
        hue = stopColor.h_;
    else // stop.h_ == -1
        hue = startColor.h_;

    return QColor::fromHsvF(hue,
                            startColor.s_ + value * (stopColor.s_ - startColor.s_),
                            startColor.v_ + value * (stopColor.v_ - startColor.v_),
                            startColor.a_ + value * (stopColor.a_ - startColor.a_));
}

/**
  * @param value [const double]
  * @returns double
  */
double LinearColorStyle::normalizeValue(const double value) const
{
    return (boundValue(value) - valueRange_.first) / (valueRange_.second - valueRange_.first);
}
