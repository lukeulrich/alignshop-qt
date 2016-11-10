/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SequenceImporter.h"

#include <QtCore/QStringBuilder>

#include "models/AdocTreeModel.h"
#include "AdocTreeNode.h"
#include "AnonSeqFactory.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  */
SequenceImporter::SequenceImporter() : adocTreeModel_(0)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Conditions for a successful alignment import:
  * o 2 or more sequences
  * o all sequences must have the same length
  * o Known alphabet
  * o Insertion into all database tables was successful
  *
  * Subthrows:
  * o DatabaseError
  * o InvalidConnectionError
  *
  * @param alignmentName [const QString &]
  * @param parsedBioStrings [const QList<ParsedBioString> &]
  * @param alphabet [Alphabet]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool SequenceImporter::importAlignment(const QString &alignmentName, const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet, const QModelIndex &parent) const
{
    Q_ASSERT_X(alphabet != eUnknownAlphabet, "SequenceImporter::importAlignment", "alphabet must not be unknown");
    if (alphabet == eUnknownAlphabet)  // Release mode catch
        return false;

    if (!adocTreeModel_)
        return false;

    if (!anonSeqFactories_.contains(alphabet))
        return false;

    if (parsedBioStrings.isEmpty())
        return true;

    if (parsedBioStrings.count() < 2)
        return false;

    // Determine the node type, and msa join table field names based on the alphabet
    AdocTreeNode::NodeType nodeType;
    QString msa_id_field;
    QString subseq_id_field;
    switch (alphabet)
    {
    case eAminoAlphabet:
        nodeType = AdocTreeNode::MsaAminoType;
        msa_id_field = "amino_msa_id";
        subseq_id_field = "amino_subseq_id";
        break;
    case eDnaAlphabet:
        nodeType = AdocTreeNode::MsaDnaType;
        msa_id_field = "dna_msa_id";
        subseq_id_field = "dna_subseq_id";
        break;
    case eRnaAlphabet:
        nodeType = AdocTreeNode::MsaRnaType;
        msa_id_field = "rna_msa_id";
        subseq_id_field = "rna_subseq_id";
        break;
    default:
        Q_ASSERT_X(0, "SequenceImporter::importAlignment", "Unrecognized alphabet");
        return false;   // Release mode guard
    }

    // Check that the lengths are equivalent
    int alignmentLength = parsedBioStrings.at(0).bioString_.length();
    for (int i=1, z=parsedBioStrings.count(); i<z; ++i)
        if (parsedBioStrings.at(i).bioString_.length() != alignmentLength)
            return false;

    // Check that the MSA relevant tables exist in the database
    adocTreeModel_->adocDbDataSource_.checkTable(constants::kTableHash[alphabet][eMsaGroup]);
    adocTreeModel_->adocDbDataSource_.checkTable(constants::kTableHash[alphabet][eMsaSubseqGroup]);

    // Start nested transaction
    QString savepointName = "insert_alignment";
    adocTreeModel_->adocDbDataSource_.savePoint(savepointName);

    try
    {
        // Save sequences to the database
        QList<int> subseq_ids = insertSeqSubseqs(parsedBioStrings, alphabet);

        // ------------------------------
        // Alignment specific insertion
        // A. Msa table record
        DataRow msaRow;
        msaRow.setValue("name", alignmentName);
        adocTreeModel_->adocDbDataSource_.insert(msaRow, constants::kTableHash[alphabet][eMsaGroup], QStringList() << "name");
        Q_ASSERT_X(msaRow.id_.isNull() == false, "SequenceImporter::importAlignment", "msa id is not allowed to be empty");

        // B. Associate each of the subseqs with this msa
        QStringList i_msa_subseq_fields;
        i_msa_subseq_fields << msa_id_field << subseq_id_field;
        for (int i=0, z=subseq_ids.count(); i<z; ++i)
        {
            DataRow msaSubseqRow;
            msaSubseqRow.setValue(msa_id_field, msaRow.id_);
            msaSubseqRow.setValue(subseq_id_field, subseq_ids.at(i));
            adocTreeModel_->adocDbDataSource_.insert(msaSubseqRow, constants::kTableHash[alphabet][eMsaSubseqGroup], i_msa_subseq_fields);
            Q_ASSERT_X(msaSubseqRow.id_.isNull() == false, "SequenceImporter::importAlignment", "Unable to insert msa join row");
            if (msaSubseqRow.id_.isNull())  // Release mode guard
                return false;
        }

        // C. Finally, append this MSA to the data tree
        AdocTreeNode *node = new AdocTreeNode(nodeType, alignmentName, constants::kTableHash[alphabet][eMsaGroup], msaRow.id_.toInt());
        if (!adocTreeModel_->appendRow(node, parent))
        {
            delete node;
            node = 0;

            // Failure to add to data tree - rollback all database activity
            adocTreeModel_->adocDbDataSource_.rollbackToSavePoint(savepointName);

            return false;
        }
    }
    catch (...)
    {
        // Rollback the nested transaction and rethrow the exception
        adocTreeModel_->adocDbDataSource_.rollbackToSavePoint(savepointName);
        throw;
    }

    // Import successful - release the save point
    adocTreeModel_->adocDbDataSource_.releaseSavePoint(savepointName);

    return true;
}

