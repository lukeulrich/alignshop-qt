/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef RESTRICTIONENZYMETEXTBOX_H
#define RESTRICTIONENZYMETEXTBOX_H

#include <QtGui>

namespace PrimerDesign
{
    /// Represents a restriction enzyme.
    class RestrictionEnzyme
    {
    public:
        /// The name.
        QString name;
        /// The value.
        QString value;
        /// The behavior (sticky, blunt, etc)
        QString behavior;
    };

    /// Represents the input control for a primer finder's restriction enzyme.
    class RestrictionEnzymeTextbox : public QLineEdit
    {
        Q_OBJECT

    public:
        /// The constructor.
        /// @param parent QWidget *
        RestrictionEnzymeTextbox(QWidget *parent);

        /// Gets the restriction enzyme.
        /// @return RestrictionEnzyme
        RestrictionEnzyme restrictionEnzyme() const;

        /// Sets the restriction enzyme.
        /// @param enzyme const RestrictionEnzyme &
        /// @return void
        void setRestrictionEnzyme(const RestrictionEnzyme &enzyme);

    private:
        RestrictionEnzyme enzyme_;

        /// The control got focus.
        /// @param e QFocusEvent *
        /// @return void
        void focusInEvent(QFocusEvent *e);

        /// The control lost focus.
        /// @param e QFocusEvent *
        /// @return void
        void focusOutEvent(QFocusEvent *e);

    private slots:
        /// Triggered when the text changes.
        /// @return void
        void onTextChanged();

        /// Triggered when the name should be shown.
        /// @return void
        void showName();
    };
}

#endif // RESTRICTIONENZYMETEXTBOX_H
