/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SELECTGROUPNODEDIALOG_H
#define SELECTGROUPNODEDIALOG_H

#include <QtGui/QDialog>

class QModelIndex;
class AdocTreeModel;
class AdocTreeNodeFilterModel;

namespace Ui {
    class SelectGroupNodeDialog;
}

class SelectGroupNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectGroupNodeDialog(QWidget *parent = 0);
    ~SelectGroupNodeDialog();

    QModelIndex selectedGroupIndex() const;
    void setSelectedGroup(const QModelIndex &groupIndex);
    void setAdocTreeModel(AdocTreeModel *adocTreeModel);
    void setLabelText(const QString &text);

private Q_SLOTS:
    void makeNewGroupButtonClicked();
    void onTreeViewCurrentChanged(const QModelIndex &currentIndex);

private:
    Ui::SelectGroupNodeDialog *ui_;
    AdocTreeModel *adocTreeModel_;
    AdocTreeNodeFilterModel *groupModel_;
};

#endif // SELECTGROUPNODEDIALOG_H
