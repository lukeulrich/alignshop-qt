#include "ObservableSequence.h"

using namespace PrimerDesign;

ObservableSequence::ObservableSequence(QObject *parent)
    : QObject(parent)
{
}

bool ObservableSequence::setName(const QString &name)
{
    if (DnaSequence::setName(name))
    {
        emit nameChanged(this);
        return true;
    }

    return false;
}
