/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore>
#include "ObservableList.h"
#include "ObservableListNotifier.h"

class TestItem
{
public:
    static int DeleteCount;
    int id;

    TestItem(int id = 1)
    {
        this->id = id;
    }

    ~TestItem()
    {
        ++DeleteCount;
    }
};

int TestItem::DeleteCount = 0;

class VerifyEvent : public QObject
{
    Q_OBJECT

private:
    static const int INVALID = -333;

    bool firedInOrder;
    bool secondFired;

public:
    int firstValue;
    int secondValue;

    VerifyEvent()
    {
        secondFired = false;
        firedInOrder = false;
        firstValue = INVALID;
        secondValue = INVALID;
    }

public slots:
    virtual void first()
    {
        firedInOrder = !secondFired;
        secondFired = false;
    }

    void first(int i)
    {
        firstValue = i;
        first();
    }

    virtual void second()
    {
        secondFired = true;
    }

    void second(int i)
    {
        secondValue = i;
        second();
    }

    bool verifyAndReset()
    {
        bool isValid = firedInOrder && secondFired;
        firstValue = INVALID;
        secondValue = INVALID;
        firedInOrder = false;
        secondFired = false;

        return isValid;
    }
};

class VerifyCurrentChangedEvent : public VerifyEvent
{
    Q_OBJECT

private:
    const ObservableList<TestItem> *list_;

public:
    VerifyCurrentChangedEvent(const ObservableList<TestItem> &list)
    {
        list_ = &list;
    }

public slots:
    virtual void first()
    {
        VerifyEvent::first();
        firstValue = list_->currentIndex();
    }


    virtual void second()
    {
        VerifyEvent::second();
        secondValue = list_->currentIndex();
    }
};

class TestObservableList : public QObject
{
    Q_OBJECT

private slots:
    void addedItemsCanBeRetrieved()
    {
        ObservableList<TestItem> l;
        l.add(new TestItem(1));
        l.add(new TestItem(93));

        QVERIFY(l[0]->id == 1);
        QVERIFY(l[1]->id == 93);
    }

    void indexerChecksBounds()
    {
        ObservableList<TestItem> l;
        l.add(new TestItem());
        QVERIFY(!l[1]);
        QVERIFY(!l[-1]);
    }

    void indexOf()
    {
        ObservableList<TestItem> l;
        l.add(new TestItem());
        l.add(new TestItem());
        TestItem notInList;

        QVERIFY(l.indexOf(l[0]) == 0);
        QVERIFY(l.indexOf(l[1]) == 1);
        QVERIFY(l.indexOf(0) == -1);
        QVERIFY(l.indexOf(&notInList) == -1);
        QVERIFY(l.indexOf(l[0], 1) == -1);
        QVERIFY(l.indexOf(l[0], -1) == -1);
        QVERIFY(l.indexOf(l[1], 1) == 1);
        QVERIFY(l.indexOf(l[1], 2) == -1);
    }

    void itemsAreDeletedOnDestruction()
    {
        TestItem::DeleteCount = 0;
        ObservableList<TestItem>* l = new ObservableList<TestItem>();
        l->add(new TestItem());
        delete l;

        QVERIFY(TestItem::DeleteCount == 1);
    }

    void itemsAreDeletedOnRemove()
    {
        TestItem::DeleteCount = 0;
        ObservableList<TestItem> l;
        l.add(new TestItem());
        l.removeAt(0);

        QVERIFY(TestItem::DeleteCount == 1);
    }

    void lengthUpdatesAppropriately()
    {
        ObservableList<TestItem> l;
        QVERIFY(!l.length());
        l.add(new TestItem(1));
        QVERIFY(l.length() == 1);
        l.add(new TestItem(93));
        QVERIFY(l.length() == 2);
    }

    void removeChecksBounds()
    {
        ObservableList<TestItem> l;
        l.add(new TestItem());
        QVERIFY(!l.removeAt(1));
        QVERIFY(l.removeAt(0));
        QVERIFY(!l.removeAt(0));
    }

    void eventsFireWhenItemAdded()
    {
        ObservableList<TestItem> l;
        VerifyEvent verifier;

        QObject::connect(l.notifier(), SIGNAL(itemAdding(int)), &verifier, SLOT(first(int)));
        QObject::connect(l.notifier(), SIGNAL(itemAdded(int)), &verifier, SLOT(second(int)));
        l.add(new TestItem());

        QVERIFY(verifier.firstValue == 0);
        QVERIFY(verifier.secondValue == 0);
        QVERIFY(verifier.verifyAndReset());

        l.add(new TestItem());

        QVERIFY(verifier.firstValue == 1);
        QVERIFY(verifier.secondValue == 1);
        QVERIFY(verifier.verifyAndReset());
    }

    void eventsFireWhenItemRemoved()
    {
        ObservableList<TestItem> l;
        VerifyEvent verifier;

        QObject::connect(l.notifier(), SIGNAL(itemRemoving(int)), &verifier, SLOT(first(int)));
        QObject::connect(l.notifier(), SIGNAL(itemRemoved(int)), &verifier, SLOT(second(int)));
        l.add(new TestItem());
        l.add(new TestItem());

        l.removeAt(1);
        QVERIFY(verifier.firstValue == 1);
        QVERIFY(verifier.secondValue == 1);
        QVERIFY(verifier.verifyAndReset());

        l.removeAt(0);
        QVERIFY(verifier.firstValue == 0);
        QVERIFY(verifier.secondValue == 0);
        QVERIFY(verifier.verifyAndReset());
    }

    void currentIsNullByDefault()
    {
        ObservableList<TestItem> l;
        QVERIFY(!l.current());
    }

    void currentChangesWhenSet()
    {
        ObservableList<TestItem> l;
        l.add(new TestItem());
        QVERIFY(!l.setCurrentIndex(10));
        QVERIFY(!l.current());
        QVERIFY(!l.setCurrentIndex(-10));
        QVERIFY(!l.current());
        QVERIFY(l.setCurrentIndex(0));
        QVERIFY(l.current() == l[0]);
        QVERIFY(l.setCurrentIndex(Row::ObservableList::NO_CURRENT));
        QVERIFY(!l.current());
    }

    void eventsFireWhenCurrentChanges()
    {
        ObservableList<TestItem> l;
        VerifyCurrentChangedEvent verifier(l);

        QObject::connect(l.notifier(), SIGNAL(currentChanging()), &verifier, SLOT(first()));
        QObject::connect(l.notifier(), SIGNAL(currentChanged()), &verifier, SLOT(second()));
        l.add(new TestItem());
        l.add(new TestItem());

        QVERIFY(l.currentIndex() == Row::ObservableList::NO_CURRENT);
        QVERIFY(l.setCurrentIndex(1));
        QVERIFY(verifier.verifyAndReset());

        QVERIFY(l.currentIndex() == 1);
        QVERIFY(l.setCurrentIndex(0));
        QVERIFY(verifier.firstValue == 1);
        QVERIFY(l.currentIndex() == 0);

        l.removeAt(0);
        QVERIFY(verifier.firstValue == 0);
        QVERIFY(l.currentIndex() == Row::ObservableList::NO_CURRENT);
    }
};

QTEST_MAIN(TestObservableList)
#include "TestObservableList.moc"
