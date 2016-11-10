/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef REGEXDELEGATE_H
#define REGEXDELEGATE_H

#include <QtGui/QLineEdit>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QRegExpValidator>
#include "../../core/global.h"

class RegexDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RegexDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent),
          validator_(nullptr)
    {
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setValidator(validator_);
        return lineEdit;
    }

    void setRegExp(const QRegExp &regExp)
    {
        if (regExp.isEmpty())
        {
            delete validator_;
            validator_ = nullptr;
            return;
        }
        else if (validator_ == nullptr)
        {
            validator_ = new QRegExpValidator(this);
        }
        validator_->setRegExp(regExp);
    }


private:
    QRegExpValidator *validator_;
};

#endif // REGEXDELEGATE_H
