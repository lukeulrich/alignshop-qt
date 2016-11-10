/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BASICAPPLICATION_H
#define BASICAPPLICATION_H

#include <QtGui/QApplication>

class QEvent;

class BasicApplication : public QApplication
{
    Q_OBJECT

public:
    BasicApplication(int &argc, char **argv);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public functions
    bool notify(QObject *receiver, QEvent *event);
};

#endif // BASICAPPLICATION_H
