/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTPRIMERITEM_H
#define ABSTRACTPRIMERITEM_H

#include <QtCore/QString>

#include "AbstractRangeFeatureItem.h"
#include "../../../core/util/ClosedIntRange.h"

class AbstractPrimerItem : public AbstractRangeFeatureItem
{
    Q_OBJECT

public:
    int primerId() const;
    QString name() const;
    double height() const;
    void setName(const QString &name);


protected:
    AbstractPrimerItem(const int primerId, const QString &name, const ClosedIntRange &location, QGraphicsItem *parentItem);

    double arrowLegLength() const;
    QPen arrowPen() const;
    QLineF midLine() const;

private:
    int primerId_;
    QString name_;
};

#endif // ABSTRACTPRIMERITEM_H
