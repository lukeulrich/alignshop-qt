#include "RestrictionEnzymeDialog.h"

#include "ui_RestrictionEnzymeDialog.h"


#include <QtDebug>

const int ENZYME_NAME_COL = 0;
const int ENZYME_VALUE_COL = 1;

RestrictionEnzymeDialog::RestrictionEnzymeDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::RestrictionEnzymeDialog)
{
    m_ui->setupUi(this);
    restrictionEnzymeTableModel = new RestrictionEnzymeTableModel(this);
    restrictionEnzymeTableModel->loadRebaseFile("re_list.dat");
    sortedModel_.setSourceModel(restrictionEnzymeTableModel);
    m_ui->restrictionEnzymeTableView->setModel(&sortedModel_);
    m_ui->restrictionEnzymeTableView->resizeColumnsToContents();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

RestrictionEnzymeDialog::~RestrictionEnzymeDialog()
{
    delete m_ui;
}

void RestrictionEnzymeDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void RestrictionEnzymeDialog::accept()
{
    QModelIndex currentIndex = m_ui->restrictionEnzymeTableView->currentIndex();
    QModelIndex enzymeIndex = restrictionEnzymeTableModel->index(currentIndex.row(), ENZYME_VALUE_COL);
    QModelIndex enzymeNameIndex = restrictionEnzymeTableModel->index(currentIndex.row(), ENZYME_NAME_COL);
    selectedRestrictionSite_ = restrictionEnzymeTableModel->data(enzymeIndex, 0).toString();
    selectedRestrictionSiteName_ = restrictionEnzymeTableModel->data(enzymeNameIndex, 0).toString();
    QDialog::accept();
}

QString RestrictionEnzymeDialog::selectedRestrictionSite() const
{
    return selectedRestrictionSite_;
}

QString RestrictionEnzymeDialog::selectedRestrictionSiteName() const
{
    return selectedRestrictionSiteName_;
}

void RestrictionEnzymeDialog::setSelectedRestrictionSite(const QString &name)
{
    for (int i = 0; i < m_ui->restrictionEnzymeTableView->model()->rowCount(); ++i)
    {
        QModelIndex index = restrictionEnzymeTableModel->index(i, ENZYME_NAME_COL);
        QString current = restrictionEnzymeTableModel->data(index, 0).toString();

        if (QString::compare(current, name, Qt::CaseInsensitive) == 0)
        {
            m_ui->restrictionEnzymeTableView->selectRow(sortedModel_.mapFromSource(index).row());
            return;
        }
    }
}
