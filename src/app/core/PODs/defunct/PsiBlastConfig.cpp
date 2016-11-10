/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "PsiBlastConfig.h"
#include "../macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  */
PsiBlastConfig::PsiBlastConfig()
{
    matrix_ = BLOSUM62;

    nIterations_ = 1;
    nThreads_ = 1;
    evalue_ = 10.;
    inclusionEthresh_ = .001;
    nDescriptions_ = 100;
    nAlignments_ = 100;

    nMaxTargetSeqs_ = 0;

    databaseSize_ = 0;
    searchSpace_ = 0;

    outputFormat_ = PairwiseOutput;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
bool PsiBlastConfig::operator==(const PsiBlastConfig &other) const
{
    if (this == &other)
        return true;

    return psiblastPath_ == other.psiblastPath_ &&
           blastDatabase_ == other.blastDatabase_ &&
           matrix_ == other.matrix_ &&
           agPssmFile_ == other.agPssmFile_ &&
           nIterations_ == other.nIterations_ &&
           nThreads_ == other.nThreads_ &&
           evalue_ == other.evalue_ &&
           inclusionEthresh_ == other.inclusionEthresh_ &&
           nDescriptions_ == other.nDescriptions_ &&
           nAlignments_ == other.nAlignments_ &&
           databaseSize_ == other.databaseSize_ &&
           searchSpace_ == other.searchSpace_ &&
           outputFormat_ == other.outputFormat_ &&
           configuredOptions_ == other.configuredOptions_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QString
  */
QString PsiBlastConfig::agPssmFile() const
{
    return agPssmFile_;
}

/**
  * @returns int
  */
int PsiBlastConfig::alignments() const
{
    return nAlignments_;
}

/**
  * Returns a BLAST+ compatible QStringList of command-line arguments. Does not include the psiblastPath argument.
  *
  * @returns QStringList
  */
QStringList PsiBlastConfig::argumentList() const
{
    QStringList arguments;
    if (configuredOptions_.contains(eAgPssmOption))
    {
        ASSERT(agPssmFile_.isEmpty() == false);
        arguments << "-out_ag_pssm" << agPssmFile_;
    }

    if (configuredOptions_.contains(eNumAlignmentsOption))
        arguments << "-num_alignments" << QString::number(nAlignments_);

    if (configuredOptions_.contains(eBlastDatabaseOption))
        arguments << "-db" << blastDatabase_;

    if (configuredOptions_.contains(eNumDescriptionsOption))
        arguments << "-num_descriptions" << QString::number(nDescriptions_);

    if (configuredOptions_.contains(eEvalueOption))
        arguments << "-evalue" << QString::number(evalue_);

    if (configuredOptions_.contains(eInclusionThreshOption))
        arguments << "-inclusion_ethresh" << QString::number(inclusionEthresh_);

    if (configuredOptions_.contains(eIterationsOption))
        arguments << "-num_iterations" << QString::number(nIterations_);

    if (configuredOptions_.contains(eMatrixOption))
        arguments << "-matrix" << stringFromMatrix(matrix_);

    if (configuredOptions_.contains(eMaxTargetSeqsOption))
        arguments << "-max_target_seqs" << QString::number(nMaxTargetSeqs_);

    if (configuredOptions_.contains(eDatabaseSizeOption))
        arguments << "-dbsize" << QString::number(databaseSize_);

    if (configuredOptions_.contains(eSearchSpaceOption))
        arguments << "-searchsp" << QString::number(searchSpace_);

    if (configuredOptions_.contains(eOutputFormatOption))
        arguments << "-outfmt" << QString::number(outputFormat_);

    if (configuredOptions_.contains(eThreadsOption))
        arguments << "-num_threads" << QString::number(nThreads_);

    return arguments;
}

/**
  * @returns QString
  */
QString PsiBlastConfig::blastDatabase() const
{
    return blastDatabase_;
}

/**
  * @returns qint64
  */
qint64 PsiBlastConfig::databaseSize() const
{
    return databaseSize_;
}

/**
  * @returns int
  */
int PsiBlastConfig::descriptions() const
{
    return nDescriptions_;
}

/**
  * @returns double
  */
double PsiBlastConfig::evalue() const
{
    return evalue_;
}

/**
  * @returns double
  */
double PsiBlastConfig::inclusionEthresh() const
{
    return inclusionEthresh_;
}

/**
  * @returns int
  */
int PsiBlastConfig::iterations() const
{
    return nIterations_;
}

/**
  * @returns PsiBlastConfig::BlastMatrix
  */
PsiBlastConfig::BlastMatrix PsiBlastConfig::matrix() const
{
    return matrix_;
}

/**
  * @returns int
  */
int PsiBlastConfig::maxTargetSeqs() const
{
    return nMaxTargetSeqs_;
}

/**
  * @returns PsiBlastConfig::OutputFormat
  */
PsiBlastConfig::OutputFormat PsiBlastConfig::outputFormat() const
{
    return outputFormat_;
}

/**
  * @returns QString
  */
QString PsiBlastConfig::psiBlastPath() const
{
    return psiblastPath_;
}

/**
  * @returns qint64
  */
qint64 PsiBlastConfig::searchSpace() const
{
    return searchSpace_;
}

/**
  * @returns int
  */
int PsiBlastConfig::threads() const
{
    return nThreads_;
}

/**
  * @param agPssmFile [const QString &]
  */
void PsiBlastConfig::setAgPssmFile(const QString &agPssmFile)
{
    if (!agPssmFile.isEmpty())
        configuredOptions_ << eAgPssmOption;
    else
        configuredOptions_.remove(eAgPssmOption);

    agPssmFile_ = agPssmFile;
}

/**
  * @param nAlignments [const int]
  */
void PsiBlastConfig::setAlignments(const int nAlignments)
{
    ASSERT(nAlignments >= 0);

    configuredOptions_ << eNumAlignmentsOption;
    nAlignments_ = nAlignments;
}

/**
  * @param blastDatabase [const QString &]
  */
void PsiBlastConfig::setBlastDatabase(const QString &blastDatabase)
{
    configuredOptions_ << eBlastDatabaseOption;
    blastDatabase_ = blastDatabase;
}

/**
  * @param databaseSize [const qint64]
  */
void PsiBlastConfig::setDatabaseSize(const qint64 databaseSize)
{
    ASSERT(databaseSize >= 0);

    configuredOptions_ << eDatabaseSizeOption;
    databaseSize_ = databaseSize;
}

/**
  * @param nDescriptions [const int]
  */
void PsiBlastConfig::setDescriptions(const int nDescriptions)
{
    ASSERT(nDescriptions >= 0);

    configuredOptions_ << eNumDescriptionsOption;
    nDescriptions_ = nDescriptions;
}

/**
  * @param evalue [const double]
  */
void PsiBlastConfig::setEvalue(const double evalue)
{
    configuredOptions_ << eEvalueOption;
    evalue_ = evalue;
}

/**
  * @param inclusionEthresh [const double]
  */
void PsiBlastConfig::setInclusionEthresh(const double inclusionEthresh)
{
    configuredOptions_ << eInclusionThreshOption;
    inclusionEthresh_ = inclusionEthresh;
}

/**
  * @param nIterations [const int]
  */
void PsiBlastConfig::setIterations(const int nIterations)
{
    ASSERT(nIterations > 0);

    configuredOptions_ << eIterationsOption;
    nIterations_ = nIterations;
}

/**
  * @param matrix [const BlastMatrix &]
  */
void PsiBlastConfig::setMatrix(const BlastMatrix &matrix)
{
    configuredOptions_ << eMatrixOption;
    matrix_ = matrix;
}

/**
  * @param nMaxTargetSeqs [const int]
  */
void PsiBlastConfig::setMaxTargetSeqs(const int nMaxTargetSeqs)
{
    ASSERT(nMaxTargetSeqs > 0);

    configuredOptions_ << eMaxTargetSeqsOption;
    nMaxTargetSeqs_ = nMaxTargetSeqs;
}

/**
  * @param outputFormat [const OutputFormat &]
  */
void PsiBlastConfig::setOutputFormat(const OutputFormat &outputFormat)
{
    configuredOptions_ << eOutputFormatOption;
    outputFormat_ = outputFormat;
}

/**
  * @param psiblastPath [const QString &]
  */
void PsiBlastConfig::setPsiBlastPath(const QString &psiblastPath)
{
    psiblastPath_ = psiblastPath;
}

/**
  * @param searchSpace [const qint64]
  */
void PsiBlastConfig::setSearchSpace(const qint64 searchSpace)
{
    ASSERT(searchSpace >= 0);

    configuredOptions_ << eSearchSpaceOption;
    searchSpace_ = searchSpace;
}

/**
  * @param nThreads [const int]
  */
void PsiBlastConfig::setThreads(const int nThreads)
{
    ASSERT(nThreads > 0);

    configuredOptions_ << eThreadsOption;
    nThreads_ = nThreads;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public static methods
/**
  * @returns PsiBlastConfig::BlastMatrix
  */
PsiBlastConfig::BlastMatrix PsiBlastConfig::matrixFromString(const QString &string)
{
    if (string == "BLOSUM45")
        return BLOSUM45;
    else if (string == "BLOSUM50")
        return BLOSUM50;
    else if (string == "BLOSUM62")
        return BLOSUM62;
    else if (string == "BLOSUM80")
        return BLOSUM80;
    else if (string == "BLOSUM90")
        return BLOSUM90;
    else if (string == "PAM30")
        return PAM30;
    else if (string == "PAM70")
        return PAM70;
    else if (string == "PAM250")
        return PAM250;

    return BLOSUM62;
}

/**
  * @returns QStringList
  */
QStringList PsiBlastConfig::matrixList()
{
    return QStringList()
            << "BLOSUM45"
            << "BLOSUM50"
            << "BLOSUM62"
            << "BLOSUM80"
            << "BLOSUM90"
            << "PAM30"
            << "PAM70"
            << "PAM250";
}

/**
  * @returns QString
  */
QString PsiBlastConfig::stringFromMatrix(const BlastMatrix &matrix)
{
    switch (matrix)
    {
    case BLOSUM45:
        return "BLOSUM45";
    case BLOSUM50:
        return "BLOSUM50";
    case BLOSUM62:
        return "BLOSUM62";
    case BLOSUM80:
        return "BLOSUM80";
    case BLOSUM90:
        return "BLOSUM90";

    case PAM30:
        return "PAM30";
    case PAM70:
        return "PAM70";
    case PAM250:
        return "PAM250";

    default:
        return "BLOSUM62";
    }
}
