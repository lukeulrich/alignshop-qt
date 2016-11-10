#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>

#include "ThreePrimeInput.h"
#include "../core/DnaPattern.h"

ThreePrimeInput::ThreePrimeInput(QWidget *parent) :
    QWidget(parent)
{
    comboBox1_ = createSpecificationComboBox();
    comboBox2_ = createSpecificationComboBox();
    comboBox3_ = createSpecificationComboBox();

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(comboBox1_);
    hLayout->addWidget(comboBox2_);
    hLayout->addWidget(comboBox3_);

    // Since this widget is really the combination of three comboboxes acting as a single widget inside a layout, strip
    // away the default layout content margins.
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
}

DnaPattern ThreePrimeInput::dnaPattern() const
{
    QByteArray patternArray(3, 0);
    patternArray[0] = comboBox1_->itemData(comboBox1_->currentIndex()).toChar().toLatin1();
    patternArray[1] = comboBox2_->itemData(comboBox2_->currentIndex()).toChar().toLatin1();
    patternArray[2] = comboBox3_->itemData(comboBox3_->currentIndex()).toChar().toLatin1();
    if (patternArray == "NNN")
        return DnaPattern();

    return DnaPattern(patternArray);
}

void ThreePrimeInput::setPattern(const DnaPattern &dnaPattern)
{
    // Reset the combo boxes to N's
    comboBox1_->setCurrentIndex(0);
    comboBox2_->setCurrentIndex(0);
    comboBox3_->setCurrentIndex(0);

    if (!dnaPattern.isValid())
        return;

    if (dnaPattern.length() >= 3)
    {
        int index = comboBox1_->findData(dnaPattern.pattern().at(0));
        if (index != -1)
            comboBox1_->setCurrentIndex(index);
        index = comboBox2_->findData(dnaPattern.pattern().at(1));
        if (index != -1)
            comboBox2_->setCurrentIndex(index);
        index = comboBox3_->findData(dnaPattern.pattern().at(2));
        if (index != -1)
            comboBox3_->setCurrentIndex(index);
    }
    else if (dnaPattern.length() == 2)
    {
        int index = comboBox2_->findData(dnaPattern.pattern().at(0));
        if (index != -1)
            comboBox2_->setCurrentIndex(index);
        index = comboBox3_->findData(dnaPattern.pattern().at(1));
        if (index != -1)
            comboBox3_->setCurrentIndex(index);
    }
    else if (dnaPattern.length() == 1)
    {
        int index = comboBox3_->findData(dnaPattern.pattern().at(0));
        if (index != -1)
            comboBox3_->setCurrentIndex(index);
    }
}

/**
  * @returns QString
  */
QString ThreePrimeInput::text() const
{
    QString result;
    QVector<QComboBox *> comboBoxes;
    comboBoxes << comboBox1_ << comboBox2_ << comboBox3_;
    foreach (QComboBox *comboBox, comboBoxes)
    {
        if (comboBox->currentText() == "C/G")
            result += "[C/G]";
        else
            result += comboBox->currentText();
    }

    return result;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @returns QComboBox
  */
QComboBox *ThreePrimeInput::createSpecificationComboBox() const
{
    QComboBox *comboBox = new QComboBox;
    // Map user symbols to DnaPattern symbols
    comboBox->addItem("*", 'N');
    comboBox->addItem("C/G", 'S');
    comboBox->addItem("A", 'A');
    comboBox->addItem("C", 'C');
    comboBox->addItem("G", 'G');
    comboBox->addItem("T", 'T');
    return comboBox;
}
