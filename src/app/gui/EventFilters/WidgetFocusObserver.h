/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef WIDGETFOCUSOBSERVER_H
#define WIDGETFOCUSOBSERVER_H

#include <QtCore/QObject>
#include "../../core/global.h"

class QWidget;

/**
  * Intercepts focus events and emits corresponding signals
  */
class WidgetFocusObserver : public QObject
{
    Q_OBJECT

public:
    WidgetFocusObserver(QWidget *target, QObject *parent = nullptr);

Q_SIGNALS:
    void focusLost(Qt::FocusReason reason);
    void focusGained(Qt::FocusReason reason);

protected:
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // WIDGETFOCUSOBSERVER_H
