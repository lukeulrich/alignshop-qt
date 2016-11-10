/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MSAALIGNEROPTIONSDIALOG_H
#define MSAALIGNEROPTIONSDIALOG_H

#include <QtCore/QVector>
#include <QtGui/QDialog>
#include "../../../core/global.h"

namespace Ui {
    class MsaAlignerOptionsDialog;
}

class IMsaBuilderOptionsWidget;
class OptionSet;

class MsaAlignerOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit MsaAlignerOptionsDialog(QWidget *parent = nullptr);
    ~MsaAlignerOptionsDialog();


    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setMsaBuilderOptionsWidgets(const QVector<IMsaBuilderOptionsWidget *> &msaBuilderOptionsWidgets);
    QString msaBuilderId() const;
    OptionSet msaBuilderOptions() const;


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void onAlignerChanged(const int index);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void clearExistingMsaBuilderOptionWidgets();
    void updateAlignerComboBox();
    IMsaBuilderOptionsWidget *currentMsaBuilderOptionsWidget() const;
    IMsaBuilderOptionsWidget *msaBuilderOptionsWidgetForIndex(const int index) const;
    void hideCurrentMsaBuilderOptionsWidget();
    void setActiveMsaBuilderOptionsWidget(IMsaBuilderOptionsWidget *newMsaBuilderOptionsWidget);

    // ------------------------------------------------------------------------------------------------
    // Private members
    Ui::MsaAlignerOptionsDialog *ui_;
    QVector<IMsaBuilderOptionsWidget *> msaBuilderOptionsWidgets_;
    IMsaBuilderOptionsWidget *activeMsaBuilderOptionsWidget_;
};

#endif // MSAALIGNEROPTIONSDIALOG_H
