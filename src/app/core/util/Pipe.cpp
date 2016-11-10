/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Pipe.h"
#include "../global.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param target [QIODevice *]
  * @param parent [QObject *]
  */
Pipe::Pipe(QIODevice *target, QObject *parent)
    : QIODevice(parent),
      target_(target)
{
    open(QIODevice::WriteOnly);
}

/**
  */
Pipe::~Pipe()
{
    close();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool Pipe::isSequential() const
{
    return true;
}

/**
  * @returns QIODevice *
  */
QIODevice *Pipe::target() const
{
    return target_;
}

/**
  * @param target [QIODevice *]
  */
void Pipe::setTarget(QIODevice *target)
{
    target_ = target;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  */
void Pipe::close()
{
    QIODevice::close();
}

/**
  * @param mode [OpenMode]
  * @returns bool
  */
bool Pipe::open(OpenMode mode)
{
    ASSERT(mode == QIODevice::WriteOnly);
    return QIODevice::open(mode);
}

/**
  * @param data [const char *]
  * @param len [qint64]
  * @param buffer [QByteArray &]
  * @returns bool
  */
bool Pipe::processData(const char * /* data*/ , qint64 /* len */, QByteArray & /* buffer */)
{
    return true;
}

/**
  * @param data [char *]
  * @param maxlen [qint64]
  * @returns qint64
  */
qint64 Pipe::readData(char * /* data */, qint64 /* maxlen */)
{
    setErrorString("Reading not allowed for pipes");
    return -1;
}

/**
  * By default, simply write all data straight through to the target.
  *
  * @param data [const char *]
  * @param len [qint64]
  * @returns qint64
  */
qint64 Pipe::writeData(const char *data, qint64 len)
{
    if (target_ == nullptr)
        return len;

    // Allow subclasses to manipulate the data
    QByteArray processedData;
    if (processData(data, len, processedData) &&
        writeDataToTarget(processedData))
    {
        return len;
    }

    return -1;
}

/**
  * Convenience method
  *
  * @param data [const QByteArray &]
  * @returns bool
  */
bool Pipe::writeDataToTarget(const QByteArray &data)
{
    return writeDataToTarget(data.constData(), data.length());
}

/**
  * @param data [const char *]
  * @param length [int]
  * @returns bool
  */
bool Pipe::writeDataToTarget(const char *data, int length)
{
    ASSERT(target_ != nullptr);

    int totalBytesWritten = 0;
    while (totalBytesWritten != length)
    {
        int bytesWritten = target_->write(data + totalBytesWritten, length - totalBytesWritten);
        if (bytesWritten == -1)
        {
            setErrorString("Target write error: " + target_->errorString());
            return false;
        }
        totalBytesWritten += bytesWritten;
    }

    return true;
}
