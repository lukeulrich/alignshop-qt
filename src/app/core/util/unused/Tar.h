/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef TAR_H
#define TAR_H

#include <QtCore/QObject>

class QDir;
class QIODevice;

/**
  *
  *
  * Progress signals for extract are 0, 0 if device is sequential; otherwise, it is the number of bytes processed and
  * total bytes available.
  *
  * Progress signal for archive is per-file.
  *
  * For extract, device must be in read mode. Archive, device must be in write mode
  * On archive, file must exist or an exception is thrown; similarly, directory must exist and be writable for extract
  * or an exception is thrown.
  */
class Tar : public QObject
{
    Q_OBJECT

public:
    explicit Tar(QObject *parent = 0);

    void extract(QIODevice *device, const QDir &directory); //!< Extracts all tar-encoded files in device to directory
    void archive(QIODevice *device, const QString &file);   //!< Archives file as a tar-encoded file into device

Q_SIGNALS:
    void progressChanged(int currentStep, int totalSteps);

private:
    QByteArray readTarHeader(QIODevice *device) const;      //!< Returns a 512 byte TAR file header from device or empty if an error occurs
    QByteArray tarHeader(const QString &file) const;        //!< Returns a 512 byte TAR file header for file or empty if an error occurs
};

#endif // TAR_H
