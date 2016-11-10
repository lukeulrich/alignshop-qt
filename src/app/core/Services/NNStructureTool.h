/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef NNSTRUCTURETOOL_H
#define NNSTRUCTURETOOL_H

#include <QtCore/QObject>

#include "../PODs/Q3Prediction.h"
#include "../constants.h"
#include "../global.h"
#include "../../../fann/include/floatfann.h"

class FannWrapper;
struct NormalizedPssm;

/**
  * NNStructureTool uses a two-stage neural network to predict the secondary structure from a normalized PSSM.
  *
  * The normalized PSSM expected by this tool is one that has had its scores divided by the scale factor and mapped
  * between 0 and 1 using the logistic function). Currently, it is assumed that the neural networks here are for 20
  * character (amino acids) pssms and that input scores are in alphabetical order.
  */
class NNStructureTool : public QObject
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    //! Construct an instance of NNStructureTool with the given neural network files and parent
    explicit NNStructureTool(const QString &stage1NeuralNetFile,
                             const QString &stage2NeuralNetFile,
                             QObject *parent = nullptr);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Predict and return a Q3Prediction from the normalizedPssm
    Q3Prediction predictSecondary(const NormalizedPssm &normalizedPssm);
    int windowSize() const;                                 //!< Returns the current window size

private:
    // ------------------------------------------------------------------------------------------------
    // Private members
    static const int kWindowSize = 15;
    static const int kHalfWindowSize = kWindowSize / 2;
    static const int kStage1NInputsPerRow = constants::kPssmWidth + 1;   // The extra input is to delineate whether this position
                                                                         // in the sliding window extends beyond pssm boundaries
    static const int kStage1TotalInputArraySize = kStage1NInputsPerRow * kWindowSize;
    static const int kNOutputs = 3;

    static const int kStage2NInputsPerRow = kNOutputs + 1;
    static const int kStage2TotalInputArraySize = kStage2NInputsPerRow * kWindowSize;

    FannWrapper *stage1NN_;
    FannWrapper *stage2NN_;

    QVector<fann_type> stage1Inputs_;
    QVector<fann_type> stage2Inputs_;
};

#endif // NNSTRUCTURETOOL_H
