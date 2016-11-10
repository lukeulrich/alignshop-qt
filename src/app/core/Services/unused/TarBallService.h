/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TARBALLSERVICE_H
#define TARBALLSERVICE_H

#include <QtCore/QObject>
#include <QtCore/QString>

class QDir;

class TarBallService : public QObject
{
    Q_OBJECT
public:
    explicit TarBallService(QObject *parent = 0);

public Q_SLOTS:
    void cancel();
    void inflateExtract(const QString &file, const QDir &directory);

Q_SIGNALS:
    void error(const QString &error);
    void finished();
    void progressChanged(int currentStep, int totalSteps);

private:
    bool canceled_;
};

#endif // TARBALLSERVICE_H
