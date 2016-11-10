#include "DnaSequenceValidator.h"

DnaSequenceValidator::DnaSequenceValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State DnaSequenceValidator::validate(QString &value, int & /* position */) const
{
    QRegExp invalidCharacters("[^ACGT]", Qt::CaseInsensitive);
    fixup(value);

    return value.contains(invalidCharacters) ? Invalid : Acceptable;
}

void DnaSequenceValidator::fixup(QString &value) const
{
    value = value.toUpper();
    QRegExp blanks("\\s*");
    value = value.replace(blanks, "");
}
