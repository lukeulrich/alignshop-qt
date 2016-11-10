#ifndef ENUMS_H
#define ENUMS_H

#include <QtCore/QMetaType>

// Deviating a bit from convention here and not using 'tri' or 'bool' within each defined enum. Reason: these are a very
// basic type that is somewhat ubiquitous
enum TriBool
{
    eUnknown,
    eFalse,
    eTrue
};

enum Grammar
{
    eUnknownGrammar = 0,
    eAminoGrammar,
    eDnaGrammar,
    eRnaGrammar
};
Q_DECLARE_METATYPE(Grammar)

enum DataFormatType
{
    eUnknownFormat = 0,
    eFastaFormat,
    eAlignedFastaFormat,
    eClustalFormat
};

enum AdocNodeType
{
    eUndefinedNode = 0,      //!< Catch-all node
    eRootNode,               //!< Tree root
    eGroupNode,              //!< Folder node for nested grouping

    eAminoMsaNode,           //!< Amino acid / protein alignment
    eAminoSeqNode,           //!< Amino acid / protein sequence

    eDnaMsaNode,             //!< DNA alignment
    eDnaSeqNode,             //!< DNA sequence

    eBlastReportNode,        //!< Blast report for an amino or DNA sequence

    eTransientTaskNode,

    eMaxNode                 //!< Not a valid value to use, but useful for constraint checking
};

enum EntityType
{
    // Entities which may be present as AdocNodes
    eAminoMsaEntity = eAminoMsaNode,
    eAminoSeqEntity = eAminoSeqNode,

    eDnaMsaEntity = eDnaMsaNode,
    eDnaSeqEntity = eDnaSeqNode,

    eBlastReportEntity = eBlastReportNode,

    eTransientTaskEntity = eTransientTaskNode,

    // Entities which may not be present as AdocNodes
    eAstringEntity = eMaxNode + 1,
    eDstringEntity,
    ePrimerEntity
};

#endif
