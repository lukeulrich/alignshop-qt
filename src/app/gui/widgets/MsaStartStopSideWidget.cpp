/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPainter>

#include "MsaStartStopSideWidget.h"

#include "AbstractMsaView.h"
#include "../../core/ObservableMsa.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QWidget *]
  */
MsaStartStopSideWidget::MsaStartStopSideWidget(QWidget *parent)
    : AbstractMsaSideWidget(parent),
      positionType_(StartType)
{
    QFont font(this->font());
    font.setItalic(true);
    setFont(font);
}

/**
  * @param msaView [AbstractMsaView *]
  * @param positionType [const PositionType]
  * @param parent [QWidget *]
  */
MsaStartStopSideWidget::MsaStartStopSideWidget(AbstractMsaView *msaView, const PositionType positionType, QWidget *parent)
    : AbstractMsaSideWidget(msaView, parent),
      positionType_(positionType)
{
    QFont font(this->font());
    font.setItalic(true);
    setFont(font);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns PositionType
  */
MsaStartStopSideWidget::PositionType MsaStartStopSideWidget::positionType() const
{
    return positionType_;
}

/**
  * @param positionType [const PositionType]
  */
void MsaStartStopSideWidget::setPositionType(const PositionType positionType)
{
    if (positionType == positionType_)
        return;

    positionType_ = positionType;
    emit positionTypeChanged();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  */
void MsaStartStopSideWidget::updateWidth()
{
    if (msaView_ == nullptr)
        return;

    ObservableMsa *msa = msaView_->msa();
    if (msa == nullptr)
        return;

    // Depending on the position type, find the smallest/largest values and then calculate the width for the current
    // font
    QFontMetrics fontMetrics(font());
    QString longestString = longestStringForType(msa, positionType_);
    int newWidth = fontMetrics.width(longestString) + 2 * horizontalPadding();

    QRect geometry = this->geometry();
    geometry.setWidth(newWidth);
    setGeometry(geometry);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param painter [QPainter *]
  * @param rowHeight [const int]
  * @param baseline [const double]
  * @param startY [const int]
  * @param startMsaRow [const int]
  */
void MsaStartStopSideWidget::paintEvent(QPainter *painter, const int rowHeight, const double baseline, const int startY, const int startMsaRow)
{
    if (msaView_ == nullptr)
        return;

    ObservableMsa *msa = msaView_->msa();
    if (msa == nullptr)
        return;

    QFontMetrics fontMetrics(font());
    int y = startY;
    int row = startMsaRow;

    // Turn off anti-aliasing if text is really small
    painter->setRenderHint(QPainter::Antialiasing, font().pointSize() >= 10);

    painter->setFont(font());
//    painter->setPen(palette().color(QPalette::Text));
    painter->setPen(QColor(64, 64, 64));
    for (int h = height(), nRows = msa->rowCount(); y < h && row <= nRows; y += rowHeight, ++row)
    {
        QString label;
        switch (positionType_)
        {
        case StartType:
            label = QString::number(msa->at(row)->start());
            break;
        case StopType:
            label = QString::number(msa->at(row)->stop());
            break;
        case InverseStartType:
            label = QString::number(msa->at(row)->inverseStart());
            break;
        case InverseStopType:
            label = QString::number(msa->at(row)->inverseStop());
            break;
        }

        // Determine where it is to be placed
        qreal x = horizontalPadding();
        if (alignment() & Qt::AlignRight)
            x = width() - fontMetrics.width(label) - horizontalPadding();
        else if (alignment() & Qt::AlignCenter)
            x = (width() - fontMetrics.width(label)) / 2.;

        painter->drawText(x, y + baseline, label);
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param msa [const ObservableMsa *]
  * @param positionType [const PositionType positionType]
  * @returns QString
  */
QString MsaStartStopSideWidget::longestStringForType(const ObservableMsa *msa, const PositionType positionType) const
{
    ASSERT(msa);

    int largest = 0;
    if (positionType == StartType)
    {
        for (int i=1, z= msa->rowCount(); i<=z; ++i)
            if (msa->at(i)->start() > largest)
                largest = msa->at(i)->start();
    }
    else if (positionType == StopType)
    {
        for (int i=1, z= msa->rowCount(); i<=z; ++i)
            if (msa->at(i)->stop() > largest)
                largest = msa->at(i)->stop();
    }
    else if (positionType == InverseStartType)
    {
        for (int i=1, z= msa->rowCount(); i<=z; ++i)
            if (msa->at(i)->inverseStart() < largest)
                largest = msa->at(i)->inverseStart();
    }
    else if (positionType == InverseStopType)
    {
        for (int i=1, z= msa->rowCount(); i<=z; ++i)
            if (msa->at(i)->inverseStop() < largest)
                largest = msa->at(i)->inverseStop();
    }
    else
    {
        ASSERT_X(0, "Unreachable!");
    }

    return QString::number(largest);
}

