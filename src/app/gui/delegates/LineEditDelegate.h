/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QtGui/QStyledItemDelegate>

/**
  * LineEditDelegate provides a more user-friendly interface for directly editing a model's Qt::DisplayRole via a LineEdit
  * control.
  *
  * The default line edit delegate creates a new line editor with just a blank line. LineEditDelegate enhances this
  * by pre-populating the LineEdit with the entry text and pre-selecting all the data.
  * [UNTESTED]
  */
class LineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct an instance of this delegate for column column of a given model
    explicit LineEditDelegate(QObject *parent = 0);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    //! Returns the widget used to edit the item specified by index for editing
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //! Sets the data to be displayed by the editor from the data model item specified by index
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    //! Paints the items normally if not cut
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // LINEEDITDELEGATE_H
