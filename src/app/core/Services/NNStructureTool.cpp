/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "NNStructureTool.h"
#include "FannWrapper.h"
#include "../PODs/NormalizedPssm.h"
#include "../macros.h"

#ifdef QT_DEBUG
/**
  * Helper method that dumps the inputs in an easy to read manner
  */
void dumpInputs(fann_type *inputs)
{
    for (int i=0; i< 15; ++i)
    {
        for (int j=0; j< 21; ++j)
        {
            printf("%6.3f ", *inputs);
            ++inputs;
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");
    fflush(stdout);
}
#endif

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param stage1NeuralNetFile [const QString &]
  * @param stage2NeuralNetFile [const QString &]
  * @param parent [QObject *]
  */
NNStructureTool::NNStructureTool(const QString &stage1NeuralNetFile,
                                 const QString &stage2NeuralNetFile,
                                 QObject *parent)
    : QObject(parent),
      stage1NN_(nullptr),
      stage2NN_(nullptr),
      stage1Inputs_(kStage1TotalInputArraySize),
      stage2Inputs_(kStage2TotalInputArraySize)
{
    // Note: this could throw an exception!
    stage1NN_ = new FannWrapper(stage1NeuralNetFile, this);
    stage2NN_ = new FannWrapper(stage2NeuralNetFile, this);

    ASSERT_X(stage1NN_->nInputs() == kStage1TotalInputArraySize, "Unexpected number of inputs");
    ASSERT_X(stage2NN_->nOutputs() == kNOutputs, "Unexpected number of outputs");
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * Because the input vector will be the same size for each row in the pssm, we create this in memory first and then
  * simply update this structure each iteration by shifting off the first element and pushing on the new calculation.
  * This operates very similar to a queue.
  *
  * @param normalizedPssm [const NormalizedPssm &]
  * @returns Q3Prediction
  */
Q3Prediction NNStructureTool::predictSecondary(const NormalizedPssm &normalizedPssm)
{
    Q3Prediction secondary;

    if (normalizedPssm.isEmpty())
        return secondary;

    int pssmLength = normalizedPssm.rows_.size();
    secondary.q3_.resize(pssmLength);

    // The NN expects all scores to be fed according to their alphabetical order; however, they may not be stored
    // in the pssm in alphabetical order. Therefore, create a lookup table to map the values.
    //
    // For example:
    // @data = [ 99, 55, 22 ]
    // @mapping = [ M, T, K]
    //
    // For obtaining the values in alphabetical order, we arrange scoreMap as follows
    // scoreMap[ 0 or K] = 2;
    // scoreMap[ 1 or M] = 0;
    // scoreMap[ 2 or T] = 1;
    //
    // Thus, if we want the value for T, we first lookup its position in the scoreMap, this resolves to 2. The scoreMap
    // value (1 in this case) indicates the corresponding index in the original data array that will contain the value
    // for T:
    //
    // printf("The value of T is: %d\n", data[ scoreMap[ 2 ] ]);

    // static_cast is to simply prevent warning about comparing signed and unsigned integer expressions
    ASSERT(static_cast<int>(qstrlen(constants::kAminoBasicCharacters)) == constants::kPssmWidth);
    int scoreMap[constants::kPssmWidth];
    {
        const char *x = constants::kAminoBasicCharacters;
        for (int i=0; *x; ++x, ++i)
        {
            ASSERT(normalizedPssm.mapping_.indexOf(*x) >= 0);
            scoreMap[i] = normalizedPssm.mapping_.indexOf(*x);
        }
    }

    // Zero input array
    fann_type *stage1InputsAddress = stage1Inputs_.data();
    memset(stage1InputsAddress, 0, kStage1TotalInputArraySize * sizeof(fann_type));

    // Create the first array structure to begin the secondary search process
    fann_type *x = stage1InputsAddress;
    for (int i=0; i< kHalfWindowSize + 1; ++i)
    {
        x += constants::kPssmWidth;
        *x = 1.;                        // This indicates that this position is off the left edge
        ++x;
    }

    for (int i=0; i<= kHalfWindowSize - 1; ++i)
    {
        if (i >= pssmLength)
            break;

        const NormalizedPssmRow &row = normalizedPssm.rows_.at(i);

        // Copy all other values into the array from the normalized PSSM
        for (int j=0; j< constants::kPssmWidth; ++j)
        {
            // Briefly, the NN expects input values in alphabetical order. Thus, we treat j as an indicator of the
            // alphabetical order. In so doing, we can then map j to its real position in the pssm data.
            // See the above description for how we this mapping is created and organized.
            *x = static_cast<fann_type>(row.scores_[ scoreMap[j] ]);
            ++x;
        }
        // Skip the last element in this row (its value remains 0 because this indicates that it is inside the
        // pssm boundaries.
        ++x;
    }

    // Essentially, normalizedPssm.rows_.size() == length of query --> generate secondary prediction for each
    // row. Save the output of this stage for the second stage
    QVector<QVector<double> > stage1_outputs(pssmLength);
    fann_type *src = stage1InputsAddress + kStage1NInputsPerRow;
    fann_type *last_row = stage1InputsAddress + (kStage1TotalInputArraySize - kStage1NInputsPerRow);
    size_t amount = (kStage1TotalInputArraySize - kStage1NInputsPerRow) * sizeof(fann_type);
    for (int i=0; i<pssmLength; ++i)
    {
        // Copy everything to the left
        memmove(stage1InputsAddress, src, amount);

        // Calculate the right-most column
        int column = i + kHalfWindowSize;
        if (column < pssmLength)
        {
            const NormalizedPssmRow &row = normalizedPssm.rows_.at(column);
            x = last_row;
            for (int j=0; j< constants::kPssmWidth; ++j)
            {
                *x = static_cast<fann_type>(row.scores_[ scoreMap[j] ]);
                ++x;
            }
            // Zero because this element is not outside the pssm bounds
            *x = 0.;
        }
        else
        {
            // Extending beyond C-terminus; zero out array
            memset(last_row, 0, sizeof(fann_type) * kStage1NInputsPerRow);

            // Set the very last input for each row to 1 to indicate that it extends beyond the terminus
            *(last_row + kStage1NInputsPerRow - 1) = 1.;
        }

        // Run Stage 1
        stage1_outputs[i] = stage1NN_->runFann(stage1Inputs_);
    }

    // ------------------------------------
    // Stage 2 - rinse and repeat
    // A) Zero the input array
    fann_type *stage2InputsAddress = stage2Inputs_.data();
    memset(stage2InputsAddress, 0, kStage2TotalInputArraySize * sizeof(fann_type));

    // B) Initialize the first array structure - offset by one in the positive direction to compensate for first
    //    time the actual NN loop is executed
    x = stage2InputsAddress;
    for (int i=0; i< kHalfWindowSize + 1; ++i)
    {
        x += kNOutputs;
        *x = 1.;                        // This indicates that this position is off the left edge
        ++x;
    }

    for (int i=0; i<= kHalfWindowSize - 1; ++i)
    {
        if (i >= pssmLength)
            break;

        const QVector<double> &row = stage1_outputs.at(i);

        // Copy all other values into the array from the normalized PSSM
        for (int j=0; j< kNOutputs; ++j)
        {
            *x = static_cast<fann_type>(row.at(j));
            ++x;
        }
        // Skip the last element in this row (its value remains 0 because this indicates that it is inside the
        // pssm boundaries.
        ++x;
    }

    char *q3_x = secondary.q3_.data();

    src = stage2InputsAddress + kStage2NInputsPerRow;
    last_row = stage2InputsAddress + (kStage2TotalInputArraySize - kStage2NInputsPerRow);
    amount = (kStage2TotalInputArraySize - kStage2NInputsPerRow) * sizeof(fann_type);
    for (int i=0; i<pssmLength; ++i)
    {
        // Copy everything to the left
        memmove(stage2InputsAddress, src, amount);

        // Calculate the right-most row
        int column = i + kHalfWindowSize;
        if (column < pssmLength)
        {
            const QVector<double> &row = stage1_outputs.at(column);
            x = last_row;
            for (int j=0; j< kNOutputs; ++j)
            {
                *x = static_cast<fann_type>(row.at(j));
                ++x;
            }
            // Zero because this element is not outside the pssm bounds
            *x = 0.;
        }
        else
        {
            // Extending beyond C-terminus; zero out array
            memset(last_row, 0, sizeof(fann_type) * kStage2NInputsPerRow);
            *(last_row + kStage2NInputsPerRow - 1) = 1.;
        }

        // Run Stage 2
        QVector<double> result = stage2NN_->runFann(stage2Inputs_);

        // Translate the results into a prediction !
        char ss_char = 'L';
        double max = result.first();
        if (result.at(1) > max)
        {
            ss_char = 'H';
            max = result.at(1);
        }
        if (result.at(2) > max)
        {
            ss_char = 'E';
            max = result.at(2);
        }

        *q3_x = ss_char;
        ++q3_x;
        secondary.confidence_ << max;
    }

    return secondary;
}

/**
  * @returns int
  */
int NNStructureTool::windowSize() const
{
    return kWindowSize;
}

