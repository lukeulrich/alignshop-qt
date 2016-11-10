/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef VALIDATORLABEL_H
#define VALIDATORLABEL_H

#include <QLabel>

namespace PrimerDesign
{
    /// A label used as a header for major UI sections.
    class HeaderLabel : public QLabel
    {
    public:
        /// The constructor.
        /// @param parent QWidget *
        HeaderLabel(QWidget *parent);
    };

    /// A label used to represent label data or validation errors.
    class ValidatorLabel : public QLabel
    {
    public:
        /// The constructor.
        /// @param parent QWidget *
        ValidatorLabel(QWidget *parent);

        /// Sets the label's text.
        /// @param value const QString &
        /// @return void
        void setText(const QString &value);

        /// Sets the label's error text.
        /// @param value const QString &
        /// @return void
        void setError(const QString &value);

        /// Clears the error text.
        /// @return void
        void clearError();

    private:
        QString originalText_;
    };
}

#endif // VALIDATORLABEL_H
