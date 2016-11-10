/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTFEAUREITEM_H
#define ABSTRACTFEAUREITEM_H

#include "../gitems/AbstractLinearItem.h"
#include "../../../core/global.h"

#include <QtDebug>

class AbstractFeatureItem : public AbstractLinearItem
{
    Q_OBJECT

public:
    int type() const
    {
        return type_;
    }

    int id() const
    {
        return id_;
    }

    virtual void highlight()
    {
    }

    virtual void clearHighlight()
    {
    }


protected:
    AbstractFeatureItem(const int type, const int id, const int length, QGraphicsItem *parentItem = nullptr)
        : AbstractLinearItem(length, parentItem),
          type_(type),
          id_(id)
    {
#ifdef QT_DEBUG
        if (type_ < QGraphicsItem::UserType)
            qWarning("Creating feature item with restricted user type [AbstractFeatureItem]");
#endif
    }


private:
    int type_;
    int id_;
};


#endif // ABSTRACTFEAUREITEM_H