/**
  * Subthrows:
  * o DatabaseError
  * o InvalidConnectionError
  *
  * Conditions which return false:
  * o adocTreeModel_ is not defined
  * o no corresponding DbAnonSeqFactory for alphabet
  *
  * If there are no parsedBioStrings, in essence there is nothing to import, so the function returns true.
  *
  * All parsedBioStrings will be imported regardless of their checked and valid status.
  *
  * Note: Because of the minimal nature of ParsedBioString's, only a few columns will be inserted with non-null
  *       values. Thus, this function will fail with a DatabaseError if there are additional columns with non-null
  *       constraints.
  *
  * For example:
  * create table amino_seqs (
  *   id integer primary key,
  *   astring_id integer not null,
  *   label text not null,
  *   organism text not null,      -- This not null constraint will cause the import to fail
  * );
  *
  * But, the following table design will function as expected:
  * create table amino_seqs (
  *   id integer primary key,
  *   astring_id integer not null,
  *   label text not null,
  *   organism text
  * );
  *
  * @param parsedBioStrings [const QList<ParsedBioString> &]
  * @param alphabet [Alphabet]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool SequenceImporter::importSequences(const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet, const QModelIndex &parent) const
{
    Q_ASSERT_X(alphabet != eUnknownAlphabet, "SequenceImporter::importSequences", "alphabet must not be unknown");
    if (alphabet == eUnknownAlphabet)  // Release mode catch
        return false;

    if (!adocTreeModel_)
        return false;

    if (!anonSeqFactories_.contains(alphabet))
        return false;

    // Determine the node type based on the alphabet
    AdocTreeNode::NodeType nodeType;
    switch (alphabet)
    {
    case eAminoAlphabet:        nodeType = AdocTreeNode::SubseqAminoType;        break;
    case eDnaAlphabet:          nodeType = AdocTreeNode::SubseqDnaType;          break;
    case eRnaAlphabet:          nodeType = AdocTreeNode::SubseqRnaType;          break;
    default:
        Q_ASSERT_X(0, "SequenceImporter::importSequences", "Unrecognized alphabet");
        return false;   // Release mode guard
    }

    // Start nested transaction
    QString savepointName = "insert_sequences";
    adocTreeModel_->adocDbDataSource_.savePoint(savepointName);

    try
    {
        // Save sequences to the database
        QList<int> subseq_ids = insertSeqSubseqs(parsedBioStrings, alphabet);

        // Add to the tree model
        for (int i=0, z=parsedBioStrings.size(); i<z; ++i)
        {
            AdocTreeNode *node = new AdocTreeNode(nodeType, parsedBioStrings.at(i).header_, constants::kTableHash[alphabet][eSubseqGroup], subseq_ids.at(i));

            if (!adocTreeModel_->appendRow(node, parent))
            {
                // Free the memory associated with this node before returning
                delete node;
                node = 0;

                // Process failed - rollback nested transaction
                adocTreeModel_->adocDbDataSource_.rollbackToSavePoint(savepointName);

                return false;
            }
        }
    }
    catch (...)
    {
        // Process failed - rollback nested transaction and rethrow the exception
        adocTreeModel_->adocDbDataSource_.rollbackToSavePoint(savepointName);
        throw;
    }

    // Process succeeded - release the savepoint
    adocTreeModel_->adocDbDataSource_.releaseSavePoint(savepointName);

    return true;
}

/**
  * Does NOT take ownership of adocTreeModel, but rather shares it from a longer-lived external object
  *
  * @param adocTreeModel [AdocTreeModel]
  */
void SequenceImporter::setAdocTreeModel(AdocTreeModel *adocTreeModel)
{
    adocTreeModel_ = adocTreeModel;
}

/**
  * @param anonSeqFactories [QHash<Alphabet, AnonSeqFactory *>]
  */
