/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "BasicApplication.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param argc [int &]
  * @param argv [char **]
  */
BasicApplication::BasicApplication(int &argc, char **argv)
    : QApplication(argc, argv)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param receiver [QObject *]
  * @param event [QEvent *]
  * @returns bool
  */
bool BasicApplication::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (QString &errorMessage)
    {
        qDebug() << "Unhandled exception:" << errorMessage;
    }
    catch (const char *message)
    {
        qDebug() << "Unhandled exception:" << message;
    }
    catch (const int number)
    {
        qDebug() << "Unhandled exception:" << number;
    }
    catch (...)
    {
        qDebug() << "Unhandled exception: unknown";
    }

    return false;
}
