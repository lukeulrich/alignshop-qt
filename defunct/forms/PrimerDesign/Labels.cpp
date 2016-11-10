#include "Labels.h"

using namespace PrimerDesign;

HeaderLabel::HeaderLabel(QWidget *parent)
    : QLabel(parent)
{
}

ValidatorLabel::ValidatorLabel(QWidget *parent)
    : QLabel(parent)
{
}

void ValidatorLabel::setText(const QString &value)
{
    this->originalText_ = value;
    QLabel::setText(value);
}

void ValidatorLabel::setError(const QString &value)
{
    QLabel::setText("<img src=':/TriangularError' /> " + value);
}

void ValidatorLabel::clearError()
{
    setText(originalText_);
}
