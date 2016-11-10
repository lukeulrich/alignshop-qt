/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "GunzipPipe.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param target [QIODevice *]
  * @param parent [QObject *]
  */
GunzipPipe::GunzipPipe(QIODevice *target, QObject *parent)
    : Pipe(target, parent),
      ready_(true)
{
    setup();
}

/**
  */
GunzipPipe::~GunzipPipe()
{
    inflateEnd(&zStream_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool GunzipPipe::reset()
{
    ready_ = true;
    inflateEnd(&zStream_);
    setup();

    return QIODevice::reset() && ready_;
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
bool GunzipPipe::processData(const char *data , qint64 len, QByteArray &buffer)
{
    if (!ready_)
        return false;

    zStream_.avail_in = len;
    zStream_.next_in = (uchar *)data;

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
            return false;
        }
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
void GunzipPipe::setup()
{
    zStream_.zalloc = Z_NULL;
    zStream_.zfree = Z_NULL;
    zStream_.opaque = Z_NULL;

    if (inflateInit2(&zStream_, 15 + 16) != Z_OK)
    {
        setErrorString("Unable to initialize zlib");
        ready_ = false;
    }
}
