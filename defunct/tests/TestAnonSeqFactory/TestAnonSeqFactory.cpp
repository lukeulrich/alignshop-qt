/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "AminoString.h"
#include "AnonSeqFactory.h"

/**
  * Because AnonSeqFactory is an abstract base class with pure virtual functions, we must define a Mock class
  * that permits us to test the base class functionality. Specifically, this mock class, enables us to instantiate
  * an object which in turn provides a vehicle for testing the base class methods.
  *
  * The only necessity is to provide an adequate constructor and hide the pure virtual functions inside
  * the private section.
  */
class MockAnonSeqFactory : public AnonSeqFactory
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    MockAnonSeqFactory(QCryptographicHash::Algorithm digestAlgorithm) : AnonSeqFactory(digestAlgorithm)
    {
    }

private:
    // Pure virtual stub functions
    AnonSeq add(const BioString & /* bioString */)
    {
        return AnonSeq();
    }

    AnonSeq fetch(const BioString & /* bioString */) const
    {
        return AnonSeq();
    }

    AnonSeq fetch(int /* id */) const
    {
        return AnonSeq();
    }

    int remove(const BioString & /* bioString */)
    {
        return false;
    }

    int remove(int /* id */)
    {
        return false;
    }

    int size() const
    {
        return 0;
    }
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TestAnonSeqFactory : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setBioStringPrototype();       // Also tests the bioStringPrototype function
};

void TestAnonSeqFactory::constructor()
{
    MockAnonSeqFactory f(QCryptographicHash::Sha1);
}

void TestAnonSeqFactory::setBioStringPrototype()
{
    MockAnonSeqFactory f(QCryptographicHash::Sha1);

    // Initially, the bioString prototype should be invalid
    QVERIFY(f.bioStringPrototype() == 0);

    AminoString *as = new AminoString();
    f.setBioStringPrototype(as);
    QVERIFY(f.bioStringPrototype() == as);

    f.setBioStringPrototype(0);
    QVERIFY(f.bioStringPrototype() == 0);
}



QTEST_MAIN(TestAnonSeqFactory)
#include "TestAnonSeqFactory.moc"


