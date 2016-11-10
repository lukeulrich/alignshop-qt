/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef THREEPRIMEINPUT_H
#define THREEPRIMEINPUT_H

#include <QtGui/QWidget>
#include <QtGui/QComboBox>

#include "../core/DnaPattern.h"


/// Represents the 3' input for a primer.
class ThreePrimeInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(DnaPattern dnaPattern READ dnaPattern)
    Q_PROPERTY(QString text READ text)

public:
    /// Constructs a new instance.
    /// @param parent QWidget * (Defaults to 0.)
    explicit ThreePrimeInput(QWidget *parent = 0);

    /// Gets the string-representation of the 3' input.
    /// @return QString
    DnaPattern dnaPattern() const;

    /// Sets the value of the 3' input.
    /// @param text const QString &
    /// @return void
    void setPattern(const DnaPattern &dnaPattern);      // Only accepts the first three letters of dnaPattern

    QString text() const;       // Returns the textual representation of all three combo boxes

private:
    QComboBox *createSpecificationComboBox() const;

    QComboBox *comboBox1_;
    QComboBox *comboBox2_;
    QComboBox *comboBox3_;
};

#endif // THREEPRIMEINPUT_H
