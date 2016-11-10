/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef UNTARSINK_H
#define UNTARSINK_H

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QIODevice>
#include <QtCore/QPair>
#include "../global.h"
#include "../PODs/TarHeaderPod.h"

/**
  * UntarSink is a write only device that extracts files encoded in the tar format.
  *
  * Before any incoming data will be extracted, an output directory must have been specified. For each file encountered
  * the extractStart / extractFinished signals will be emitted. During each file extraction, the progressChanged signals
  * correspond to the amount that the file has been extracted. There will be approximately 50 progressChanged signals
  * emitted per file.
  *
  * If an error is encountered at any point, the error signal will be emitted and all further data is discarded until
  * the device has been reset. Reseting the device closes any open files, clears the error state, and readies the
  * device for receiving data.
  *
  * Note: It is important to remember that the file sizes are rounded up to a multiple of 512 byte disk sectors with
  *       any extra space being zero-padded.
  */
class UntarSink : public QIODevice
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    UntarSink(QObject *parent = nullptr);                           //!< Default constructor with no output directory specified
    UntarSink(const QDir &directory, QObject *parent = nullptr);    //!< Constructor with output directory set to directory
    ~UntarSink();                                                   //!< Destructor; closes any open file

    // ------------------------------------------------------------------------------------------------
    // Public methods
    QDir directory() const;                                         //!< Returns the destination directory for extracted files
    bool isSequential();                                            //!< Returns true, indicating that this device does not support random access
    bool reset();                                                   //!< Close any open files, clears the error state, and readies the device for reading data again
    bool setDirectory(const QDir &directory);                       //!< Returns true if the destination directory could be set to directory; false otherwise

Q_SIGNALS:
    // ------------------------------------------------------------------------------------------------
    // Signals
    void extractStarted(const QString &file);                         //!< Emitted when the extraction of file has started
    void extractFinished(const QString &file);                      //!< Emitted when the extraction of file has finished
    void progressChanged(qint64 done, qint64 total);                //!< Emitted when the extraction progress of the current file has completed done steps out of total steps; it is possible to have a zero total

    void error(const QString &error);                               //!< Emitted when an error has been encountered

protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void close();                                                   //!< Closes the device
    bool open(OpenMode mode);                                       //!< Returns true if the device could be opened in mode
    qint64 readData(char *data, qint64 maxlen);                     //!< Always returns -1, indicating that reading from this device is not supported
    qint64 writeData(const char *data, qint64 len);                 //!< Writes up to len bytes of data to the device and returns the number of bytes written or -1 if an error occurred

private:
    // ------------------------------------------------------------------------------------------------
    // Private structs
    struct FileProgress {
        qint64 written_;
        qint64 total_;

        FileProgress() : written_(0), total_(0)
        {
        }
    };

    // ------------------------------------------------------------------------------------------------
    // Private methods
    bool processTarDirectory(const TarHeaderPod &pod);              //!< Helper process method; returns true if the directory for pod could successfully be handled; false otherwise
    bool processTarFile(const TarHeaderPod &pod);                   //!< Helper process method; returns true if the file for pod could successfully be handled; false otherwise
    bool processTarHeaderPod(const TarHeaderPod &pod);              //!< Main process method; returns true if the pod was successfully handled (e.g. start extraction, create directory, etc); false otherwise
    bool processTarSymlink(const TarHeaderPod &pod);                //!< Helper process method; returns true if the symlink for pod could successfully be handled; false otherwise
    bool writeAll(QIODevice *device, const char *data, int length); //!< Returns true if length bytes of data could be written to device or false otherwise

    // ------------------------------------------------------------------------------------------------
    // Private members
    QFile file_;                    //!< Currently opened file
    QDir directory_;                //!< Output directory
    bool errored_;                  //!< Flag denoting error state
    FileProgress fileProgress_;     //!< Number of bytes written and total file size for the current file being transferred
    //! Raw tar header pod (may be partial) for the current tar
    TarHeaderRawPod tarHeaderRawPod_;
    int tarHeaderBytesRead_;        //!< It is quite possible to receive a partial tar header and thus this variable tracks how many bytes have been stored in the tar header
    int skipBytes_;                 //!< Number of bytes to skip when receiving data to move to the next tar header
};


#endif // UNTARSINK_H
