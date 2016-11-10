#include "ThreePrimeInput.h"
#include "ui_ThreePrimeInput.h"

using namespace PrimerDesign;

ThreePrimeInput::ThreePrimeInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreePrimeInput)
{
    ui->setupUi(this);

}

ThreePrimeInput::~ThreePrimeInput()
{
    delete ui;
}

QString ThreePrimeInput::text() const
{
    QString value = ui->pos1->currentText() + ui->pos2->currentText() + ui->pos3->currentText();
    value.replace('*', '.').replace("G/C", "[GC]");
    if (value.contains(QRegExp("[ATCG]")))
    {
        return value;
    }

    return "";
}

void ThreePrimeInput::setText(const QString &text)
{
    QString padded = text.rightJustified(3, '.', true);
    padded.replace('.', '*');

    ui->pos1->setCurrentIndex(ui->pos1->findText(padded.at(0)));
    ui->pos2->setCurrentIndex(ui->pos2->findText(padded.at(1)));
    ui->pos3->setCurrentIndex(ui->pos3->findText(padded.at(2)));
}
