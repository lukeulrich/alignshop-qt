/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef RESTRICTIONENZYMEACCENTDELEGATE_H
#define RESTRICTIONENZYMEACCENTDELEGATE_H

#include <QtGui/QStyledItemDelegate>

class RestrictionEnzymeAccentDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RestrictionEnzymeAccentDelegate(int recognitionSiteColumn, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


private:
    QByteArray recognitionSite(const QModelIndex &siblingIndex) const;
    QModelIndex recognitionSiteIndex(const QModelIndex &siblingIndex) const;

    int recognitionSiteColumn_;
};


#endif // RESTRICTIONENZYMEACCENTDELEGATE_H
