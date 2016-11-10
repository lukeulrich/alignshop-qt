#include "ProjectGroupSelectionDialog.h"
#include "ui_ProjectGroupSelectionDialog.h"

#include <QtCore/QTimer>
#include <QtGui/QMessageBox>

#include "widgets/LineEditDelegate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  *
  *
  * @param adocTreeModel [AdocTreeModel *]
  * @param parent [QWidget *]
  */
ProjectGroupSelectionDialog::ProjectGroupSelectionDialog(AdocTreeModel *adocTreeModel, QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::ProjectGroupSelectionDialog)
{
    ui_->setupUi(this);

    // Filter the tree model to only display root, project, and group nodes
    QList<AdocTreeNode::NodeType> keepNodeTypes;
    keepNodeTypes << AdocTreeNode::RootType
                  << AdocTreeNode::GroupType;
    typeFilterModel_.setAcceptNodeTypes(keepNodeTypes);
    ui_->treeView->setModel(&typeFilterModel_);

    ui_->treeView->setItemDelegate(new LineEditDelegate(ui_->treeView));

    // Update the source model used by the TreeView
    setAdocModel(adocTreeModel);

    // Connect the response handlers for the new project/group buttons
    QObject::connect(ui_->newGroupButton, SIGNAL(clicked()), this, SLOT(newGroupButtonClicked()));
    QObject::connect(ui_->newProjectButton, SIGNAL(clicked()), this, SLOT(newProjectButtonClicked()));
}

/**
  */
ProjectGroupSelectionDialog::~ProjectGroupSelectionDialog()
{
    delete ui_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
QModelIndex ProjectGroupSelectionDialog::selectedIndex() const
{
    return typeFilterModel_.mapToSource(ui_->treeView->currentIndex());
}

void ProjectGroupSelectionDialog::setAdocModel(AdocTreeModel *adocTreeModel)
{
    sourceTreeModel_ = adocTreeModel;
    typeFilterModel_.setSourceModel(adocTreeModel);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * If a source model has been defined, attempt to create a new group node beneath the currently selected
  * index. If this is successful, then a QLineEdit editor appears with the group name selected for editing.
  */
void ProjectGroupSelectionDialog::newGroupButtonClicked()
{
    if (!sourceTreeModel_)
        return;

    QModelIndex groupIndex = sourceTreeModel_->newGroup("New group", selectedIndex());
    if (groupIndex.isValid())
    {
        groupIndex = typeFilterModel_.mapFromSource(groupIndex);
        ui_->treeView->setCurrentIndex(groupIndex);
        ui_->treeView->edit(groupIndex);
    }
    else
    {
        QMessageBox::warning(this, tr("Unable to create group"), tr("Please select a valid item before creating a new group"), QMessageBox::Ok);
    }
}

/**
  * If a source model has been defined, attempt to create a new project. If this is successful, then
  * open a QLineEdit editor with the project name selected for editing.
  */
void ProjectGroupSelectionDialog::newProjectButtonClicked()
{
    /*
    if (!sourceTreeModel_)
        return;

    QModelIndex projectIndex = sourceTreeModel_->newProject("New project");
    if (projectIndex.isValid())
    {
        projectIndex = typeFilterModel_.mapFromSource(projectIndex);
        ui_->treeView->setCurrentIndex(projectIndex);
        ui_->treeView->edit(projectIndex);
    }
    else
    {
        QMessageBox::warning(this, tr("Unable to create project"), tr("An unexpected error occurred. Please try again or contact support."), QMessageBox::Ok);
    }
    */
}
