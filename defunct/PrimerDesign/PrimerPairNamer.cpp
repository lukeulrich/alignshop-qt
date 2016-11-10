#include "PrimerPairNamer.h"

using namespace PrimerDesign;

PrimerPairNamer::PrimerPairNamer(const QString &seqName, ObservablePrimerPairList *list)
{
    QString randomCharAToZ = QChar((qrand() % 26) + 'a');
    prefix_ = randomCharAToZ + "_" + seqName + "_";
    suffix_ = list->length();

    for (int i = 0; i < list->length(); ++i)
    {
        names_ << list->at(i)->name();
    }
}

QString PrimerPairNamer::nextName()
{
    QString name = prefix_;

    do
    {
        name = prefix_ + QString::number(suffix_++);
    }
    while (names_.contains(name));

    return name;
}
