/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GZIPPIPE_H
#define GZIPPIPE_H

#include "zlib.h"

#include "Pipe.h"
#include "../global.h"

/**
  * GzipPipe extends Pipe with the capability to deflate (compress) a data stream on the fly.
  *
  * If an error occurs during processing, it is necessary to call reset before any more data will be processed. Since
  * zlib maintains an internal buffer for compression, and it is not possible to determine distinct gzip operations, a
  * flush method enables users to process any buffered data.
  */
class GzipPipe : public Pipe
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    GzipPipe(QIODevice *target, QObject *parent = nullptr);     //!< Construct a gzip pipe to send deflated data to target
    ~GzipPipe();                                                //!< Destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool flush();                                               //!< Flushes any buffered data; returns true on success, false otherwise
    bool reset();                                               //!< Resets the pipe and readies it for processing; clears any errors; returns true on success, false otherwise
    void setCompressionLevel(int level);                        //!< Sets the compression level which must be between -1 and 9


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    //! Deflates up to len bytes of data and stores the compressed data in buffer
    virtual bool processData(const char *data, qint64 len, QByteArray &buffer);


private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    void setup();                                               //!< Resets and initializes and the zlib library


    // ------------------------------------------------------------------------------------------------
    // Private static members
    static const int kChunkSize = 16384;                        //!< Buffer size during I/O operations


    // ------------------------------------------------------------------------------------------------
    // Private members
    bool ready_;                    //!< Is the stream ready for processing
    z_stream zStream_;              //!< Zlib stream member
    int compressionLevel_;          //!< Level of compression (-1 is default, 0-9)
    char out_[kChunkSize];          //!< Internal buffer for storing inflated data
};


#endif // GZIPPIPE_H
