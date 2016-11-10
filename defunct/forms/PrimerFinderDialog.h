#ifndef PRIMERFINDERDIALOG_H
#define PRIMERFINDERDIALOG_H

#include <QtCore/QTextStream>

#include <QtGui/QDialog>
#include <QtGui/QMessageBox>

#include "RestrictionEnzymeDialog.h"

#include "ui_PrimerFinderDialog.h"


namespace Ui {
    class PrimerFinderDialog;
}

class PrimerFinderDialog : public QDialog, public Ui::PrimerFinderDialog
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Constructs an instance of this dialog and performs other necessary intialization including: signals/slots setup, detector configuration, and miscellaneous widget details
    explicit PrimerFinderDialog(QWidget *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods

protected:


private slots:

    void onSelectRestrictionSite_clicked();
    //add function here ui->clicked()

private:
    RestrictionEnzymeDialog *restrictionEnzymeDialog_;        //!< Detailed dialog box for selecting a restriction site


};

#endif // PRIMERFINDERDIALOG_H
