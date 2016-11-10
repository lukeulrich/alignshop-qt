/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "Gzip.h"
#include "../global.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param source [QIODevice *]
  * @param parent [QObject *]
  */
Gzip::Gzip(QIODevice *source, QObject *parent)
    : QIODevice(parent),
      source_(source),
      compressionLevel_(Z_DEFAULT_COMPRESSION)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool Gzip::atEnd() const
{
    if (QIODevice::openMode() == QIODevice::ReadOnly && source_ != nullptr)
        return source_->atEnd();

    return false;
}

/**
  * @returns qint64
  */
qint64 Gzip::bytesAvailable() const
{
    return inflationBuffer_.size() + QIODevice::bytesAvailable();
}

/**
  */
void Gzip::close()
{
    if (!QIODevice::isOpen())
        return;

    QIODevice::close();

    if (QIODevice::openMode() == QIODevice::ReadOnly)
        inflateEnd(&zStream_);
    else
        deflateEnd(&zStream_);
}

/**
  * It is important to call this method to flush the zlib deflation buffer when all writing is done.
  */
void Gzip::flushWrite()
{
    ASSERT(source_ != nullptr);
    ASSERT(source_->isOpen());
    ASSERT(QIODevice::isOpen());
    ASSERT(QIODevice::openMode() == QIODevice::WriteOnly);

    ASSERT(zStream_.next_in != nullptr);

    // Presumably the next_in has already been set - could have been lost at this point...
    QByteArray deflationBuffer;
    int retval;
    do
    {
        zStream_.avail_out = kChunkSize;
        zStream_.next_out = (uchar *)out_;
        retval = deflate(&zStream_, Z_FINISH);
        deflationBuffer.append(out_, kChunkSize - zStream_.avail_out);
    }
    while (zStream_.avail_out == 0);
    ASSERT(retval == Z_STREAM_END);

    writeDataToSource(deflationBuffer.constData(), deflationBuffer.length());

    deflateEnd(&zStream_);
}

/**
  * @returns bool
  */
bool Gzip::isSequential() const
{
    return true;
}

/**
  * Checks:
  * o We have a valid source
  * o Valid open mode
  * o Not already open
  * o Read and write modes are the same - ignore all others
  *
  * Source mode     Gzip mode   Accept
  * ReadOnly        ReadOnly    Yes
  * ReadOnly        WriteOnly   No
  *
  * WriteOnly       ReadOnly    No
  * WriteOnly       WriteOnly   Yes
  *
  * ReadWrite       ReadOnly    Yes
  * ReadWrite       WriteOnly   Yes
  *
  * @param mode [OpenMode]
  * @returns bool
  */
