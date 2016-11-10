#ifndef RESTRICTIONENZYMEDIALOG_H
#define RESTRICTIONENZYMEDIALOG_H

#include <QtGui>
#include <QtGui/QDialog>
#include <QtGui/QAbstractButton>

#include "../models/RestrictionEnzymeTableModel.h"

namespace Ui {
    class RestrictionEnzymeDialog;
}

class RestrictionEnzymeDialog : public QDialog {
    Q_OBJECT
public:
    RestrictionEnzymeDialog(QWidget *parent = 0);
    ~RestrictionEnzymeDialog();
    QString selectedRestrictionSite() const;
    QString selectedRestrictionSiteName() const;

    void setSelectedRestrictionSite(const QString &name);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::RestrictionEnzymeDialog *m_ui;
    RestrictionEnzymeTableModel *restrictionEnzymeTableModel;
    QSortFilterProxyModel sortedModel_;
    QString selectedRestrictionSite_;
    QString selectedRestrictionSiteName_;

private slots:
    void accept();
};

#endif // RESTRICTIONENZYMEDIALOG_H
