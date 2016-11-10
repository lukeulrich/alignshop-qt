/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PIPE_H
#define PIPE_H

#include <QtCore/QIODevice>

/**
  * Pipe provides a general interface for receiving raw input from other sources, manipulating this data, and then
  * sending this data to a target device.
  *
  * Motivation: Virtually all *nix platforms support chaining together multiple programs that simply pipe their output
  *             from one program to the next. This functionality is incredibly powerful for generating powerful
  *             processing pipelines.
  *
  * Pipe extends QIODevice providing both an interface and generic implementation for manipulating data stream's.
  * By nature, it is a sequential device - random access is not supported. Upon construction it is immediately opened in
  * write only mode. The open and close methods are protected to hide these from the user.
  *
  * Subclasses should implement any data manipulation in the processData() method.
  */
class Pipe : public QIODevice
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructors and destructor
    Pipe(QIODevice *target, QObject *parent = 0);           //!< Construct a pipe which will send all its output to target and is owned by parent
    ~Pipe();                                                //!< Destructor


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isSequential() const;                              //!< Always returns true
    QIODevice *target() const;                              //!< Returns the output target
    void setTarget(QIODevice *target);                      //!< Sets the output device to target


Q_SIGNALS:
    void error(const QString &error);                       //!< Emitted whenever an error occurs


protected:
    // ------------------------------------------------------------------------------------------------
    // Protected methods
    void close();                                           //!< Closes the pipe
    bool open(OpenMode mode);                               //!< Opens the pipe in mode; mode should always be QIODevice::WriteOnly
    //! Core data manipulation method for tweaking len bytes of data and storing the result in buffer; this base class implementation does nothing
    virtual bool processData(const char *data, qint64 len, QByteArray &buffer);
    qint64 readData(char *data, qint64 maxlen);             //!< Always returns -1; reading from a pipe is not supported
    qint64 writeData(const char *data, qint64 len);         //!< Returns the number of bytes processed from data and len
    bool writeDataToTarget(const QByteArray &data);         //!< Convenience method that writes data to the output target and returns true on success, false otherwise
    bool writeDataToTarget(const char *data, int length);   //!< Method for writing length bytes of data to the output target; returns true on success, false otherwise


private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    QIODevice *target_;         //!< Output device
};

#endif // PIPE_H
