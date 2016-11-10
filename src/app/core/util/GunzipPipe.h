/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef GUNZIPPIPE_H
#define GUNZIPPIPE_H

#include "zlib.h"
#include "Pipe.h"
#include "../global.h"

/**
  * GunzipPipe extends Pipe with the capability to inflate (decompress) a gzip-encoded stream on the fly.
  *
  * If an error occurs during processing, it is necessary to call reset before any more data will be processed.
  */
class GunzipPipe : public Pipe
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    GunzipPipe(QIODevice *target, QObject *parent = nullptr);   //!< Construct a gunzip pipe to send inflated data to target
    ~GunzipPipe();                                              //!< Destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool reset();                                               //!< Resets the pipe and readies it for processing; clears any errors; returns true on success, false otherwise


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected method
    //! Inflates up to len bytes of data and stores the decompressed data in buffer
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
    char out_[kChunkSize];          //!< Internal buffer for storing inflated data
};

#endif // GUNZIPPIPE_H
