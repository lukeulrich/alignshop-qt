/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFontComboBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>

#include "FontAndSizeChooser.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
FontAndSizeChooser::FontAndSizeChooser(QWidget *parent)
    : QWidget(parent),
      fontComboBox_(nullptr),
      pointSizeComboBox_(nullptr)
{
    fontComboBox_ = new QFontComboBox;
    pointSizeComboBox_ = makePointSizeComboBox();

    QHBoxLayout *horzLayout = new QHBoxLayout;
    horzLayout->addWidget(fontComboBox_);
    horzLayout->addWidget(pointSizeComboBox_);
    setLayout(horzLayout);

    connect(fontComboBox_, SIGNAL(currentFontChanged(QFont)), SLOT(onFontComboBoxFontChanged()));
    connect(pointSizeComboBox_, SIGNAL(currentIndexChanged(int)), SLOT(onPointSizeComboBoxCurrentChanged()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
QFont FontAndSizeChooser::currentFont() const
{
    QFont font = fontComboBox_->currentFont();
    font.setPointSize(pointSize());
    return font;
}

int FontAndSizeChooser::pointSize() const
{
    return pointSizeFromIndex(pointSizeComboBox_->currentIndex());
}

void FontAndSizeChooser::setCurrentFont(const QFont &font)
{
    if (font == currentFont())
        return;

    fontComboBox_->setCurrentFont(font);
    pointSizeComboBox_->setCurrentIndex(findIndexWithBestPointSizeMatch(font.pointSize()));
    emit fontChanged(currentFont());
}

void FontAndSizeChooser::setFamily(const QString &family)
{
    QFont font;
    font.setFamily(family);
    fontComboBox_->setCurrentFont(font);
}

void FontAndSizeChooser::setPointSize(const int newPointSize)
{
    int newPointIndex = findIndexWithBestPointSizeMatch(newPointSize);
    if (newPointIndex == pointSizeComboBox_->currentIndex())
        return;

    pointSizeComboBox_->setCurrentIndex(newPointIndex);
    emit fontChanged(currentFont());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void FontAndSizeChooser::onFontComboBoxFontChanged()
{
    emit fontChanged(currentFont());
}

void FontAndSizeChooser::onPointSizeComboBoxCurrentChanged()
{
    emit fontChanged(currentFont());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QComboBox *FontAndSizeChooser::makePointSizeComboBox() const
{
    QComboBox *comboBox = new QComboBox;
    comboBox->addItems(allowedPointSizesAsStrings());
    return comboBox;
}

int FontAndSizeChooser::findIndexWithBestPointSizeMatch(const int queryPointSize) const
{
    if (queryPointSize <= minimumAllowedPointSize())
        return 0;

    for (int i=1, z=pointSizeComboBox_->count(); i<z; ++i)
    {
        int pointSize = pointSizeFromIndex(i);
        if (pointSize == queryPointSize)
            return i;

        if (queryPointSize < pointSize)
            return i-1;
    }

    return pointSizeComboBox_->count() - 1;
}

int FontAndSizeChooser::pointSizeFromIndex(const int index) const
{
    ASSERT(index >= 0 && index < pointSizeComboBox_->count());

    return pointSizeComboBox_->itemText(index).toInt();
}

int FontAndSizeChooser::minimumAllowedPointSize() const
{
    return pointSizeFromIndex(0);
}

int FontAndSizeChooser::maximumAllowedPointSize() const
{
    return pointSizeFromIndex(pointSizeComboBox_->count() - 1);
}

QStringList FontAndSizeChooser::allowedPointSizesAsStrings() const
{
    QVector<int> pointSizes;
    pointSizes << 4 << 6 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 16 << 18
               << 20 << 22 << 24 << 28 << 32 << 36 << 40 << 48 << 56 << 64
               << 72 << 144;

    QStringList stringifiedPointSizes;
    foreach (const int pointSize, pointSizes)
        stringifiedPointSizes << QString::number(pointSize);
    return stringifiedPointSizes;
}

