/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QTimer>

#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>

#include "ConsensusGroupsDialog.h"
#include "ui_ConsensusGroupsDialog.h"

#include "../../delegates/RegexDelegate.h"
#include "../../models/ConsensusGroupsModel.h"
#include "../../../core/BioSymbol.h"
#include "../../../core/BioSymbolGroup.h"
#include "../../../core/data/CommonBioSymbolGroups.h"
#include "../../../core/macros.h"
#include "../../../core/constants.h"

#include "../../../core/misc.h"
#include "../../gui_misc.h"


ConsensusGroupsDialog::ConsensusGroupsDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::ConsensusGroupsDialog),
    consensusGroupsModel_(nullptr)
{
    ui_->setupUi(this);
    consensusGroupsModel_ = new ConsensusGroupsModel(this);
    ui_->tableView_->setModel(consensusGroupsModel_);

    connect(ui_->resetToDefaultsButton_, SIGNAL(released()), SLOT(resetToDefaults()));
    connect(ui_->newGroupButton_, SIGNAL(released()), SLOT(appendRowAndBeginEditing()));

    RegexDelegate *symbolDelegate = new RegexDelegate(this);
    symbolDelegate->setRegExp(QRegExp(QString("^[%1]$").arg(constants::k7BitCharacters)));
    ui_->tableView_->setItemDelegateForColumn(ConsensusGroupsModel::eSymbolColumn, symbolDelegate);

    RegexDelegate *residuesDelegate = new RegexDelegate(this);
    QString validUpperCaseAminoSymbols = constants::kAminoAmbiguousCharacters;
    QString validLowerCaseAminoSymbols = validUpperCaseAminoSymbols.toLower();
    residuesDelegate->setRegExp(QRegExp(QString("^[%1%2]*$").arg(validUpperCaseAminoSymbols).arg(validLowerCaseAminoSymbols)));
    ui_->tableView_->setItemDelegateForColumn(ConsensusGroupsModel::eResiduesColumn, residuesDelegate);

    connect(consensusGroupsModel_, SIGNAL(bioSymbolGroupValidChanged(bool)), ui_->buttonBox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));

    // ---------------
    // Intercept events from the label view
    ui_->tableView_->installEventFilter(this);
}

ConsensusGroupsDialog::~ConsensusGroupsDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
BioSymbolGroup ConsensusGroupsDialog::bioSymbolGroup() const
{
    return consensusGroupsModel_->bioSymbolGroup();
}

void ConsensusGroupsDialog::setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup)
{
    consensusGroupsModel_->setBioSymbolGroup(bioSymbolGroup);
}

void ConsensusGroupsDialog::setUseDefaultGroups(bool useDefaultGroups)
{
    if (useDefaultGroups)
        ui_->useDefaultGroupsRadioButton_->setChecked(true);
    else
        ui_->useCustomGroupsRadioButton_->setChecked(true);
}

bool ConsensusGroupsDialog::useDefaultGroups() const
{
    return ui_->useDefaultGroupsRadioButton_->isChecked();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
bool ConsensusGroupsDialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui_->tableView_)
        return tableViewEventFilter(event);

    return QObject::eventFilter(object, event);

}

bool ConsensusGroupsDialog::tableViewEventFilter(QEvent *event)
{
    if (event->type() == QKeyEvent::KeyPress &&
        ui_->tableView_->model() != nullptr)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete)
        {
            // Check for any selected sequences
            QModelIndexList selectedIndexes = ui_->tableView_->selectionModel()->selectedIndexes();
            if (selectedIndexes.isEmpty())
                return false;

            QVector<int> selectedRows = ::reduceToUniqueRows(selectedIndexes);
            QVector<ClosedIntRange> selectedRanges = ::convertIntVectorToClosedIntRanges(selectedRows);

            for (int i=selectedRanges.size() - 1; i>= 0; --i)
            {
                ClosedIntRange selectedRange = selectedRanges.at(i);
                consensusGroupsModel_->removeRows(selectedRange.begin_, selectedRange.length());
            }
            event->accept();
            return true;
        }
    }

    return false;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
void ConsensusGroupsDialog::appendRowAndBeginEditing()
{
    QModelIndex newIndex = consensusGroupsModel_->appendEmptyRow();
    ASSERT(newIndex.isValid());
    ui_->tableView_->scrollToBottom();
    ui_->tableView_->setCurrentIndex(newIndex);

    appendedIndex_ = newIndex;

    // Have to wait for the event queue to be processed before we can begin editing
    QTimer::singleShot(0, this, SLOT(beginEditing()));
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
void ConsensusGroupsDialog::resetToDefaults()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm reset");
    msgBox.setText("Are you sure you want to reset the groupings below to the default setting?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    if (msgBox.exec() == QMessageBox::Cancel)
        return;

    setBioSymbolGroup(constants::CommonBioSymbolGroups::defaultConsensusSymbolGroup());
}

void ConsensusGroupsDialog::beginEditing()
{
    ui_->tableView_->edit(appendedIndex_);
    appendedIndex_ = QModelIndex();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
