/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#include "PrimerDesignFile.h"
#include <QtCore>
#include <QtXml>

namespace PrimerDesign
{
    namespace PrimerDesignFields
    {
        const QString ROOT = "Project";
        const QString PAIR = "Pair";
        const QString PAIRS = "Pairs";
        const QString PARAM = "Param";
        const QString PARAM_ID = "ParamId";
        const QString PARAMS = "Params";
        const QString SEQUENCE = "Sequence";
        const QString AMPLICON = "Amplicon";
        const QString AMPLICON_BOUNDS = "AmpliconBounds";
        const QString AMPLICON_SIZE_RANGE = "AmpliconSizeRange";
        const QString FORWARD = "Forward";
        const QString FORWARD_PREFIX = "ForwardPrefix";
        const QString FORWARD_SUFFIX = "ForwardSuffix";
        const QString NAME = "Name";
        const QString NOTES = "Notes";
        const QString PRIMER_SIZE_RANGE = "PrimerSizeRange";
        const QString REVERSE = "Reverse";
        const QString REVERSE_PREFIX = "ReversePrefix";
        const QString REVERSE_SUFFIX = "ReverseSuffix";
        const QString TM = "Tm";
        const QString TM_RANGE = "TmRange";
        const QString SEQUENCE_POSITION = "SeqPosition";
        const QString SODIUM_CONCENTRATION = "SodiumConcentration";
    }
}

using namespace PrimerDesign;
using namespace PrimerDesign::PrimerDesignFields;

static QString toString(const Range &r)
{
    return QString::number(r.min()) + "," + QString::number(r.max());
}

static QString toString(const RangeF &r)
{
    return QString::number(r.min()) + "," + QString::number(r.max());
}

static Range parseRange(const QString &value)
{
    QStringList values = value.split(",");
    return Range(values[0].toInt(), values.length() > 1 ? values[1].toInt() : 0);
}

static RangeF parseRangeF(const QString &value)
{
    QStringList values = value.split(",");
    return RangeF(values[0].toDouble(), values.length() > 1 ? values[1].toDouble() : 0);
}

static QString fetch(const QString &name, const QDomElement &node)
{
    return node.namedItem(name).toElement().text();
}

static void writeDetails(DnaSequence &sequence, QXmlStreamWriter &writer)
{
    writer.writeTextElement(NAME, sequence.name());
    writer.writeTextElement(SEQUENCE, sequence.sequence());
    writer.writeTextElement(NOTES, sequence.notes());
}

static void writeDetails(Primer &primer, QXmlStreamWriter &writer)
{
    writer.writeTextElement(SEQUENCE, primer.sequence());
    writer.writeTextElement(TM, QString::number(primer.tm()));
    writer.writeTextElement(SEQUENCE_POSITION, QString::number(primer.sequencePosition()));
}

static void writeDetails(PrimerPair *pair, QXmlStreamWriter &writer)
{
    writer.writeStartElement(PAIR);

    writer.writeTextElement(PARAM_ID, QString::number((long)pair->params()));
    writer.writeTextElement(NAME, pair->name());
    writer.writeStartElement(FORWARD);
    writeDetails(pair->forwardPrimer(), writer);
    writer.writeEndElement();

    writer.writeStartElement(REVERSE);
    writeDetails(pair->reversePrimer(), writer);
    writer.writeEndElement();

    writer.writeEndElement();
}

static void writeDetails(PrimerDesignInput *param, QXmlStreamWriter &writer)
{
    writer.writeStartElement(PARAM);
    writer.writeTextElement(PARAM_ID, QString::number((long)param));
    writer.writeTextElement(AMPLICON_BOUNDS, toString(param->ampliconBounds));
    writer.writeTextElement(AMPLICON_SIZE_RANGE, toString(param->ampliconSizeRange));
    writer.writeTextElement(PRIMER_SIZE_RANGE, toString(param->primerSizeRange));
    writer.writeTextElement(TM_RANGE, toString(param->tmRange));
    writer.writeTextElement(FORWARD_PREFIX, param->forwardPrefix);
    writer.writeTextElement(REVERSE_PREFIX, param->reversePrefix);
    writer.writeTextElement(FORWARD_SUFFIX, param->forwardSuffix);
    writer.writeTextElement(REVERSE_SUFFIX, param->reverseSuffix);
    writer.writeTextElement(SODIUM_CONCENTRATION, QString::number(param->sodiumConcentration));
    writer.writeEndElement();
}

