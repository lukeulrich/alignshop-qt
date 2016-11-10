/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QMessageBox>

#include "SelectGroupNodeDialog.h"
#include "ui_SelectGroupNodeDialog.h"

#include "../../core/global.h"
#include "../../core/macros.h"
#include "../delegates/LineEditDelegate.h"
#include "../models/AdocTreeModel.h"
#include "../models/AdocTreeNodeFilterModel.h"
#include "../models/CustomRoles.h"

SelectGroupNodeDialog::SelectGroupNodeDialog(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::SelectGroupNodeDialog),
    adocTreeModel_(nullptr),
    groupModel_(nullptr)
{
    ui_->setupUi(this);
    ui_->treeView->setItemDelegate(new LineEditDelegate(ui_->treeView));
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(ui_->makeNewGroupButton, SIGNAL(clicked()), SLOT(makeNewGroupButtonClicked()));
    ui_->treeView->header()->setSortIndicator(0, Qt::AscendingOrder);
}

SelectGroupNodeDialog::~SelectGroupNodeDialog()
{
    delete ui_;
}

/**
  * Returns the model index of the tree item
  *
  * @returns QModelIndex
  */
QModelIndex SelectGroupNodeDialog::selectedGroupIndex() const
{
    ASSERT(groupModel_ != nullptr);
    ASSERT(adocTreeModel_ != nullptr);
    ASSERT(groupModel_->sourceModel() == adocTreeModel_);

    // Only one item may be selected at a time
    return groupModel_->mapToSource(ui_->treeView->currentIndex());
}

/**
  * groupIndex may be invalid if adding to the root when there are no other nodes.
  *
  * @param groupIndex [const QModelIndex &]
  */
void SelectGroupNodeDialog::setSelectedGroup(const QModelIndex &groupIndex)
{
    ASSERT(adocTreeModel_ != nullptr);
    ASSERT(groupIndex.isValid() == false || groupIndex.model() == adocTreeModel_);
//    ASSERT(groupIndex.data(CustomRoles::kIsGroupRole).toBool() == true);

    if (groupIndex.isValid())
        ui_->treeView->setCurrentIndex(groupModel_->mapFromSource(groupIndex));
}

void SelectGroupNodeDialog::setAdocTreeModel(AdocTreeModel *adocTreeModel)
{
    if (adocTreeModel_ != nullptr)
        disconnect(ui_->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTreeViewCurrentChanged(QModelIndex)));

    adocTreeModel_ = adocTreeModel;
    if (adocTreeModel_ != nullptr)
    {
        if (groupModel_ == nullptr)
        {
            groupModel_ = new AdocTreeNodeFilterModel(this);
            groupModel_->setAcceptableNodeTypes(QSet<AdocNodeType>() << eRootNode << eGroupNode);
            groupModel_->setDynamicSortFilter(true);
            groupModel_->sort(0);
        }
        groupModel_->setSourceModel(adocTreeModel_);
    }
    else
    {
        delete groupModel_;
        groupModel_ = nullptr;
    }

    ui_->treeView->setModel(groupModel_);
    ui_->treeView->sortByColumn(0);
    connect(ui_->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onTreeViewCurrentChanged(QModelIndex)));
}

void SelectGroupNodeDialog::setLabelText(const QString &text)
{
    ui_->label->setText(text);
}

void SelectGroupNodeDialog::makeNewGroupButtonClicked()
{
    if (adocTreeModel_ == nullptr)
        return;

    QModelIndex groupIndex = adocTreeModel_->newGroup("New group", selectedGroupIndex());
    if (groupIndex.isValid())
    {
        groupIndex = groupModel_->mapFromSource(groupIndex);
        ui_->treeView->setCurrentIndex(groupIndex);
        ui_->treeView->edit(groupIndex);
    }
    else
    {
        QMessageBox::warning(this, tr("Unable to create group"), tr("Please select a valid item before creating a new group"), QMessageBox::Ok);
    }
}

/**
  * Enables and disables the Ok button in response to the selection of a valid or invalid index.
  *
  * @param currentIndex [const QModelIndex &]
  */
void SelectGroupNodeDialog::onTreeViewCurrentChanged(const QModelIndex &currentIndex)
{
    if (adocTreeModel_ == nullptr)
        return;

    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(currentIndex.isValid());
}
