/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

#include "RestrictionEnzymeBrowserDialog.h"
#include "ui_RestrictionEnzymeBrowserDialog.h"
#include "RestrictionEnzymeTableModel.h"
#include "../core/global.h"

RestrictionEnzymeBrowserDialog::RestrictionEnzymeBrowserDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::RestrictionEnzymeBrowserDialog),
    restrictionEnzymeModel_(nullptr)
{
    ui_->setupUi(this);

    restrictionEnzymeModel_ = new RestrictionEnzymeTableModel(this);

    sortFilterProxyModel_ = new QSortFilterProxyModel(this);
    sortFilterProxyModel_->setSourceModel(restrictionEnzymeModel_);

    ui_->tableView->setModel(sortFilterProxyModel_);
    ui_->tableView->resizeColumnsToContents();

    connect(ui_->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onSelectionChanged()));

    connect(ui_->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(released()), SLOT(accept()));
    connect(ui_->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(released()), SLOT(reject()));
}

/**
  */
RestrictionEnzymeBrowserDialog::~RestrictionEnzymeBrowserDialog()
{
    delete ui_;
}

/**
  * @returns RestrictionEnzyme
  */
RestrictionEnzyme RestrictionEnzymeBrowserDialog::selectedRestrictionEnzyme() const
{
    return restrictionEnzymeModel_->restrictionEnzymeAt(sortFilterProxyModel_->mapToSource(ui_->tableView->currentIndex()));
}

/**
  * @param rebaseFile [const QString &]
  */
void RestrictionEnzymeBrowserDialog::setRebaseFile(const QString &rebaseFile)
{
    restrictionEnzymeModel_->loadRebaseFile(rebaseFile);
}

/**
  * @param name [const QString &]
  */
void RestrictionEnzymeBrowserDialog::setSelectedRestrictedSite(const QString &name)
{
    int sourceRow = restrictionEnzymeModel_->findRowWithName(name);
    if (sourceRow == -1)
        return;

    QModelIndex sourceIndex = restrictionEnzymeModel_->index(sourceRow, 0);
    QModelIndex sortedIndex = sortFilterProxyModel_->mapFromSource(sourceIndex);
    ui_->tableView->selectRow(sortedIndex.row());
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void RestrictionEnzymeBrowserDialog::onSelectionChanged()
{
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui_->tableView->selectionModel()->selection().isEmpty());
}
