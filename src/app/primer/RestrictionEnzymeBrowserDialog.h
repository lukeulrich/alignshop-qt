/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RESTRICTIONENZYMEBROWSERDIALOG_H
#define RESTRICTIONENZYMEBROWSERDIALOG_H

#include <QtGui/QDialog>
#include "RestrictionEnzyme.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class QSortFilterProxyModel;

class RestrictionEnzymeTableModel;

namespace Ui {
    class RestrictionEnzymeBrowserDialog;
}

class RestrictionEnzymeBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RestrictionEnzymeBrowserDialog(QWidget *parent = 0);
    ~RestrictionEnzymeBrowserDialog();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    RestrictionEnzyme selectedRestrictionEnzyme() const;

    void setRebaseFile(const QString &rebaseFile);
    void setSelectedRestrictedSite(const QString &name);


private Q_SLOTS:
    // ------------------------------------------------------------------------------------------------
    // Private slots
    void onSelectionChanged();


private:
    Ui::RestrictionEnzymeBrowserDialog *ui_;
    RestrictionEnzymeTableModel *restrictionEnzymeModel_;
    QSortFilterProxyModel *sortFilterProxyModel_;
};

#endif // RESTRICTIONENZYMEBROWSERDIALOG_H
