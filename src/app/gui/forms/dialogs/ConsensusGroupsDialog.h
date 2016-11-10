/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef CONSENSUSGROUPSDIALOG_H
#define CONSENSUSGROUPSDIALOG_H

#include <QtGui/QDialog>

#include <QtCore/QModelIndex>

namespace Ui {
    class ConsensusGroupsDialog;
}

class QModelIndex;

class BioSymbolGroup;
class ConsensusGroupsModel;

class ConsensusGroupsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConsensusGroupsDialog(QWidget *parent = 0);
    ~ConsensusGroupsDialog();

    BioSymbolGroup bioSymbolGroup() const;
    void setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup);
    void setUseDefaultGroups(bool useDefaultGroups);
    bool useDefaultGroups() const;


protected:
    bool eventFilter(QObject *object, QEvent *event);
    bool tableViewEventFilter(QEvent *event);


private Q_SLOTS:
    void appendRowAndBeginEditing();
    void resetToDefaults();
    void beginEditing();


private:
    Ui::ConsensusGroupsDialog *ui_;
    ConsensusGroupsModel *consensusGroupsModel_;
    QModelIndex appendedIndex_;
};

#endif // CONSENSUSGROUPSDIALOG_H
