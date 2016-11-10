/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef IADOCSOURCE_H
#define IADOCSOURCE_H

#include "Crud/IAnonSeqEntityCrud.h"
#include "Crud/IBlastReportCrud.h"
#include "Crud/IEntityCrud.h"
#include "Crud/IMsaCrud.h"

class AdocTreeNode;
class Astring;
struct AstringPod;
class AminoSeq;
struct AminoSeqPod;
class BlastReport;
struct BlastReportPod;
class Dstring;
struct DstringPod;
class DnaSeq;
struct DnaSeqPod;

class AminoMsa;
struct AminoMsaPod;
class DnaMsa;
struct DnaMsaPod;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
class IAdocSource
{
public:
    virtual ~IAdocSource() {}

    virtual void begin() = 0;
    virtual void end() = 0;

    // General garbage collection method
    virtual void removeCruft() = 0;
    virtual void vacuum() = 0;

    // Entity tree hierarchy associated with this data source
    virtual AdocTreeNode *readEntityTree() = 0;
    virtual void saveEntityTree(AdocTreeNode *root) = 0;

    // Convenience method to provide a single point of access for templated generics!
    virtual IAnonSeqEntityCrud<Astring, AstringPod> *crud(Astring *) = 0;
    virtual IEntityCrud<AminoSeq, AminoSeqPod> *crud(AminoSeq *) = 0;
    virtual IAnonSeqEntityCrud<Dstring, DstringPod> *crud(Dstring *) = 0;
    virtual IEntityCrud<DnaSeq, DnaSeqPod> *crud(DnaSeq *) = 0;

    virtual IMsaCrud<AminoMsa, AminoMsaPod> *crud(AminoMsa *) = 0;
    virtual IMsaCrud<DnaMsa, DnaMsaPod> *crud(DnaMsa *) = 0;

    virtual IBlastReportCrud *crud(BlastReport *) = 0;

    virtual IAnonSeqEntityCrud<Astring, AstringPod> *astringCrud() = 0;
    virtual IEntityCrud<AminoSeq, AminoSeqPod> *aminoSeqCrud() = 0;
    virtual IAnonSeqEntityCrud<Dstring, DstringPod> *dstringCrud() = 0;
    virtual IEntityCrud<DnaSeq, DnaSeqPod> *dnaSeqCrud() = 0;

    virtual IMsaCrud<AminoMsa, AminoMsaPod> *aminoMsaCrud() = 0;
    virtual IMsaCrud<DnaMsa, DnaMsaPod> *dnaMsaCrud() = 0;

    virtual IBlastReportCrud *blastReportCrud() = 0;
};

#endif // IADOCSOURCE_H
