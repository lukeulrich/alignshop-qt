/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QHash>

#include "BlastXmlHandler.h"
#include "../../BioString.h"

enum BlastFields
{
    eHitId = 0,
    eHitDefinition,
    eHitAccession,
    eHitLength,
    eHspBitScore,
    eHspScore,
    eHspEvalue,
    eHspQueryFrom,
    eHspQueryTo,
    eHspHitFrom,
    eHspHitTo,
    eHspQueryFrame,
    eHspHitFrame,
    eHspIdentities,
    eHspPositives,
    eHspGaps,
    eHspAlignLength,
    eHspQuerySequence,
    eHspSubjectSequence,
    eHspMidline
};

QHash<QString, int> initializeBlastFieldHash()
{
    QHash<QString, int> hash;
    hash.insert("Hit_id",           eHitId);
    hash.insert("Hit_def",          eHitDefinition);
    hash.insert("Hit_accession",    eHitAccession);
    hash.insert("Hit_len",          eHitLength);
    hash.insert("Hsp_bit-score",    eHspBitScore);
    hash.insert("Hsp_score",        eHspScore);
    hash.insert("Hsp_evalue",       eHspEvalue);
    hash.insert("Hsp_query-from",   eHspQueryFrom);
    hash.insert("Hsp_query-to",     eHspQueryTo);
    hash.insert("Hsp_hit-from",     eHspHitFrom);
    hash.insert("Hsp_hit-to",       eHspHitTo);
    hash.insert("Hsp_query-frame",  eHspQueryFrame);
    hash.insert("Hsp_hit-frame",    eHspHitFrame);
    hash.insert("Hsp_identity",     eHspIdentities);
    hash.insert("Hsp_positive",     eHspPositives);
    hash.insert("Hsp_gaps",         eHspGaps);
    hash.insert("Hsp_align-len",    eHspAlignLength);
    hash.insert("Hsp_qseq",         eHspQuerySequence);
    hash.insert("Hsp_hseq",         eHspSubjectSequence);
    hash.insert("Hsp_midline",      eHspMidline);

    return hash;
}
// Optimization to facilitate mapping string to integers during parsing
static const QHash<QString, int> kBlastFieldHash_(initializeBlastFieldHash());


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
BlastXmlHandler::BlastXmlHandler(Grammar grammar)
    : grammar_(grammar)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param text [const QString &]
  * @returns bool
  */
bool BlastXmlHandler::characters(const QString &text)
{
    text_ += text;

    return true;
}

/**
  * @param namespaceURI [const QString &]
  * @param localName [const QString &]
  * @param qName [const QString &]
  * @returns bool
  */
bool BlastXmlHandler::endElement(const QString & /* namespaceURI */,
                                 const QString & /* localName */,
                                 const QString &qName)
{
    switch (kBlastFieldHash_.value(qName, -1))
    {
    case eHitId:
        hits_.last().id_ = text_;
        break;
    case eHitDefinition:
        hits_.last().definition_ = text_;
        break;
    case eHitAccession:
        hits_.last().accession_ = text_;
        break;
    case eHitLength:
        hits_.last().length_ = text_.toInt();
        break;
    case eHspBitScore:
        hits_.last().hsps_.last().bitScore_ = text_.toDouble();
        break;
    case eHspScore:
        hits_.last().hsps_.last().rawScore_ = text_.toInt();
        break;
    case eHspEvalue:
        hits_.last().hsps_.last().evalue_ = text_.toDouble();
        break;
    case eHspQueryFrom:
        hits_.last().hsps_.last().queryRange_.begin_ = text_.toInt();
        break;
    case eHspQueryTo:
        hits_.last().hsps_.last().queryRange_.end_ = text_.toInt();
        break;
    case eHspHitFrom:
        hits_.last().hsps_.last().subjectRange_.begin_ = text_.toInt();
        break;
    case eHspHitTo:
        hits_.last().hsps_.last().subjectRange_.end_ = text_.toInt();
        break;
    case eHspQueryFrame:
        hits_.last().hsps_.last().queryFrame_ = text_.toInt();
        break;
    case eHspHitFrame:
        hits_.last().hsps_.last().subjectFrame_ = text_.toInt();
        break;
    case eHspIdentities:
        hits_.last().hsps_.last().identities_ = text_.toInt();
        break;
    case eHspPositives:
        hits_.last().hsps_.last().positives_ = text_.toInt();
        break;
    case eHspGaps:
        hits_.last().hsps_.last().gaps_ = text_.toInt();
        break;
    case eHspAlignLength:
        hits_.last().hsps_.last().length_ = text_.toInt();
        break;
    case eHspQuerySequence:
        hits_.last().hsps_.last().queryAlignment_ = BioString(text_.toAscii(), grammar_);
        break;
    case eHspSubjectSequence:
        hits_.last().hsps_.last().subjectAlignment_ = BioString(text_.toAscii(), grammar_);
        break;
    case eHspMidline:
        hits_.last().hsps_.last().midline_ = text_.toAscii();
        break;

    default:
        break;
    }

    return true;
}

/**
  * @returns QString
  */
QString BlastXmlHandler::errorString()
{
    if (errorString_.isEmpty())
        return QXmlDefaultHandler::errorString();

    return errorString_;
}

/**
  * This method is called both when there is a problem with XML document itself or we trigger an error by returning
  * false from one of the handler methods. If the latter occurred, then errorString_ should have been set to the
  * relevant error message and thus will not be empty.
  *
  * @param exception [const QXmlParseException &]
  * @returns bool
  */
bool BlastXmlHandler::fatalError(const QXmlParseException &exception)
{
    if (errorString_.isEmpty())
    {
        errorString_ = QString("Error parsing BLAST XML file at line %1, column %2: %3")
                       .arg(exception.lineNumber())
                       .arg(exception.columnNumber())
                       .arg(exception.message());
    }

    return false;
}

/**
  * @param namespaceURI [const QString &]
  * @param localName [const QString &]
  * @param qName [const QString &]
  * @param atts [const QXmlAttributes &]
  * @returns bool
  */
bool BlastXmlHandler::startElement(const QString & /* namespaceURI */,
                                   const QString & /* localName */,
                                   const QString &qName,
                                   const QXmlAttributes & /* atts */)
{
    static bool seenBlastOutput = false;
    if (!seenBlastOutput && qName != "BlastOutput")
    {
        errorString_ = QString("BLAST XML is not valid");
        return false;
    }
    else if (qName == "BlastOutput")
    {
        seenBlastOutput = true;
        return true;
    }

    if (qName == "Iteration")
        hits_.clear();
    else if (qName == "Hit")
        hits_ << HitPod();
    else if (qName == "Hsp")
        hits_.last().hsps_ << HspPod();

    text_.clear();

    return true;
}

/**
  * @returns QVector<HitPod>
  */
QVector<HitPod> BlastXmlHandler::hits() const
{
    return hits_;
}
