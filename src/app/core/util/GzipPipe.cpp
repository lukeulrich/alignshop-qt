/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "GzipPipe.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param target [QIODevice *]
  * @param parent [QObject *]
  */
GzipPipe::GzipPipe(QIODevice *target, QObject *parent)
    : Pipe(target, parent),
      ready_(false),
      compressionLevel_(Z_DEFAULT_COMPRESSION)
{
    // Prepares the zStream
    setup();
}

/**
  */
GzipPipe::~GzipPipe()
{
    deflateEnd(&zStream_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool GzipPipe::flush()
{
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

    return writeDataToTarget(deflationBuffer);
}

/**
  * Does not change the compression level
  *
  * @returns bool
  */
bool GzipPipe::reset()
{
    ready_ = true;
    deflateEnd(&zStream_);
    setup();

    return QIODevice::reset() && ready_;
}

/**
  * Must call reset to take effect.
  *
  * @param level
  */
void GzipPipe::setCompressionLevel(int level)
{
    ASSERT_X(level >= -1 && level <= 9, "compression level out of range");

    compressionLevel_ = level;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param data [const char *]
  * @param len [qint64]
  * @param buffer [QByteArray &]
  * @returns bool
  */
bool GzipPipe::processData(const char *data, qint64 len, QByteArray &buffer)
{
    zStream_.avail_in = len;
    zStream_.next_in = (uchar *)data;

    do
    {
        zStream_.avail_out = kChunkSize;
        zStream_.next_out = (uchar *)out_;
        deflate(&zStream_, Z_NO_FLUSH);
        buffer.append(out_, kChunkSize - zStream_.avail_out);
    }
    while (zStream_.avail_out == 0);

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  */
void GzipPipe::setup()
{
    zStream_.zalloc = Z_NULL;
    zStream_.zfree = Z_NULL;
    zStream_.opaque = Z_NULL;

    // The 15 + 16 indicates that we should use gzip encoding
    if (deflateInit2(&zStream_, compressionLevel_, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        setErrorString("Unable to initialize zlib");
        ready_ = false;
    }
}
