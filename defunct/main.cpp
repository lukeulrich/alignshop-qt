/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QList>
#include <QtCore/QLocale>

#include <QtGui/QApplication>

#include "MainWindow.h"
#include "forms/PrimerDesign/SequenceView.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    QWidget *window;
    QFontDatabase::addApplicationFont(":/fonts/Cousine-Regular-Latin.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Cabin-Regular.ttf");

    if (a.arguments().contains("p", Qt::CaseInsensitive))
    {
        window = new SequenceView(0);
    }
    else
    {
        window = new MainWindow(0);
    }

    window->show();
    int exitCode = a.exec();
    delete window;
    return exitCode;
}