void SequenceImporter::setAnonSeqFactories(QHash<Alphabet, AnonSeqFactory *> anonSeqFactories)
{
    anonSeqFactories_ = anonSeqFactories;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param parsedBioStrings [const QList<ParsedBioString> &]
  * @param alphabet [Alphabet]
  * @returns QList<int>
  */
QList<int> SequenceImporter::insertSeqSubseqs(const QList<ParsedBioString> &parsedBioStrings, Alphabet alphabet) const
{
    // Check that the relevant tables exist in the database
    adocTreeModel_->adocDbDataSource_.checkTable(constants::kTableHash[alphabet][eSeqGroup]);
    adocTreeModel_->adocDbDataSource_.checkTable(constants::kTableHash[alphabet][eSubseqGroup]);

    // Determine some default field names and values
    QString string_id_field;
    QString seq_id_field;
    switch (alphabet)
    {
    case eAminoAlphabet:
        string_id_field = "astring_id";
        seq_id_field = "amino_seq_id";
        break;
    case eDnaAlphabet:
        string_id_field = "dstring_id";
        seq_id_field = "dna_seq_id";
        break;
    case eRnaAlphabet:
        string_id_field = "rstring_id";
        seq_id_field = "rna_seq_id";
        break;
    default:
        Q_ASSERT_X(0, "SequenceImporter::insertSeqSubseqs", "Unrecognized alphabet");
        return QList<int>();
    }

    QStringList i_seq_fields;
    i_seq_fields << string_id_field << "label";
    QStringList i_subseq_fields;
    i_subseq_fields << seq_id_field << "label" << "start" << "stop" << "sequence";

    // Remember the list of subseq ids to associate when decorating the tree
    QList<int> subseq_ids;

    // loop through each parsedBioString and add to the various database tables as necessary
    for (int i=0, z=parsedBioStrings.size(); i<z; ++i)
    {
        // A. Get the appropriate seq_id via an AnonSeq object
        AnonSeq anonSeq = anonSeqFactories_[alphabet]->add(parsedBioStrings.at(i).bioString_);

        // B. Add to the appropriate Seq table
        DataRow seq;
        seq.setValue(string_id_field, anonSeq.id());
        seq.setValue("label", parsedBioStrings.at(i).header_);
        adocTreeModel_->adocDbDataSource_.insert(seq, constants::kTableHash[alphabet][eSeqGroup], i_seq_fields);
        Q_ASSERT_X(seq.id_.isNull() == false, "SequenceImporter::insertSeqSubseqs", "Seq id is not allowed to be empty");

        // C. Add to the appropriate Subseq table
        DataRow subseq;
        subseq.setValue(seq_id_field, seq.id_);
        subseq.setValue("label", QString("%1_%2-%3").arg(parsedBioStrings.at(i).header_.left(20)).arg(1).arg(anonSeq.bioString().length()));
        subseq.setValue("start", 1);
        subseq.setValue("stop", anonSeq.bioString().length());
        subseq.setValue("sequence", parsedBioStrings.at(i).bioString_.sequence());
        adocTreeModel_->adocDbDataSource_.insert(subseq, constants::kTableHash[alphabet][eSubseqGroup], i_subseq_fields);
        Q_ASSERT_X(subseq.id_.isNull() == false, "SequenceImporter::insertSeqSubseqs", "Subseq id is not allowed to be empty");

        subseq_ids.append(subseq.id_.toInt());
    }

    Q_ASSERT_X(subseq_ids.count() == parsedBioStrings.size(), "SequenceImporter::insertSeqSubseqs", "subseq_id list should equal number of parsedBioStrings");

    return subseq_ids;
}

/*
bool import(const QList<ParsedBioString> parsedBioStrings, Alphabet alphabet, DataFormatType dataFormatType, const QModelIndex parent)
{
    Q_ASSERT(alphabet != eUnknownAlphabet && alphabet != eAutoDetect);
    if (alphabet == eUnknownAlphabet
        || alphabet == eAutoDetect)
    {
        return false;
    }

    Q_ASSERT(dataFormatType != eUnknownFormatType && dataFormatType != eAutoDetectType);
    if (dataFormatType == eUnknownFormatType
        || dataFormatType == eAutoDetect)
    {
        return false;
    }

    if (!model_)
        return false;

    int project_id = model_->data(parentIndex, ProjectIdRole);
    SeqNode *prototype = prototypeSeqNodeHash_[Alphabet].data();
    DbAnonSeqFactory *factory = dbAnonSeqFactoryHash_[Alphabet].data();

    QList<AdocTreeNode *> nodes;
    foreach (ParsedBioString pbs, parsedBioStrings)
    {
        SeqNode *node = prototype->create();
        node->createRecord(database(), project_id, pbss.bioString_, factory);
        nodes.append(node);
    }
    model_->appendRows(nodes, parentIndex);

    if (Format == eClustalType)
    {
        // Create an alignment

    }

}
*/
