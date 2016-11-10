/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef RESTRICTIONENZYMELINEEDIT_H
#define RESTRICTIONENZYMELINEEDIT_H

#include <QtCore/QString>
#include <QtGui/QLineEdit>
#include "RestrictionEnzyme.h"

/// Represents the input control for a primer finder's restriction enzyme.
class RestrictionEnzymeLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(RestrictionEnzyme restrictionEnzyme READ restrictionEnzyme WRITE setRestrictionEnzyme)

public:
    /// The constructor.
    /// @param parent QWidget *
    RestrictionEnzymeLineEdit(QWidget *parent = 0);

    /// Gets the restriction enzyme.
    /// @return RestrictionEnzyme
    RestrictionEnzyme restrictionEnzyme() const;

    /// Sets the restriction enzyme.
    /// @param enzyme const RestrictionEnzyme &
    /// @return void
    void setRestrictionEnzyme(const RestrictionEnzyme &enzyme);


private:
    RestrictionEnzyme enzyme_;
    QString manualSite_;

    /// The control got focus.
    /// @param e QFocusEvent *
    /// @return void
    void focusInEvent(QFocusEvent *e);

    /// The control lost focus.
    /// @param e QFocusEvent *
    /// @return void
    void focusOutEvent(QFocusEvent *e);


private Q_SLOTS:
    /// Triggered when the text changes.
    /// @return void
    void onTextChanged();

    /// Triggered when the name should be shown.
    /// @return void
    void showName();
};

#endif // RESTRICTIONENZYMELINEEDIT_H
