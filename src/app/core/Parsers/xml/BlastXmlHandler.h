/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef BLASTXMLHANDLER_H
#define BLASTXMLHANDLER_H

#include <QtCore/QString>
#include <QtXml/QXmlDefaultHandler>

#include "../../PODs/HitPod.h"

/**
  *
  *
  * Only keeps the hits from the last iteration.
  */
class BlastXmlHandler : public QXmlDefaultHandler
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    BlastXmlHandler(Grammar grammar);

    // ------------------------------------------------------------------------------------------------
    // Reimplemented virtual methods
    bool characters(const QString &text);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    QString errorString();
    bool fatalError(const QXmlParseException &exception);
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);


    // ------------------------------------------------------------------------------------------------
    // Public methods
    QVector<HitPod> hits() const;


private:
    Grammar grammar_;
    QString text_;
    QVector<HitPod> hits_;
    QString errorString_;
};

#endif // BLASTXMLHANDLER_H
