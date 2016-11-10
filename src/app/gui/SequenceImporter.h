/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEQUENCEIMPORTER_H
#define SEQUENCEIMPORTER_H

#include <QtCore/QObject>
#include <QtCore/QModelIndex>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QVector>

#include "../core/Alphabet.h"
#include "../core/Detectors/DataFormatDetector.h"
#include "../core/DataFormat.h"
#include "../core/PODs/SequenceParseResultPod.h"
#include "../core/PODs/SimpleSeqPod.h"
#include "../core/global.h"

class QProgressDialog;

class Adoc;
class AdocTreeModel;
class IAlphabetDetector;
class IDataFormatDetector;

class SequenceImporter : public QObject
{
    Q_OBJECT

public:
    SequenceImporter(QWidget *parent = nullptr);
    ~SequenceImporter();

    QVector<DataFormat> dataFormats() const;
    void importFile(const QString &fileName, Adoc *adoc, AdocTreeModel *adocTreeModel, const QModelIndex &destination);

Q_SIGNALS:
    void importCanceled();
    void importError(const QString &error);
    void importSuccessful(const QModelIndex &parentTreeIndex);

private Q_SLOTS:
    void onParseSuccess(SequenceParseResultPod parsePod);
    void onParserProgressChanged(int currentStep, int totalSteps);

private:
    // ------------------------------------------------------------------------------------------------
    // Private methods
    Alphabet askUserForAlphabet() const;
    void upperCaseSequenceData(SequenceParseResultPod &parsePod) const;

    // ------------------------------------------------------------------------------------------------
    // Private members
    QVector<DataFormat> dataFormats_;
    DataFormatDetector dataFormatDetector_;
    QProgressDialog *parseProgressDialog_;
    IAlphabetDetector *alphabetDetector_;           // For the consensus detector

    // Temporary variables used during the import
    QThread thread_;
    struct ImportContext
    {
        Adoc *adoc_;
        AdocTreeModel *adocTreeModel_;
        QModelIndex destination_;
        QString importFileName_;
    };
    ImportContext importContext_;
};

#endif // SEQUENCEIMPORTER_H
