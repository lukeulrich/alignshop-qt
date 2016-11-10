/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MULTILINEDELEGATE_H
#define MULTILINEDELEGATE_H

#include <QtGui/QFontMetrics>
#include <QtGui/QColorGroup>
#include <QtGui/QPalette>

#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>

class MultiLineDelegate : public QStyledItemDelegate
{
public:
    explicit MultiLineDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    /**
      * Reference help: http://stackoverflow.com/questions/5334590/using-a-qstyleditemdelegate-on-a-qlistview-with-qsqlquerymodel
      */
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);

        opt.text = "";
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

        QPalette::ColorGroup colorGroup = opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
        if (colorGroup == QPalette::Normal && !(opt.state & QStyle::State_Active))
            colorGroup = QPalette::Inactive;

        // set pen color
        if (opt.state & QStyle::State_Selected)
            painter->setPen(opt.palette.color(colorGroup, QPalette::HighlightedText));
        else
            painter->setPen(opt.palette.color(colorGroup, QPalette::Text));


        QList<QByteArray> lines = index.data().toByteArray().split('\n');

        painter->save();
        QFont font = qvariant_cast<QFont>(index.data(Qt::FontRole));
        painter->setFont(font);
        painter->setClipRect(option.rect);
        QFontMetrics fm(font);

        int totalHeight = fm.height() * lines.size();

        int y = option.rect.top() + (option.rect.height() - totalHeight) / 2.;
        foreach (const QByteArray &line, lines)
        {
            painter->drawText(QRect(option.rect.left(), y, option.rect.width(), fm.height()),
                              opt.displayAlignment, line);
            y += fm.height();
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);
        QFont font = qvariant_cast<QFont>(index.data(Qt::FontRole));
        QFontMetrics fm(font);
        QList<QByteArray> lines = index.data().toByteArray().split('\n');
        return QSize(fm.width(lines.at(0)), fm.height() * lines.size());
    }
};

#endif // MULTILINEDELEGATE_H
