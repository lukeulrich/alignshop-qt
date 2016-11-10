/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef Q3PREDICTION_H
#define Q3PREDICTION_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <QtDebug>

const int kDecimalPrecision = 3;
const int kDoubleStringLength = kDecimalPrecision + 1;    // To include the decimal character

/**
  * Secondary structure Q3 prediction and confidence data
  */
struct Q3Prediction
{
    QByteArray q3_;
    QVector<double> confidence_;

    bool operator==(const Q3Prediction &other) const
    {
        return q3_ == other.q3_ &&
                confidence_ == other.confidence_;
    }

    bool isEmpty() const
    {
        return q3_.isEmpty();
    }

    // Returns the confidence values as a comma separated list with precision to the thousandth place:
    // .123,.345,.567, ...
    QString encodeConfidence() const
    {
        if (confidence_.isEmpty())
            return QString();

        QString string;
        string.reserve(kDoubleStringLength * confidence_.size() - 1);
        string = QString::number(confidence_.at(0), 'f', kDecimalPrecision);

        for (int i=1, z=confidence_.size(); i<z; ++i)
            string += ',' + QString::number(confidence_.at(i), 'f', kDecimalPrecision);

        qDebug() << string;

        return string;
    }

    // Decodes string into a vector of doubles
    static QVector<double> decodeConfidence(const QString &string)
    {
        QStringList doubleList = string.split(',');
        QVector<double> confidence;
        confidence.reserve(confidence.size());

        foreach (const QString &doubleString, doubleList)
        {
            bool ok = true;
            double value = doubleString.toDouble(&ok);
            if (ok)
                confidence << value;
            else
                throw "Unable to convert string to double";
        }

        return confidence;
    }
};

Q_DECLARE_TYPEINFO(Q3Prediction, Q_PRIMITIVE_TYPE);

#endif // Q3PREDICTION_H
