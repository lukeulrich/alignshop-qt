/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef FANNWRAPPER_H
#define FANNWRAPPER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "../global.h"
#include "../../../fann/include/floatfann.h"

/**
  * FannWrapper wraps the LGPL C-based, FANN neural network library for more convenient use with Qt.
  *
  * Currently, only the input and output methods are exposed along with the ability to run the NN with a set of inputs.
  * Other functionality such as training and testing is not supported at this time.
  */
class FannWrapper : public QObject
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    //! Construct instance and initialize a neural network from neuralNetFile
    explicit FannWrapper(const QString &neuralNetFile = QString(), QObject *parent = nullptr);
    ~FannWrapper();                                                     //!< De-allocate the FANN struct if allocated

    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isValid() const;                                               //!< Returns true if the neural network is properly initialized and available
    QString neuralNetFile() const;                                      //!< Returns the current neural network file
    int nInputs() const;                                                //!< Returns the number of inputs or 0 if no active neural network
    int nOutputs() const;                                               //!< Returns the number of outputs or 0 if no active neural network
    QVector<double> runFann(const QVector<fann_type> &inputs) const;    //!< Runs the neural network with the given inputs and returns a vector of doubles
    void setNeuralNetFile(const QString &neuralNetFile);                //!< Reinitialize FANN neural network from the data contained in neuralNetFile

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    fann *fann_;                    //!< Internal fann structure
    QString neuralNetFile_;         //!< Current neural network file
};

#endif // FANNWRAPPER_H
