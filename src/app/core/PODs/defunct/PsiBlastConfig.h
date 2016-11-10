/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef PSIBLASTCONFIG_H
#define PSIBLASTCONFIG_H

#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>

/**
  * PsiBlastConfig contains configuration data for running PSI-BLAST.
  */
class PsiBlastConfig
{
public:
    enum BlastMatrix {
        BLOSUM45 = 0,
        BLOSUM50,
        BLOSUM62,
        BLOSUM80,
        BLOSUM90,
        PAM30,
        PAM70,
        PAM250
    };

    enum OutputFormat {
        PairwiseOutput = 0,
        QueryAnchoredWithIdentitiesOutput,
        QueryAnchoredNoIdentitiesOutput,
        FlatQueryAnchoredWithIdentitiesOutput,
        FlatQueryAnchoredNoIdentitiesOutput,
        XmlOutput,
        TabularOutput,
        TabularWithCommentsOutput,
        TextAsn1Output,
        BinaryAsn1Output,
        CSVOutput,
        BlastAsn1
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    PsiBlastConfig();                                           //!< Constructor that sets default options

    // ------------------------------------------------------------------------------------------------
    // Operators
    bool operator==(const PsiBlastConfig &other) const;

    // ------------------------------------------------------------------------------------------------
    // Public methods (getter)
    QString agPssmFile() const;                                 //!< Returns the name of the AG-formatted PSSM file
    int alignments() const;                                     //!< Returns the number of alignments
    QStringList argumentList() const;                           //!< Returns a QStringList of blast+ formatted command line arguments from this configuration struct
    QString blastDatabase() const;                              //!< Returns the blast database
    qint64 databaseSize() const;                                //!< Returns the effective length of the database
    int descriptions() const;                                   //!< Returns the number of descriptions
    double evalue() const;                                      //!< Returns the evalue cutoff threshold
    double inclusionEthresh() const;                            //!< Returns the evalue inclusion cutoff threshold for multiple iterations
    int iterations() const;                                     //!< Returns the number of iterations to perform
    BlastMatrix matrix() const;                                 //!< Returns the BLAST matrix to use for searching
    int maxTargetSeqs() const;
    OutputFormat outputFormat() const;                          //!< Returns the current output format
    QString psiBlastPath() const;                               //!< Returns the path to the PSI-BLAST executable
    qint64 searchSpace() const;                                 //!< Returns the effective length of the search space
    int threads() const;                                        //!< Returns the number of threads

    // (setter)
    void setAgPssmFile(const QString &agPssmFile);              //!< Sets the AG-formatted PSSM output file to agPssmFile
    void setAlignments(const int nAlignments);                  //!< Sets the number of alignments to nAlignments
    void setBlastDatabase(const QString &blastDatabase);        //!< Sets the blast database
    void setDatabaseSize(const qint64 databaseSize);            //!< Sets the effective length of the database
    void setDescriptions(const int nDescriptions);              //!< Sets the number of descriptions to return
    void setEvalue(const double evalue);                        //!< Sets the evalue cutoff threshold
    void setInclusionEthresh(const double inclusionEthresh);    //!< Sets the evalue inclusion threadhold for multiple iterations
    void setIterations(const int nIterations);                  //!< Sets the number of iterations to perform
    void setMatrix(const BlastMatrix &matrix);                  //!< Sets the BLAST data matrix
    void setMaxTargetSeqs(const int nMaxTargetSeqs);            //!< Sets the maximum number of target sequences to nMaxTargetSeqs
    void setOutputFormat(const OutputFormat &outputFormat);     //!< Sets the output format to outputFormat
    void setPsiBlastPath(const QString &psiblastPath);          //!< Sets the path to the PSI-BLAST executable
    void setSearchSpace(const qint64 searchSpace);              //!< Sets the effective length of the search space to searchSpace
    void setThreads(const int nThreads);                        //!< Sets the number of threads


    // ------------------------------------------------------------------------------------------------
    // Public static methods
    static BlastMatrix matrixFromString(const QString &string); //!< Returns the BlastMatrix that corresponds to string or BLOSUM62 if none matches
    static QStringList matrixList();                            //!< Returns a string list of allowed BLAST matrices
    static QString stringFromMatrix(const BlastMatrix &matrix); //!< Returns the string representation of matrix


private:
    // ------------------------------------------------------------------------------------------------
    // Private enums
    enum {
        eAgPssmOption = 0,
        eNumAlignmentsOption,
        eBlastDatabaseOption,
        eNumDescriptionsOption,
        eEvalueOption,
        eInclusionThreshOption,
        eIterationsOption,
        eMatrixOption,
        eMaxTargetSeqsOption,
        eDatabaseSizeOption,
        eSearchSpaceOption,
        eOutputFormatOption,
        eThreadsOption
    };


    // ------------------------------------------------------------------------------------------------
    // Private members
    QString psiblastPath_;
    QString blastDatabase_;
    BlastMatrix matrix_;
    QString agPssmFile_;
    int nIterations_;
    int nThreads_;
    double evalue_;
    double inclusionEthresh_;
    int nDescriptions_;
    int nAlignments_;
    int nMaxTargetSeqs_;
    qint64 databaseSize_;           // Effective length of the database
    qint64 searchSpace_;            // Effective length of the search space
    OutputFormat outputFormat_;


    QSet<int> configuredOptions_;
};

#endif // PSIBLASTCONFIG_H
