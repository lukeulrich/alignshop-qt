/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SEQUENCEIMPORTER_H
#define SEQUENCEIMPORTER_H

#include <QtCore/QList>
#include <QtCore/QString>

#include "ParsedBioString.h"

#include "global.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class AdocTreeModel;
class AnonSeqFactory;
class QModelIndex;

/**
  * SequenceImporter imports raw sequence data and alignments into an Adoc.
  *
  * More specifically, SequenceImporter atomically loads this data into the database as well as
  * adding the relevant nodes to the data tree. To successfully import, SequenceImporter requires
  * the following:
  * o A valid adocTreeModel
  * o The appropriate AnonSeqFactory corresponding to the given alphabet
  * o A known alphabet
  * o One or more sequences
  *
  * These methods are only atomic if the underlying database system is sqlite.
  *
  * Specific responsibilities:
  * o Derive {a|d|r}string_ids for each sequence (using an AnonSeqFactory)
  * o Save the parsed BioStrings to the database and relevant tables (may be different depending if just
  *   sequence data or an alignment is being imported)
  * o Add the relevant nodes to the data tree (via AdocTreeModel)
  *
  * Database functionality is available via the AdocTreeModel->adocDbDataSource_ member
  *
  * All methods that access the database will throw DatabaseError or InvalidConnectionError if there
  * is a database connectivity problem.
  *
  * The default copy and assignment constructor are sufficient because SequenceImporter does not own
  * adocTreeModel_ nor the AnonSeqFactory pointers contained in anonSeqFactories_. Moreover, the
  * anonSeqFactories_ hash is implicitly shared making copying a cheap operation.
  */
class SequenceImporter
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    SequenceImporter();                                     //!< Construct an instance of this class

    // ------------------------------------------------------------------------------------------------
    // Public methods
    //! Import parsedBioStrings as an alignment with the alphabet and populate in the data tree underneath parent; return true if successful or false otherwise
    bool importAlignment(const QString &alignmentName, const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet, const QModelIndex &parent) const;
    //! Import parsedBioStrings of the type alphabet and populate in the data tree underneath parent; return true if successful or false otherwise
    bool importSequences(const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet, const QModelIndex &parent) const;
    //! Set the AdocTreeModel to adocTreeModel
    void setAdocTreeModel(AdocTreeModel *adocTreeModel);
    //! Set the anonSeqFactories to anonSeqFactories
    void setAnonSeqFactories(QHash<Alphabet, AnonSeqFactory *> anonSeqFactories);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    //! Helper function that inserts parsedBioStrings into the source database and returns the list of corresponding subseq ids for each
    QList<int> insertSeqSubseqs(const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet) const;

    AdocTreeModel *adocTreeModel_;
    QHash<Alphabet, AnonSeqFactory *> anonSeqFactories_;
};

#endif // SEQUENCEIMPORTER_H
