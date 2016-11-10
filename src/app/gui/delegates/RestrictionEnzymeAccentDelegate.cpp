/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include "RestrictionEnzymeAccentDelegate.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
RestrictionEnzymeAccentDelegate::RestrictionEnzymeAccentDelegate(int recognitionSiteColumn, QObject *parent)
    : QStyledItemDelegate(parent),
      recognitionSiteColumn_(recognitionSiteColumn)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void RestrictionEnzymeAccentDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QByteArray fullPrimerSequence = index.data().toByteArray();
    QByteArray reSiteSequence = recognitionSite(index);
    if (!fullPrimerSequence.startsWith(reSiteSequence))
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItemV4 optionV4 = option;
    initStyleOption(&optionV4, index);

    // Draw the correct background (if selected, etc.)
    optionV4.text = "";
    QStyle *style = optionV4.widget ? optionV4.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter, optionV4.widget);

    // Draw the recognition site
    QRect reSiteBoundingRect;
    QPen oldPen = painter->pen();
    painter->setPen(QColor(196, 0, 0));
    painter->drawText(option.rect, option.displayAlignment, reSiteSequence, &reSiteBoundingRect);
    painter->setPen(oldPen);

    // Finally draw the rest of the core sequence
    QRect coreSequenceOptionRect = option.rect;
    coreSequenceOptionRect.setLeft(option.rect.left() + reSiteBoundingRect.width());
    painter->drawText(coreSequenceOptionRect, option.displayAlignment, fullPrimerSequence.mid(reSiteSequence.length()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
QByteArray RestrictionEnzymeAccentDelegate::recognitionSite(const QModelIndex &siblingIndex) const
{
    return recognitionSiteIndex(siblingIndex).data().toByteArray();
}

QModelIndex RestrictionEnzymeAccentDelegate::recognitionSiteIndex(const QModelIndex &siblingIndex) const
{
    return siblingIndex.sibling(siblingIndex.row(), recognitionSiteColumn_);
}
