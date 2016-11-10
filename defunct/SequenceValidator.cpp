#include "SequenceValidator.h"

SequenceValidator::SequenceValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State SequenceValidator::validate(QString &value, int & /* position */) const
{
    QRegExp invalidCharacters("[^ACGT]", Qt::CaseInsensitive);
    fixup(value);

    return value.contains(invalidCharacters) ? Invalid : Acceptable;
}

void SequenceValidator::fixup(QString &value) const
{
    QRegExp blanks("\\s*");
    value = value.replace(blanks, "");
}