static void writeDetails(ObservablePrimerPairList &pairs, QXmlStreamWriter &writer)
{
    QHash<long, PrimerDesignInput *> params;

    for (int i = 0; i < pairs.length(); ++i)
    {
        PrimerPair *pair = pairs.at(i);
        long paramId = (long)(pair->params());
        if (!params.contains(paramId))
        {
            params[paramId] = pair->params();
            writeDetails(pair->params(), writer);
        }

        writeDetails(pair, writer);
    }
}

QString PrimerDesignFile::serialize(ObservablePrimerPairGroupList &data)
{
    QString buffer;
    QXmlStreamWriter writer(&buffer);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);
    writer.writeStartElement(ROOT);

    for (int i = 0; i < data.length(); ++i)
    {
        writer.writeStartElement(SEQUENCE);
        writeDetails(data.at(i)->sequence, writer);
        writer.writeStartElement(PARAMS);
        writeDetails(data.at(i)->pairs, writer);
        writer.writeEndElement();
        writer.writeEndElement();
    }

    writer.writeEndElement();

    return buffer;
}

static void read(DnaSequence &sequence, const QDomElement &data)
{
    sequence.setName(fetch(NAME, data));
    sequence.setSequence(fetch(SEQUENCE, data));
    sequence.setNotes(fetch(NOTES, data));
}

static PrimerDesignInput *readParam(const QDomElement &data)
{
    PrimerDesignInput *param = new PrimerDesignInput();
    param->ampliconBounds = parseRange(fetch(AMPLICON_BOUNDS, data));
    param->ampliconSizeRange = parseRange(fetch(AMPLICON_SIZE_RANGE, data));
    param->primerSizeRange = parseRange(fetch(PRIMER_SIZE_RANGE, data));
    param->tmRange = parseRangeF(fetch(TM_RANGE, data));
    param->forwardPrefix = fetch(FORWARD_PREFIX, data);
    param->reversePrefix = fetch(REVERSE_PREFIX, data);
    param->forwardSuffix = fetch(FORWARD_SUFFIX, data);
    param->reverseSuffix = fetch(REVERSE_SUFFIX, data);
    param->sodiumConcentration = fetch(SODIUM_CONCENTRATION, data).toFloat();
    return param;
}

static Primer readPrimer(const QDomElement &data)
{
    Primer primer;
    primer.setSequence(fetch(SEQUENCE, data));
    primer.setTm(fetch(TM, data).toDouble());
    primer.setSequencePosition(fetch(SEQUENCE_POSITION, data).toInt());
    return primer;
}

static PrimerPair *readPair(const QDomElement &data, QHash<QString, PrimerDesignInputRef> &params)
{
    PrimerDesignInputRef &param = params[fetch(PARAM_ID, data)];
    PrimerPair *pair = new PrimerPair(param);
    pair->setName(fetch(NAME, data));
    pair->setForwardPrimer(readPrimer(data.firstChildElement(FORWARD)));
    pair->setReversePrimer(readPrimer(data.firstChildElement(REVERSE)));
    return pair;
}

static QHash<QString, PrimerDesignInputRef> readParams(QDomElement &data)
{
    QHash<QString, PrimerDesignInputRef> params;
    QDomNodeList paramNodes = data.elementsByTagName(PARAM);

    for (int i = 0; i < paramNodes.count(); ++i)
    {
        QDomElement paramNode = paramNodes.at(i).toElement();
        params[fetch(PARAM_ID, paramNode)] = PrimerDesignInputRef(readParam(paramNode));
    }

    return params;
}

static void read(ObservablePrimerPairList &pairs, QDomElement &data)
{
    QHash<QString, PrimerDesignInputRef> params = readParams(data);
    QDomNodeList pairNodes = data.elementsByTagName(PAIR);

    for (int i = 0; i < pairNodes.count(); ++i)
    {
        pairs.add(readPair(pairNodes.at(i).toElement(), params));
    }
}

void PrimerDesignFile::deserialize(
    ObservablePrimerPairGroupList &list, const QString &data)
{
    QDomDocument doc;
    doc.setContent(data);
    QDomNodeList nodes = doc.elementsByTagName(ROOT);

    if (!nodes.isEmpty())
    {
        QDomElement seq = nodes.at(0).toElement()
            .firstChildElement(SEQUENCE);

        while (!seq.isNull())
        {
            PrimerPairGroup *group = new PrimerPairGroup();
            read(group->sequence, seq);
            read(group->pairs, seq);
            list.add(group);
            seq = seq.nextSiblingElement(SEQUENCE);
        }
    }
}
