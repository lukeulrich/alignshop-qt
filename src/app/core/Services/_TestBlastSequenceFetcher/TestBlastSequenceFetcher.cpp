/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QVector>

#include <QtTest/QtTest>

#include "../BlastSequenceFetcher.h"
#include "../../BioString.h"
#include "../../enums.h"

class TestBlastSequenceFetcher : public QObject
{
    Q_OBJECT

public:
    TestBlastSequenceFetcher()
    {
        qRegisterMetaType<BioStringVector>("BioStringVector");
        qRegisterMetaType<Grammar>("Grammar");
    }

private slots:
    void fetch_data();
    void fetch();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBlastSequenceFetcher::fetch_data()
{
    QTest::addColumn<QStringList>("ids");
    QTest::addColumn<QString>("blastDbPath");
    QTest::addColumn<Grammar>("grammar");
    QTest::addColumn<bool>("shouldError");
    QTest::addColumn<BioStringVector>("bioStringVector");

    // Assertion: prevents sending empty id list
    QTest::newRow("empty blast db") << (QStringList() << "1") << QString() << eUnknownGrammar << true << BioStringVector();
    QTest::newRow("missing blast db") << (QStringList() << "1") << QString("missing") << eUnknownGrammar << true << BioStringVector();
    QTest::newRow("missing blast db") << (QStringList() << "1") << QString("files/invalid_db") << eUnknownGrammar << true << BioStringVector();

    QTest::newRow("valid db, invalid id") << (QStringList() << "1") << QString("files/prf") << eUnknownGrammar << true << BioStringVector();

    // Database: prf
    // Also tests for whether the error message: "BLAST query/options error: Entry not found in BLAST database" is
    // properly ignored
    QTest::newRow("prf - not found ids") << (QStringList() << "prf||34" << "prf||ABC")
                                         << QString("files/prf")
                                         << eUnknownGrammar
                                         << false
                                         << (BioStringVector()
                                             << BioString(eUnknownGrammar)
                                             << BioString(eUnknownGrammar));

    QTest::newRow("prf - id: 2114402F") << (QStringList() << "prf||2114402F")
                                        << QString("files/prf")
                                        << eAminoGrammar
                                        << false
                                        << (BioStringVector()
                                            << BioString("MNFGQNLYNWFLSNAQSLVLLAIVVIGLYLGFKREFSKLIGFLIIAIIAVGLVFNAAGVKDILLELFNRIIGA", eAminoGrammar));

    QTest::newRow("prf - multi id") << (QStringList() << "prf||1813280A" << "prf||2114402F")
                                    << QString("files/prf")
                                    << eAminoGrammar
                                    << false
                                    << (BioStringVector()
                                        << BioString("MPDYLGADQRKTKEDEKDDKPIRALDEGDIALLKTYGQSTYSRQIKQVEDDIQQLLKKINELTGIKESDTGLAPPALWDLAADKQTLQSEQPLQVARCTKIINADSEDPKYIINVKQFAKFVVDLSDQVAPTDIEEGMRVGVDRNKYQIHIPLPPKIDPTVTMMQVEEKPDVTYSDVGGCKEQIEKLREVVETPLLHPERFVNLGIEPPKGVLLFGPPGTGKTLCARAVANRTDACFIRVIGSELVQKYVGEGARMVRELFEMARTKKACLIFFDEIDAIGGARFDDGAGGDNEVQRTMLELINQLDGFDPRGNIKVLMATNRPDTLDPALMRPGRLDRKIEFSLPDLEGRTHIFKIHARSMSVERDIRFELLARLCPNSTGAEIRSVCTEAGMFAIRARRKIATEKDFLEAVNKVIKSYAKFSATPRYMTYN", eAminoGrammar)
                                        << BioString("MNFGQNLYNWFLSNAQSLVLLAIVVIGLYLGFKREFSKLIGFLIIAIIAVGLVFNAAGVKDILLELFNRIIGA", eAminoGrammar));

    QTest::newRow("prf - found + not found id") << (QStringList() << "prf||2114402F" << "prf||234sdf")
                                                << QString("files/prf")
                                                << eDnaGrammar      // Just to see if the grammar is appropriately carried over
                                                << false
                                                << (BioStringVector()
                                                    << BioString("MNFGQNLYNWFLSNAQSLVLLAIVVIGLYLGFKREFSKLIGFLIIAIIAVGLVFNAAGVKDILLELFNRIIGA", eDnaGrammar)
                                                    << BioString(eDnaGrammar));

    // Database: pir
    QTest::newRow("pir - id: T49728") << (QStringList() << "pir||T49728")
                                      << QString("files/pir")
                                      << eRnaGrammar
                                      << false
                                      << (BioStringVector()
                                          << BioString("MTVVQMTSVEHAADIQGHTYLRGPSSSRFGCRGPLVSLPATARVTIHFVPRGAVQPPPPHRLSSKELDPCHKNRANKRTSMELSKLHFHELQTVSPVAATKTDAVSGLARAPVPVPFRAAEASSTENIRIDLRSLAGVLTLLNPDCTNYASTPVMNLGVETWPPNAARCSPPTVPPRFVAC", eRnaGrammar));

    // Database: gi
    QTest::newRow("gi - invalid id") << (QStringList() << "gi|abc")
                                     << QString("files/gi")
                                     << eAminoGrammar
                                     << true
                                     << BioStringVector();
    QTest::newRow("gi - invalid id among good ones") << (QStringList() << "gi|123" << "gi|15674171" << "gi|abc")
                                     << QString("files/gi")
                                     << eAminoGrammar
                                     << true
                                     << BioStringVector();
    QTest::newRow("gi - missing id") << (QStringList() << "gi|123")
                                     << QString("files/gi")
                                     << eAminoGrammar
                                     << false
                                     << (BioStringVector()
                                         << BioString(eAminoGrammar));
    QTest::newRow("gi - good ids") << (QStringList() << "gi|22124775" << "gi|987")
                                   << QString("files/gi")
                                   << eAminoGrammar
                                   << false
                                   << (BioStringVector()
                                       << BioString("RSINIAPESTISKIMMQLSGNLMKETNESRLLTTGVTRRKLVQTTLVGGLAMATGAFSLPFSRTARAVQSALNPTSANDGKVIWSACTVNCGSRCPLRMHVADGEIKYVETDNTGDDDFEGLHQVRACLRGRSMRRRVYNPDRLKYPMKRIGARGEGKFKRISWEEAFETIAGSM", eAminoGrammar)
                                       << BioString(eAminoGrammar));

    // Database: swissprot
    QTest::newRow("sp - sp|Q6GZX4") << (QStringList() << "sp|Q6GZX4")
                                    << QString("files/swissprot")
                                    << eUnknownGrammar
                                    << false
                                    << (BioStringVector()
                                        << BioString("MAFSAEDVLKEYDRRRRMEALLLSLYYPNDRKLLDYKEWSPPRVQVECPKAPVEWNNPPSEKGLIVGHFSGIKYKGEKAQASEVDVNKMCCWVSKFKDAMRRYQGIQTCKIPGKVLSDLDAKIKAYNLTVEGVEGFVRYSRVTKQHVAAFLKELRHSKQYENVNLIHYILTDKRVDIQHLEKDLVKDFKALVESAHRMRQGHMINVKYILYQLLKKHGHGPDGPDILTVKTGSKGVLYDDSFRKIYTDLGWKFTPL", eUnknownGrammar));

    QTest::newRow("sp - sp|Q6GZX4|001R_FRG3G") << (QStringList() << "sp|Q6GZX4|001R_FRG3G")
                                    << QString("files/swissprot")
                                    << eUnknownGrammar
                                    << false
                                    << (BioStringVector()
                                        << BioString("MAFSAEDVLKEYDRRRRMEALLLSLYYPNDRKLLDYKEWSPPRVQVECPKAPVEWNNPPSEKGLIVGHFSGIKYKGEKAQASEVDVNKMCCWVSKFKDAMRRYQGIQTCKIPGKVLSDLDAKIKAYNLTVEGVEGFVRYSRVTKQHVAAFLKELRHSKQYENVNLIHYILTDKRVDIQHLEKDLVKDFKALVESAHRMRQGHMINVKYILYQLLKKHGHGPDGPDILTVKTGSKGVLYDDSFRKIYTDLGWKFTPL", eUnknownGrammar));

    QTest::newRow("sp - invalid format id") << (QStringList() << "ABFA_ASPKA")
                                            << QString("files/swissprot")
                                            << eUnknownGrammar
                                            << true
                                            << BioStringVector();

    // Database: gi_mixed
    QTest::newRow("gi_mixed - gi|341852525") << (QStringList() << "gi|341852525")
                                             << QString("files/gi_mixed")
                                             << eAminoGrammar
                                             << false
                                             << (BioStringVector()
                                                 << BioString("MKKIEAWLSKKGLKNKRTLIVVIAFVLFIIFLFLLLNSNSEDSGNITITENAELRTGPNAAYPVIYKVEKGDHFKKIGKVGKWIEVEDTSSNEKGWIAGWHTNLDIVADNTKEKNPLQGKTIVLDPGHGGSDQGASSNTKYKSLEKDYTLKTAKELQRTLEKEGATVKMTRTDDTYVSLENRDIKGDAYLSIHNDALESSNANGMTVYWYHDNQRALADTLDATIQKKGLLSNRGSRQENYQVLRQTKVPAVLLELGYISNPTDETMIKDQLHRQILEQAIVDGLKIYFSA", eAminoGrammar));

    QTest::newRow("gi_mixed - gb|EGS93414.1") << (QStringList() << "gb|EGS93414.1")
                                             << QString("files/gi_mixed")
                                             << eAminoGrammar
                                             << false
                                             << (BioStringVector()
                                                 << BioString("MKKIEAWLSKKGLKNKRTLIVVIAFVLFIIFLFLLLNSNSEDSGNITITENAELRTGPNAAYPVIYKVEKGDHFKKIGKVGKWIEVEDTSSNEKGWIAGWHTNLDIVADNTKEKNPLQGKTIVLDPGHGGSDQGASSNTKYKSLEKDYTLKTAKELQRTLEKEGATVKMTRTDDTYVSLENRDIKGDAYLSIHNDALESSNANGMTVYWYHDNQRALADTLDATIQKKGLLSNRGSRQENYQVLRQTKVPAVLLELGYISNPTDETMIKDQLHRQILEQAIVDGLKIYFSA", eAminoGrammar));

    // Database: local
    QTest::newRow("local - ABC") << (QStringList() << "lcl|ABC")
                                 << QString("files/local")
                                 << eRnaGrammar
                                 << false
                                 << (BioStringVector()
                                     << BioString("MEIVTEGLRFPEGPIAMPDGSVILVEIEAQQLTRVLPDGTKQLVAKIGGGPNGAAMGPDGKIYVCNNGGFEYHDENGFLTPAGIAKDYVGGSIQRVDPDTGEVETLYNDGDFGCILRGPNDIQFDAHGGFWFTDHGKTDYEKRCHDIVGIFYAKADGSHLEEVIFPSNNPNGIGISPDGNTLYAAETFTCRLMKFNITAPGRVAPDAGPGGPGIPLYRPSGISFSTALPWKNAAIFALPTIRQGREPP", eRnaGrammar));

    QTest::newRow("local - 9 + 345325") << (QStringList() << "lcl|345325" << "lcl|9")
                                        << QString("files/local")
                                        << eDnaGrammar
                                        << false
                                        << (BioStringVector()
                                            << BioString("MIKEEVFRPKKEYKHLGYNQLSLRHAVSQAVGLNAPGGTIVLYVAGTAALLTFTFSKYPDGAFSIPLILLLALIVYSMMSYSSFEFSKYLSSSGGYYTFVANGLGKGFGLTTALSYISYQILSFTGFGILGFIGFAYAILPSLGITVPYVNILWIPVTIIFILFVSFLIYKGIKPSLKYVTYAILIEVIFFIASSVYLIGVNHTKISIKPFTAIPVGGNFIILAAMMVYAIGSFVGVGGSIPIAEETKNPKKTVPRSIIASIAILGVTIILAAYAEVISWGYGNMASFGTGSGIGAYPVLSIYKYGFSGMGLVPFAVLLIIVINSFFTATVSLGTNASRVIFSLSREGVIPEKLSRTNTRGVPVYAILFITIVSLVIVLATGISFELLYPGKIIDALLYSSVFLLVLESPISYIVHILTNTSLHMYLKKRKMKTHIFRHIIIPGISSITLVGAIIAAVYFDLSAPYIYGVYGALVWVIVIAIVVIIMYTKYNKNLDDIGNFSL", eDnaGrammar)
                                            << BioString("KPVVRLAETSRRHRLTQKLEKDGITAAAIHGNKSQGARTRALADFKQGQVRVLVATDIAARGLDIKQLPQVVNFELPNVPEDYVHRIGRTGRAGESGHALSLVSADELKMLVGIEKLIKKQLPRKEMEGFEPKNNVALKPKAKADPSKARNRSGGNGRPAGKPRSFGDKPGGRSGGRSQSGNGQRGRSAQSQSS", eDnaGrammar));

    // Database: gnl using BL_ORD_ID method
    QTest::newRow("gnl - ord id 2") << (QStringList() << "gnl|BL_ORD_ID|2")
                                    << QString("files/gnl")
                                    << eAminoGrammar
                                    << false
                                    << (BioStringVector()
                                        << BioString("MKKIEAWLSKKGLKNKRTLIVVIAFVLFIIFLFLLLNSNSEDSGNITITENAELRTGPNAAYPVIYKVEKGDHFKKIGKVGKWIEVEDTSSNEKGWIAGWHTNLDIVADNTKEKNPLQGKTIVLDPGHGGSDQGASSNTKYKSLEKDYTLKTAKELQRTLEKEGATVKMTRTDDTYVSLENRDIKGDAYLSIHNDALESSNANGMTVYWYHDNQRALADTLDATIQKKGLLSNRGSRQENYQVLRQTKVPAVLLELGYISNPTDETMIKDQLHRQILEQAIVDGLKIYFSA", eAminoGrammar));

}

void TestBlastSequenceFetcher::fetch()
{
    QFETCH(QStringList, ids);
    QFETCH(QString, blastDbPath);
    QFETCH(Grammar, grammar);
    QFETCH(bool, shouldError);
    QFETCH(BioStringVector, bioStringVector);

    BlastSequenceFetcher x;
    try
    {
        x.setBlastDbCmdPath("./blastdbcmd");

        QSignalSpy spyError(&x, SIGNAL(error(int,QString)));
        QSignalSpy spyFinished(&x, SIGNAL(finished(BioStringVector)));

        QEventLoop eventLoop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&x, SIGNAL(error(int,QString)), &eventLoop, SLOT(quit()));
        connect(&x, SIGNAL(finished(int,QByteArray)), &eventLoop, SLOT(quit()));
        connect(&x, SIGNAL(finished(BioStringVector)), &eventLoop, SLOT(quit()));
        connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));

        x.fetch(ids, blastDbPath, grammar);

        // It's possible that we have already encountered an error before the process has started (e.g. no blast database specified)
        // and thus it may not even be necessary to start the event loop.
        if (!shouldError || spyError.isEmpty())
        {
            timer.start(10000); // 10 s timeout
            eventLoop.exec();
            QVERIFY(timer.isActive());
            timer.stop();
            QVERIFY(!x.isRunning());
        }

        if (shouldError)
        {
            QVERIFY(spyFinished.isEmpty());
            QCOMPARE(spyError.size(), 1);
            QVERIFY(x.bioStrings().isEmpty());
        }
        else
        {
            if (spyError.size() > 0)
                qDebug() << spyError.at(0);

            QVERIFY(spyError.isEmpty());
            QCOMPARE(spyFinished.size(), 1);
            QVariantList spyArguments = spyFinished.takeFirst();
            BioStringVector result = qvariant_cast<BioStringVector>(spyArguments.at(0));
            QCOMPARE(result, bioStringVector);
            QCOMPARE(x.bioStrings(), bioStringVector);
        }
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_MAIN(TestBlastSequenceFetcher)
#include "TestBlastSequenceFetcher.moc"
