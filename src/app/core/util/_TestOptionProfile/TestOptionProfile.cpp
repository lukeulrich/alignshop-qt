/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../OptionProfile.h"
#include "../../PODs/Option.h"
#include "../../PODs/OptionSpec.h"

typedef QVector<Option> OptionVector;

class TestOptionProfile : public QObject
{
    Q_OBJECT

public:
    TestOptionProfile()
    {
        qRegisterMetaType<OptionProfile>("OptionProfile");
        qRegisterMetaType<OptionVector>("OptionVector");
    }

private slots:
    void constructor();

    void add();
    void argumentList_data();
    void argumentList();
    void clear();
    void operatorltlt();
    void setJoinDelimiter();
    void setJoinEnabled();
    void validOption();
    void validOptions();
};

Q_DECLARE_METATYPE(OptionVector)
Q_DECLARE_METATYPE(OptionProfile)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestOptionProfile::constructor()
{
    OptionProfile x;

    QVERIFY(!x.isJoinEnabled());
    QCOMPARE(x.joinDelimiter(), QString(" "));
}

void TestOptionProfile::add()
{
    OptionProfile x;

    QVERIFY(x.isEmpty());

    x.add(OptionSpec());
    QVERIFY(x.isEmpty());

    x.add(OptionSpec("-num_threads"));
    QVERIFY(!x.isEmpty());

    OptionProfile y;
    y.add(OptionSpec("  -num_threads  "));
    QVERIFY(!y.isEmpty());
}

void TestOptionProfile::argumentList_data()
{
    QTest::addColumn<OptionProfile>("profile");
    QTest::addColumn<OptionVector>("options");
    QTest::addColumn<QStringList>("expect");

    QTest::newRow("empty state") << OptionProfile() << OptionVector() << QStringList();

    OptionProfile x;
    OptionSpec numThreads("-num_threads");
    numThreads.emptyValue_ = false;
    numThreads.isDefault_ = true;
    x << numThreads;
    QTest::newRow("1 default argument no value") << x << OptionVector() << (QStringList() << "-num_threads");

    x.clear();
    numThreads.defaultValue_ = "1";
    x << numThreads;
    QTest::newRow("1 default argument with value") << x << OptionVector() << (QStringList() << "-num_threads" << "1");

    OptionSpec fast("fast");
    fast.isDefault_ = true;
    fast.defaultValue_ = "Should not be seen";
    x << fast;
    QTest::newRow("1 default argument with value, another default without value")
            << x
            << OptionVector()
            << (QStringList() << "-num_threads" << "1" << "fast");

    QTest::newRow("Overriding default argument")
            << x
            << (OptionVector() << Option("-num_threads", "34"))
            << (QStringList() << "-num_threads" << "34" << "fast");

    QTest::newRow("Overriding default argument with invalid value")
            << x
            << (OptionVector() << Option("-num_threads", "abc"))
            << (QStringList() << "-num_threads" << "abc" << "fast");

    QTest::newRow("Should skip option not in profile")
            << x
            << (OptionVector() << Option("-swine", "abc"))
            << (QStringList() << "-num_threads" << "1" << "fast");

    QTest::newRow("Duplicates even though not allowed should be outputted")
            << x
            << (OptionVector() << Option("fast") << Option("fast"))
            << (QStringList() << "-num_threads" << "1" << "fast" << "fast");

    QTest::newRow("Option with value, but defined to have empty value")
            << x
            << (OptionVector() << Option("fast", "swine"))
            << (QStringList() << "-num_threads" << "1" << "fast");

    // Test: join related tests
    OptionProfile y;
    y << OptionSpec("-QUICKTREE");
    OptionSpec output("-OUTPUT", true, false, false);
    y << output;
    y.setJoinEnabled();
    QTest::newRow("join enabled, default delimiter")
            << y
            << (OptionVector() << Option("-OUTPUT", "FASTA") << Option("-QUICKTREE"))
            << (QStringList() << "-OUTPUT FASTA" << "-QUICKTREE");

    y.setJoinDelimiter("=");
    QTest::newRow("join enabled, = delimiter")
            << y
            << (OptionVector() << Option("-OUTPUT", "FASTA") << Option("-QUICKTREE"))
            << (QStringList() << "-OUTPUT=FASTA" << "-QUICKTREE");

    OptionSpec type("-TYPE");
    type.isDefault_ = true;
    type.defaultValue_ = "PROTEIN";
    type.emptyValue_ = false;
    y << type;
    y.setJoinDelimiter("@@");
    QTest::newRow("join enabled, @@ delimiter, default parameter")
            << y
            << (OptionVector() << Option("-OUTPUT", "FASTA") << Option("-QUICKTREE"))
            << (QStringList() << "-OUTPUT@@FASTA" << "-QUICKTREE" << "-TYPE@@PROTEIN");
}

void TestOptionProfile::argumentList()
{
    QFETCH(OptionProfile, profile);
    QFETCH(OptionVector, options);
    QFETCH(QStringList, expect);

    QStringList result = profile.argumentList(options);

    if (result.size() != expect.size())
    {
        qDebug() << "Actual" << result;
        qDebug() << "Expected" << expect;
    }

    QCOMPARE(result.size(), expect.size());

    qSort(result);
    qSort(expect);
    QCOMPARE(result, expect);
}

