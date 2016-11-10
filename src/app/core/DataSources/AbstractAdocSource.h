/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTADOCSOURCE_H
#define ABSTRACTADOCSOURCE_H

#include "IAdocSource.h"
#include "../macros.h"

class AbstractAdocSource : public IAdocSource
{
public:
    virtual void begin() {}
    virtual void end()   {}

    virtual void vacuum() {}
    virtual void removeCruft() {}

    IAnonSeqEntityCrud<Astring, AstringPod> *crud(Astring *)        { return astringCrud();  }
    IEntityCrud<AminoSeq, AminoSeqPod> *crud(AminoSeq *)            { return aminoSeqCrud(); }
    IAnonSeqEntityCrud<Dstring, DstringPod> *crud(Dstring *)        { return dstringCrud();  }
    IEntityCrud<DnaSeq, DnaSeqPod> *crud(DnaSeq *)                  { return dnaSeqCrud();   }

    IMsaCrud<AminoMsa, AminoMsaPod> *crud(AminoMsa *)               { return aminoMsaCrud(); }
    IMsaCrud<DnaMsa, DnaMsaPod> *crud(DnaMsa *)                     { return dnaMsaCrud(); }

    IBlastReportCrud *crud(BlastReport *)   { return blastReportCrud(); }

protected:
    AbstractAdocSource() : IAdocSource() {}
};

#endif // ABSTRACTADOCSOURCE_H
