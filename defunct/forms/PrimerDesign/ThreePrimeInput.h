/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef THREEPRIMEINPUT_H
#define THREEPRIMEINPUT_H

#include <QtCore>
#include <QWidget>

namespace Ui {
    class ThreePrimeInput;
}

namespace PrimerDesign
{
    /// Represents the 3' input for a primer.
    class ThreePrimeInput : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructs a new instance.
        /// @param parent QWidget * (Defaults to 0.)
        explicit ThreePrimeInput(QWidget *parent = 0);

        /// The destructor.
        ~ThreePrimeInput();

        /// Gets the string-representation of the 3' input.
        /// @return QString
        QString text() const;

        /// Sets the value of the 3' input.
        /// @param text const QString &
        /// @return void
        void setText(const QString &text);

    private:
        Ui::ThreePrimeInput *ui;
    };
}

#endif // THREEPRIMEINPUT_H
