/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef KALIGNMSABUILDEROPTIONSWIDGET_H
#define KALIGNMSABUILDEROPTIONSWIDGET_H

#include "IMsaBuilderOptionsWidget.h"

namespace Ui {
    class KalignMsaBuilderOptionsWidget;
}

class KalignMsaBuilderOptionsWidget : public IMsaBuilderOptionsWidget
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit KalignMsaBuilderOptionsWidget(QWidget *parent = nullptr);
    ~KalignMsaBuilderOptionsWidget();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    OptionSet msaBuilderOptions() const;
    void setGapOpenPenalty(const double newGapOpenPenalty);
    void setGapExtendPenalty(const double newGapExtendPenalty);
    void setTerminalGapPenalty(const double newTerminalGapPenalty);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setPenaltySpinBoxesPrecision(const int decimals);
    void setPenaltySpinBoxesVisiblePrecision(const int decimals);
    QString sortMethod() const;
    QString distanceMethod() const;
    QString guideTree() const;

    Ui::KalignMsaBuilderOptionsWidget *ui_;
};

#endif // KALIGNMSABUILDEROPTIONSWIDGET_H
