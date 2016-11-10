/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "PrimerPairListModel.h"

using namespace PrimerDesign;

namespace Col
{
    enum PrimerPair
    {
        NAME,
        FULL_PRIMER_PAIR,
        FULL_TM,
        FULL_DELTA_TM,
        FULL_AMPLICON_LENGTH
    };

    enum CompactPrimerPair
    {
        ID,
        PRIMER_PAIR,
        TM,
        DELTA_TM,
        SCORE,
        COMPACT_AMPLICON_LENGTH
    };
}

PrimerPairListModel::PrimerPairListModel(bool isCompact)
    : ListModel(
          1,
          isCompact ?
              QList<QString>() << "" << "Primer Pair" << "Tm" << QString::fromUtf8("Δ Tm") << "Score" << "Amplicon Length" :
              QList<QString>() << "Name" << "Primer Pair" << "Tm" << QString::fromUtf8("Δ Tm") << "Amplicon Length")
{
    isCompact_ = isCompact;
}

QVariant PrimerPairListModel::data(const QModelIndex &index, int role) const
{
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && list())
    {
        ObservablePrimerPairList *pairs = (ObservablePrimerPairList*)list();
        PrimerPair *pair = pairs->at(index.row());

        if (isCompact_)
        {
            return getCompactColValue(pair, index.row(), index.column());
        }
        else
        {
            return getNonCompactColValue(pair, index.column());
        }
    }
    else if (role == Qt::FontRole &&
        ((isCompact_ && (index.column() == Col::PRIMER_PAIR || index.column() == Col::TM)) ||
         (!isCompact_ && (index.column() == Col::FULL_PRIMER_PAIR || index.column() == Col::FULL_TM))))
    {
        return QFont("Cousine", 9);
    }

    return QVariant();
}

QString PrimerPairListModel::toColumnText(const QString &value)
{
    const int COLUMN_WIDTH = 15;
    QString clone = value;
    clone.truncate(COLUMN_WIDTH);
    return value;
}

QString PrimerPairListModel::number(double value)
{
    return QString::number(value, 'f', 1);
}

QString PrimerPairListModel::primerSummary(PrimerPair *pair)
{
    return "F: " + toColumnText(pair->forwardPrimer().sequence()) +
            "\nR: " + toColumnText(pair->reversePrimer().sequence());
}

QString PrimerPairListModel::tmSummary(PrimerPair *pair)
{
    return number(pair->forwardPrimer().tm()) +
            "\n" + number(pair->reversePrimer().tm());
}

QString PrimerPairListModel::deltaTm(PrimerPair *pair)
{
    return number(qAbs(pair->forwardPrimer().tm() - pair->reversePrimer().tm()));
}

QVariant PrimerPairListModel::getCompactColValue(PrimerPair *pair, int row, int col)
{
    switch (col)
    {
        case Col::ID:
            return QVariant(QString::number(row + 1) + ".");
        case Col::PRIMER_PAIR:
            return QVariant(primerSummary(pair));
        case Col::TM:
            return QVariant(tmSummary(pair));
        case Col::DELTA_TM:
            return QVariant(deltaTm(pair));
        case Col::SCORE:
            return QVariant(number(100 - pair->score()));
        case Col::COMPACT_AMPLICON_LENGTH:
            return QVariant(pair->ampliconLength());
        default:
            return QVariant();
    }
}

QVariant PrimerPairListModel::getNonCompactColValue(PrimerPair *pair, int col)
{
    switch (col)
    {
        case Col::NAME:
            return QVariant(pair->name());
        case Col::FULL_PRIMER_PAIR:
            return QVariant(primerSummary(pair));
        case Col::FULL_TM:
            return QVariant(tmSummary(pair));
        case Col::FULL_AMPLICON_LENGTH:
            return QVariant(pair->ampliconLength());
        case Col::FULL_DELTA_TM:
            return QVariant(deltaTm(pair));
        default:
            return QVariant();
    }
}

bool PrimerPairListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && list())
    {
        ObservablePrimerPairList *pairs = (ObservablePrimerPairList*)list();
        PrimerPair *pair = pairs->at(index.row());
        switch (index.column())
        {
            case Col::NAME:
                return pair->setName(value.toString());
            default:
                return false;
        }
    }
    else
    {
        return false;
    }
}
