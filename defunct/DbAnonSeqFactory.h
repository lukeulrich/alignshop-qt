/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef DBANONSEQFACTORY_H
#define DBANONSEQFACTORY_H

#include "AnonSeqFactory.h"

// ------------------------------------------------------------------------------------------------
// Forward declarations
class SynchronousAdocDataSource;

/**
  * Utilizes a relational database to manage a collection of AnonSeqs.
  *
  * Before this factory will function as expected, the user must initialize the Factory with a SynchronousAdocDataSource
  * and source table (that contains the relevant AnonSeq data). Specifically, this table must have at least the following
  * three columns (or else the database queries will fail and all methods will return invalid data):
  *
  * 1. id integer
  * 2. digest text
  * 3. sequence text
  *
  * The digest column must be a base64-encoded representation of the sequence hash. A binary digest is more storage
  * efficient but not as portable (which in this case is more important).
  *
  * TODO: Error reporting
  */
class DbAnonSeqFactory : public AnonSeqFactory
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct a factory that with digestAlgorithm using data from tableName via adocDataSource
    DbAnonSeqFactory(QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Sha1, const QString &sourceTable = QString(), SynchronousAdocDataSource *adocDataSource = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    virtual AnonSeq add(const BioString &bioString);                    //!< Create (if it does not already exist) or add an AnonSeq via a BioString object
    const SynchronousAdocDataSource *adocDataSource() const;               //!< Returns adocDataSource_
    virtual AnonSeq fetch(const BioString &bioString) const;            //!< Retrieve any AnonSeq with an identical reduced sequence as bioString; returns a valid AnonSeq on success or invalid AnonSeq if not found
    virtual AnonSeq fetch(int id) const;                                //!< Retrieve any AnonSeq with the identifier id; returns a valid AnonSeq on success or invalid AnonSeq if id is not found
    bool ready() const;                                                 //!< Returns true if tableName is not empty and adocDataSource has been defined
    virtual int remove(const BioString &bioString);                     //!< Remove the AnonSeq with an identical reduced sequence as bioString and return the number of rows deleted
    virtual int remove(int id);                                         //!< Remove any AnonSeq identified by id and return the number of rows deleted
    void setAdocDataSource(SynchronousAdocDataSource *adocDataSource);     //!< Sets the AdocDataSource that will serve the underlying AnonSeq data
    void setSourceTable(const QString &sourceTable);                    //!< Sets the table that contains the anonymous data to tableName
    virtual int size() const;                                           //!< Returns the number of AnonSeqs in this factory or 0 if the factory is not properly initialized
    QString sourceTable() const;                                        //!< Returns the name of the database table utilized by this factory

protected:
    //!< Override the base class function to add in the constraint that the id must be greater than zero
    virtual AnonSeq createPrototypedAnonSeq(int id, const BioString &bioString) const;

    QString sourceTable_;                           //!< Actual table containing the anonymous sequence data
    SynchronousAdocDataSource *adocDataSource_;        //!< Underlying data source powering this DbAnonSeqFactory; not owned by this class
};

Q_DECLARE_METATYPE(DbAnonSeqFactory *);

#endif // DBANONSEQFACTORY_H