bool Gzip::open(OpenMode mode)
{
    if (source_ == nullptr)
        return false;

    // This ensures that mode is either ReadOnly or WriteOnly
    if (!isValidOpenMode(mode))
        return false;

    if (!QIODevice::open(mode))
        return false;

    if (source_->isOpen())
    {
        // Test for compatible open mode
        if (mode != source_->openMode() &&                // The open modes are not the same
            source_->openMode() != QIODevice::ReadWrite)  // source mode is either ReadOnly or WriteOnly
        {
            // Incompatible
            QIODevice::close();
            return false;
        }
    }
    // Attempt to open the source with this mode
    else if (!source_->open(mode))
    {
        // Failed to open source
        QIODevice::close();
        return false;
    }

    // ---------------------------------------------------------------------
    // Context: source is open, this device is open and modes are compatible
    // --> Initialize zlib
    zStream_.zalloc = Z_NULL;
    zStream_.zfree = Z_NULL;
    zStream_.opaque = Z_NULL;
    int retval;
    if (mode == QIODevice::ReadOnly)
    {
        // Prepare for decompression / inflation; + 16 indicates that we are using gzip header
        retval = inflateInit2(&zStream_, 15 + 16);
    }
    else
    {
        ASSERT(mode == QIODevice::WriteOnly);

        // Just using the defaults
        retval = deflateInit2(&zStream_, compressionLevel_, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    }

    if (retval != Z_OK)
    {
        setErrorString(zStream_.msg);
        return false;
    }

    return true;
}

/**
  * Must be called before open to take effect.
  *
  * @param level
  */
void Gzip::setCompressionLevel(int level)
{
    ASSERT_X(level >= -1 && level <= 9, "compression level out of range");

    compressionLevel_ = level;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Core decompression / inflation method.
  *
  *
  * @param data [char *]
  * @param maxlen [qint64]
  * @returns qint64
  */
qint64 Gzip::readData(char *data, qint64 maxlen)
{
    // Assume: valid context
    ASSERT(source_->isOpen());
    ASSERT(source_->openMode() & QIODevice::ReadOnly);
    ASSERT(QIODevice::isOpen());

    if (inflationBuffer_.isEmpty())
    {
        // Initially attempt to read initial batch from source
        int sourceBytesRead = source_->read(in_, qMin(static_cast<qint64>(kChunkSize), maxlen));
        if (sourceBytesRead == -1)
        {
            setErrorString("Source device error: " + source_->errorString());
            return -1;
        }

        if (sourceBytesRead == 0)
            return 0;

        zStream_.avail_in = sourceBytesRead;
        zStream_.next_in = (uchar *)in_;
        do
        {
            zStream_.avail_out = kChunkSize;
            zStream_.next_out = (uchar *)out_;
            int retval = inflate(&zStream_, Z_NO_FLUSH);
            ASSERT(retval != Z_STREAM_ERROR);
            switch(retval)
            {
            case Z_NEED_DICT:
                retval = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&zStream_);
                setErrorString(zStream_.msg);
                return -1;
            }
            inflationBuffer_.append(out_, kChunkSize - zStream_.avail_out);
        }
        while (zStream_.avail_out == 0);
    }

    int bytesToReturn = qMin(maxlen, static_cast<qint64>(inflationBuffer_.length()));
    if (bytesToReturn)
    {
        memcpy(data, inflationBuffer_.constData(), bytesToReturn);
        inflationBuffer_ = inflationBuffer_.right(inflationBuffer_.length() - maxlen);
    }
    return bytesToReturn;
}

/**
  * Writing up to len bytes of data to the source_ device as compressed. Typically, will be writing less than len bytes
  * because of the compression; however, this is not necessarily so. For instance, the first write will include the gzip
  * header, which very well could surpass len.
  *
  * @param data [const char *]
  * @param len [qint64]
  * @returns qint64
  */
qint64 Gzip::writeData(const char *data, qint64 len)
{
    zStream_.avail_in = len;
    zStream_.next_in = (uchar *)data;

    QByteArray deflationBuffer;
    do
    {
        zStream_.avail_out = kChunkSize;
        zStream_.next_out = (uchar *)out_;
        deflate(&zStream_, Z_NO_FLUSH);
        deflationBuffer.append(out_, kChunkSize - zStream_.avail_out);
    }
    while (zStream_.avail_out == 0);

    if (!writeDataToSource(deflationBuffer.constData(), deflationBuffer.length()))
        return -1;

    return len;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param mode [QIODevice::OpenMode]
  * @returns bool
  */
bool Gzip::isValidOpenMode(QIODevice::OpenMode mode) const
{
    return mode == QIODevice::ReadOnly ||
           mode == QIODevice::WriteOnly;
}

/**
  * @param data [const char *]
  * @param length [int]
  * @returns bool
  */
bool Gzip::writeDataToSource(const char *data, int length)
{
    ASSERT(source_ != nullptr);

    int bytesWritten = 0;
    while (bytesWritten != length)
    {
        bytesWritten = source_->write(data + bytesWritten, length - bytesWritten);
        if (bytesWritten == -1)
            return false;
    }

    return true;
}
