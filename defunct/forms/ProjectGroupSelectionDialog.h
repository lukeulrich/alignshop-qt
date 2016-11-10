#ifndef PROJECTGROUPSELECTIONDIALOG_H
#define PROJECTGROUPSELECTIONDIALOG_H

#include <QtGui/QDialog>

#include "../models/AdocTreeModel.h"
#include "../models/AdocTypeFilterModel.h"

namespace Ui {
    class ProjectGroupSelectionDialog;
}

/**
  * ProjectGroupSelectionDialog provides the means to select a single project or group node index for
  * downstream use.
  *
  * It requires a source AdocTreeModel supplied upon construction
  */
class ProjectGroupSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct an instance of this object, set the source model to AdocTreeModel and perform other necessary configuration
    ProjectGroupSelectionDialog(AdocTreeModel *adocTreeModel = 0, QWidget *parent = 0);
    ~ProjectGroupSelectionDialog();                     //!< Clean-up

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QModelIndex selectedIndex() const;                  //!< Returns the source model index of the currently selected index in the treeview
    void setAdocModel(AdocTreeModel *adocTreeModel);    //!< Set the model to be used by the tree view to adocTreeModel

private slots:
    void newGroupButtonClicked();                       //!< Response handler for when the New Group button is clicked
    void newProjectButtonClicked();                     //!< Response handler for when the New Project button is clicked

private:
    Ui::ProjectGroupSelectionDialog *ui_;
    AdocTreeModel *sourceTreeModel_;                    //!< Original model used to populate this dialog box
    AdocTypeFilterModel typeFilterModel_;               //!< Type filter model for selecting only project and group nodes
};

#endif // PROJECTGROUPSELECTIONDIALOG_H