void TestOptionProfile::clear()
{
    OptionProfile x;

    x.add(OptionSpec("-db", true));
    QVERIFY(!x.isEmpty());
    x.clear();
    QVERIFY(x.isEmpty());
}

void TestOptionProfile::operatorltlt()
{
    OptionProfile x;

    x << OptionSpec("-evalue", false) << OptionSpec("-num_threads", true, false);
    QVERIFY(x.isEmpty() == false);
    x.clear();
    QVERIFY(x.isEmpty());
}

void TestOptionProfile::setJoinDelimiter()
{
    OptionProfile x;

    x.setJoinDelimiter("");

    QCOMPARE(x.joinDelimiter(), QString());
    x.setJoinDelimiter("=");
    QCOMPARE(x.joinDelimiter(), QString("="));
    x.setJoinDelimiter("blargh");
    QCOMPARE(x.joinDelimiter(), QString("blargh"));
}

void TestOptionProfile::setJoinEnabled()
{
    OptionProfile x;

    QVERIFY(x.isJoinEnabled() == false);
    x.setJoinEnabled();
    QVERIFY(x.isJoinEnabled());
}

void TestOptionProfile::validOption()
{
    OptionProfile x;

    QCOMPARE(x.validOption(Option()), false);
    QCOMPARE(x.validOption(Option("-num_threads")), false);
    QCOMPARE(x.validOption(Option("-num_threads", "3")), false);

    OptionSpec numThreads("-num_threads");
    numThreads.emptyValue_ = false;
    numThreads.valueRegex_ = QRegExp("^[1-9][0-9]*$");
    x << numThreads;

    QCOMPARE(x.validOption(Option()), false);
    QCOMPARE(x.validOption(Option("-num_threads")), false);
    QCOMPARE(x.validOption(Option("-num_threads", "abc")), false);
    QCOMPARE(x.validOption(Option("-num_threads", "01")), false);
    QCOMPARE(x.validOption(Option("-num_threads", "1")), true);
    QCOMPARE(x.validOption(Option("-num_threads", "23")), true);
    QCOMPARE(x.validOption(Option("  -num_threads  ", "23")), true);

    // Test: empty value
    x << OptionSpec("-h");
    QCOMPARE(x.validOption(Option("-h")), true);
    QCOMPARE(x.validOption(Option("-h", "blah")), true);
    QCOMPARE(x.validOption(Option("-h", "123")), true);

    // Test: non-options
    QCOMPARE(x.validOption(Option("nothing")), false);
    QCOMPARE(x.validOption(Option("-invalid_flag", "234")), false);

    // Test: setting an OptionSpec with the same name as one already in the profile should use the last version
    x << OptionSpec("-h", true, false, false, QString(), QRegExp("^ABC|DEF$"));
    QCOMPARE(x.validOption(Option("-h")), false);
    QCOMPARE(x.validOption(Option("-h", "blah")), false);
    QCOMPARE(x.validOption(Option("-h", "123")), false);

    QCOMPARE(x.validOption(Option("-h", "ABC")), true);
    QCOMPARE(x.validOption(Option("-h", "DEF")), true);

    // Test: clearing
    x.clear();
    QCOMPARE(x.validOption(Option("-num_threads", "1")), false);
    QCOMPARE(x.validOption(Option("-num_threads", "23")), false);
    QCOMPARE(x.validOption(Option("-h", "ABC")), false);
    QCOMPARE(x.validOption(Option("-h", "DEF")), false);

    // Test: default value but no value provided for the option
    x << OptionSpec("-num_threads", false, false, true, "1", QRegExp("^[1-9][0-9]*$"));
    QCOMPARE(x.validOption(Option("-num_threads")), false);
}

void TestOptionProfile::validOptions()
{
    OptionProfile x;

    // Test: no options
    QCOMPARE(x.validOptions(QVector<Option>()), false);

    // Test: single option
    OptionSpec spec("-num_threads");
    spec.emptyValue_ = false;
    spec.valueRegex_ = QRegExp("^[1-9][0-9]*$");
    x << spec;

    QCOMPARE(x.validOptions(QVector<Option>() << Option("-num_threads")), false);
    QCOMPARE(x.validOptions(QVector<Option>() << Option("-num_threads", "abc")), false);
    QCOMPARE(x.validOptions(QVector<Option>() << Option("-num_threads", "21")), true);

    // Test: duplicates should fail
    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "21")
                            << Option("-num_threads", "34")), false);

    // Test: allow duplicates
    x.clear();
    spec.allowDuplicate_ = true;
    x << spec;
    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "21")
                            << Option("-num_threads", "34")), true);

    // Test: multiple options
    x.clear();
    spec.allowDuplicate_ = false;
    x << spec << OptionSpec("-h");
    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")
                            << Option("-h")), true);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-h")), true);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")), true);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-bad_option", "PAS")
                            << Option("-num_threads", "98")
                            << Option("-h")), false);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")
                            << Option("-bad_option", "PAS")
                            << Option("-h")), false);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")
                            << Option("-h")
                            << Option("-bad_option", "PAS")), false);

    // Test: required option that is not provided
    x << OptionSpec("-required", true);
    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")
                            << Option("-h")), false);

    QCOMPARE(x.validOptions(QVector<Option>()
                            << Option("-num_threads", "98")
                            << Option("-h")
                            << Option("-required")), true);
}


QTEST_APPLESS_MAIN(TestOptionProfile)
#include "TestOptionProfile.moc"
