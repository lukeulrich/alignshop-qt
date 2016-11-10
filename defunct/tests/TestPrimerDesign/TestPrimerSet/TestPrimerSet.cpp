/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "primerset.h"

using namespace PrimerDesign;

class TestPrimerSet : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Properties
    void sequenceShouldBeSettable()
    {
        PrimerSet ps;
        DnaSeq seq;
        seq.setName("test");
        QVERIFY(ps.setSquence(seq));
        QVERIFY(ps.sequence().name() == seq.name());
    }
};

QTEST_MAIN(TestPrimerSet)
#include "TestPrimerSet.moc"
