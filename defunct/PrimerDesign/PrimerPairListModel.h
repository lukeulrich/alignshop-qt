/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef OBSERVABLEPRIMERPAIRLIST_H
#define OBSERVABLEPRIMERPAIRLIST_H

#include <QtCore>
#include <QtGui>
#include <PrimerDesign/PrimerPair.h>
#include <PrimerDesign/ListModel.h>

namespace PrimerDesign
{
    /// The model for a priper pair list.
    class PrimerPairListModel : public ListModel
    {
        Q_OBJECT

    public:
        /// The default constructor.
        /// @param isCompact bool (Defaults to false.)
        PrimerPairListModel(bool isCompact = false);

        /// Gets the specified data.
        /// @param index const QModelIndex &
        /// @param role int (Defaults to Qt::DisplayRole.)
        /// @return QVariant
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        /// Sets the specified data.
        /// @param index const QModelIndex &
        /// @param value const QVariant &
        /// @param role int (Defaults to Qt::EditRole.)
        /// @return bool
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private:
        bool isCompact_;

        /// Gets the textual representation of a number.
        /// @param value double
        /// @return QString
        static QString number(double value);

        /// Gets the column text of the specified string.
        /// @param value const QString &
        /// @return QString
        static QString toColumnText(const QString &value);

        /// Gets the primer summary text.
        /// @param pair PrimerPair *
        /// @return QString
        static QString primerSummary(PrimerPair *pair);

        /// Gets the tm summary text
        /// @param pair PrimerPair *
        /// @return QString
        static QString tmSummary(PrimerPair *pair);

        /// Gets the delta tm text
        /// @param pair PrimerPair *
        /// @return QString
        static QString deltaTm(PrimerPair *pair);

        /// Gets the compact value for the specified pair, row, and column
        /// @param pair PrimerPair *
        /// @param row int
        /// @param col int
        /// @return QVariant
        static QVariant getCompactColValue(PrimerPair *pair, int row, int col);

        /// Gets the non-compact value for the specified pair and column
        /// @param pair PrimerPair *
        /// @param col int
        /// @return QVariant
        static QVariant getNonCompactColValue(PrimerPair *pair, int col);
    };
}

#endif // OBSERVABLEPRIMERPAIRLIST_H
