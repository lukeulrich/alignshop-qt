/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>

#include "FannWrapper.h"

#ifdef Q_OS_UNIX
  #include <unistd.h>
#elif defined Q_OS_WIN
  #include <io.h>        // For _pipe
  #include <fcntl.h>     // For O_TEXT definition
  #include <cstdio>      // For _fdopen
#endif

#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * If neuralNetFile is not empty, attempts to initialize a FANN neural network from this file, which in turn may throw
  * an error (QString).
  *
  * @param neuralNetFile [const QString &]
  * @param parent [QObject *]
  * @see setNeuralNetFile()
  */
FannWrapper::FannWrapper(const QString &neuralNetFile, QObject *parent)
    : QObject(parent),
      fann_(nullptr),
      neuralNetFile_(neuralNetFile)
{
    if (neuralNetFile_.isEmpty() == false)
        setNeuralNetFile(neuralNetFile);
}

/**
  */
FannWrapper::~FannWrapper()
{
    if (fann_ != nullptr)
        fann_destroy(fann_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool FannWrapper::isValid() const
{
    return fann_ != nullptr;
}

/**
  * @returns QString
  */
QString FannWrapper::neuralNetFile() const
{
    return neuralNetFile_;
}

/**
  * @returns int
  */
int FannWrapper::nInputs() const
{
    if (fann_ != nullptr)
        return fann_get_num_input(fann_);

    return 0;
}

/**
  * @returns int
  */
int FannWrapper::nOutputs() const
{
    if (fann_ != nullptr)
        return fann_get_num_output(fann_);

    return 0;
}

/**
  * This method will throw an error if the inputs vector size is not equal to nInputs() or an error is encountered from
  * the fann_run method. If a valid neural network has not been initialized via setNeuralNetFile(), then an empty result
  * will be returned.
  *
  * @param inputs [const QVector<fann_type> &]
  * @returns QVector<double>
  * @see setNeuralNetFile(), nInputs(), nOutputs()
  */
QVector<double> FannWrapper::runFann(const QVector<fann_type> &inputs) const
{
    if (fann_ == nullptr)
        return QVector<double>();

    if (inputs.size() != nInputs())
        throw QString("Invalid number of input values (%1; expected: %2)").arg(inputs.size()).arg(nInputs());

    // Note: we are performing a const_cast to get around the fact that fann_run takes a non-const pointer even though
    // it does not modify them. If we simply removed the const qualifier from the inputs argument, it would convey that
    // this method indeed may alter the inputs; however, this is not the case.
    fann_type *outputs = fann_run(fann_, const_cast<fann_type *>(inputs.data()));

    // Check for an error condition
    if (fann_get_errno(reinterpret_cast<struct fann_error*>(fann_)) != FANN_E_NO_ERROR)
        throw QString("Unknown error encountered.");

    // Convert the results to a QVector of doubles
    int nOutputs = this->nOutputs();
    QVector<double> result(nOutputs);
    for (int i=0; i<nOutputs; ++i)
        result[i] = static_cast<double>(outputs[i]);

    // Note: the outputs while accessible here via the outputs pointer are owned by the fann_ instance. Therefore,
    // not our responsibility to delete them.

    return result;
}

/**
  * Throws an error (QString) if any of the following conditions occur:
  * 1) neuralNetFile does not exist
  * 2) Could not create a pipe instance
  * 3) Could not open file handle connection to pipe
  * 4) FANN library was unable to create neural network from the given file
  *
  * The neural network file is cleared regardless if this method succeeds or fails.
  *
  * @param neuralNetFile [const QString &]
  */
void FannWrapper::setNeuralNetFile(const QString &neuralNetFile)
{
    neuralNetFile_.clear();
    if (fann_ != nullptr)
    {
        fann_destroy(fann_);
        fann_ = nullptr;
    }

    if (!QFile::exists(neuralNetFile))
        throw QString("Neural network file, '%1', does not exist.").arg(neuralNetFile);

    int pipefd[2];
    if (pipe(pipefd) == -1)
        throw QString("Unable to initialize neural network engine");

    FILE *handle = 0;
    handle = fdopen(pipefd[1], "w");
    if (handle == nullptr)
        throw QString("Unable to initialize neural network engine");

    fann_set_error_log(nullptr, handle);
    fann_ = fann_create_from_file(neuralNetFile.toAscii().constData());
    fclose(handle);
#ifdef Q_OS_WIN
    // In windows, fclose also calls _close on the underlying handle
#else
    close(pipefd[1]);
#endif
    if (!fann_)
    {
        // To get the actual error:
        /*
        char buffer[1024];
        read(pipefd[0], buffer, 1024);
        */
#ifdef Q_OS_WIN
        _close(pipefd[0]);
#else
        close(pipefd[0]);
#endif

        throw QString("Invalid neural network file.");
    }

#ifdef Q_OS_WIN
    _close(pipefd[0]);
#else
    close(pipefd[0]);
#endif

    neuralNetFile_ = neuralNetFile;
}
