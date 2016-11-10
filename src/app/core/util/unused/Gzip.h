/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GZIP_H
#define GZIP_H

#include <QtCore/QByteArray>
#include <QtCore/QIODevice>
#include "zlib.h"

/**
  * Gzip wraps a source QIODevice and provides decompression (inflation) when reading and compression (deflation) when
  * writing.
  *
  * May only be opened in the ReadOnly mode or WriteOnly mode.
  *
  * Not certain this class is useful
  */
class Gzip : public QIODevice
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit Gzip(QIODevice *source, QObject *parent = 0);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual bool atEnd() const;                                 //!< Returns true if the openMode is ReadOnly and the source device is
    virtual qint64 bytesAvailable() const;
    virtual void close();
    void flushWrite();                                          //!< Flushes any compressed data to be written to the source device
    virtual bool isSequential() const;
    virtual bool open(OpenMode mode);
    void setCompressionLevel(int level);


protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented protected methods
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    bool isValidOpenMode(QIODevice::OpenMode mode) const;
    bool writeDataToSource(const char *data, int length);

    static const int kChunkSize = 16384;

    QIODevice *source_;
    z_stream zStream_;
    int compressionLevel_;
    char in_[kChunkSize];
    char out_[kChunkSize];
    QByteArray inflationBuffer_;       // To accommodate inflated data
};



/* Example code:

  // --------------------------------------
  // Inflating gzip'ed file to another file

    QFile file("file.svg");
    QFile gzFile("file.svg.gz");
    Gzip gzip(&gzFile);

    // Example of inflating gzip file
    gzip.open(QIODevice::ReadOnly);
    file.open(QIODevice::WriteOnly);
    char buffer[16384];
    while (!gzip.atEnd())
    {
        int bytesRead = gzip.read(buffer, 16384);
        int bytesWritten = 0;
        while (bytesWritten != bytesRead)
            bytesWritten += file.write(buffer + bytesWritten, bytesRead - bytesWritten);
    }

    gzip.close();
    file.close();


  // --------------------------------------
  // Deflating regular file to gzip

    QFile file("file.svg");
    QFile gzFile("file.svg.gz");
    Gzip gzip(&gzFile);

    gzip.open(QIODevice::WriteOnly);
    file.open(QIODevice::ReadOnly);
    while (!file.atEnd())
        gzip.write(file.readLine());
    gzip.flushWrite();

    gzip.close();
    file.close();

*/

#endif // GZIP_H
