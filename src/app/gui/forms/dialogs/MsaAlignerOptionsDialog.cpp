/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QPushButton>

#include "MsaAlignerOptionsDialog.h"
#include "ui_MsaAlignerOptionsDialog.h"
#include "../../widgets/IMsaBuilderOptionsWidget.h"
#include "../../../core/util/OptionSet.h"
#include "../../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
MsaAlignerOptionsDialog::MsaAlignerOptionsDialog(QWidget *parent)
    : QDialog(parent),
      ui_(new Ui::MsaAlignerOptionsDialog),
      activeMsaBuilderOptionsWidget_(nullptr)
{
    ui_->setupUi(this);
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setText("Create alignment");
    connect(ui_->alignerComboBox_, SIGNAL(currentIndexChanged(int)), SLOT(onAlignerChanged(int)));
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    ui_->optionContainerWidget_->setLayout(new QVBoxLayout());
    ui_->optionContainerWidget_->layout()->setMargin(0);
}

MsaAlignerOptionsDialog::~MsaAlignerOptionsDialog()
{
    clearExistingMsaBuilderOptionWidgets();
    delete ui_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void MsaAlignerOptionsDialog::setMsaBuilderOptionsWidgets(const QVector<IMsaBuilderOptionsWidget *> &msaBuilderOptionsWidgets)
{
    clearExistingMsaBuilderOptionWidgets();
    msaBuilderOptionsWidgets_ = msaBuilderOptionsWidgets;
    updateAlignerComboBox();
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(msaBuilderOptionsWidgets_.size() > 0);
}

QString MsaAlignerOptionsDialog::msaBuilderId() const
{
    return ui_->alignerComboBox_->currentText();
}

OptionSet MsaAlignerOptionsDialog::msaBuilderOptions() const
{
    if (ui_->alignerComboBox_->count() == 0)
        return OptionSet();

    return currentMsaBuilderOptionsWidget()->msaBuilderOptions();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void MsaAlignerOptionsDialog::onAlignerChanged(const int index)
{
    IMsaBuilderOptionsWidget *newMsaBuilderOptionsWidget = msaBuilderOptionsWidgetForIndex(index);
    if (newMsaBuilderOptionsWidget == activeMsaBuilderOptionsWidget_)
        return;

    hideCurrentMsaBuilderOptionsWidget();
    setActiveMsaBuilderOptionsWidget(newMsaBuilderOptionsWidget);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void MsaAlignerOptionsDialog::clearExistingMsaBuilderOptionWidgets()
{
    qDeleteAll(msaBuilderOptionsWidgets_);
    msaBuilderOptionsWidgets_.clear();
}

void MsaAlignerOptionsDialog::updateAlignerComboBox()
{
    ui_->alignerComboBox_->clear();
    foreach (const IMsaBuilderOptionsWidget *msaBuilderOptionsWidget, msaBuilderOptionsWidgets_)
    {
        ASSERT(msaBuilderOptionsWidget != nullptr);
        ui_->alignerComboBox_->addItem(msaBuilderOptionsWidget->msaBuilderId());
    }
}

IMsaBuilderOptionsWidget *MsaAlignerOptionsDialog::currentMsaBuilderOptionsWidget() const
{
    int alignerIndex = ui_->alignerComboBox_->currentIndex();
    if (alignerIndex == -1)
        return nullptr;

    return msaBuilderOptionsWidgetForIndex(alignerIndex);
}

IMsaBuilderOptionsWidget *MsaAlignerOptionsDialog::msaBuilderOptionsWidgetForIndex(const int index) const
{
    return msaBuilderOptionsWidgets_.at(index);
}

void MsaAlignerOptionsDialog::hideCurrentMsaBuilderOptionsWidget()
{
    if (activeMsaBuilderOptionsWidget_ != nullptr)
    {
        activeMsaBuilderOptionsWidget_->hide();
        ui_->optionContainerWidget_->layout()->removeWidget(activeMsaBuilderOptionsWidget_);
    }
    activeMsaBuilderOptionsWidget_ = nullptr;
}

void MsaAlignerOptionsDialog::setActiveMsaBuilderOptionsWidget(IMsaBuilderOptionsWidget *newMsaBuilderOptionsWidget)
{
    activeMsaBuilderOptionsWidget_ = newMsaBuilderOptionsWidget;
    activeMsaBuilderOptionsWidget_->show();
    ui_->optionContainerWidget_->layout()->addWidget(activeMsaBuilderOptionsWidget_);
}
