/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtCore/QDebug>
#include <QtCore/QSharedPointer>
#include <QtCore/QWeakPointer>

class SharedItem
{
public:
    static QSharedPointer<SharedItem> create(const QString &name, int num)
    {
        QSharedPointer<SharedItem> newItem(new SharedItem(name, num));
        newItem->weakThis_ = newItem;
        return newItem;
    }

    void appendChild(QSharedPointer<SharedItem> child)
    {
        children_.append(child);
        child->parent_ = weakThis_;
    }

    int childCount()
    {
        return children_.count();
    }

    QSharedPointer<SharedItem> childAt(int row)
    {
        return children_.at(row);
    }

    int num()
    {
        return num_;
    }


private:
    SharedItem(const QString &name, int num) : name_(name), num_(num)
    {
    }

    QList<QSharedPointer<SharedItem> > children_;

    QWeakPointer<SharedItem> parent_;
    QWeakPointer<SharedItem> weakThis_;

    QString name_;
    int num_;
};


class RawItem
{
public:
    RawItem(const QString &name, int num) : name_(name), num_(num)
    {
    }

    ~RawItem()
    {
        qDeleteAll(children_);
    }

    void appendChild(RawItem *child)
    {
        children_.append(child);
        child->parent_ = this;
    }

    int childCount()
    {
        return children_.count();
    }

    RawItem *childAt(int row)
    {
        return children_.at(row);
    }

    int num()
    {
        return num_;
    }


private:

    QList<RawItem *> children_;
    RawItem *parent_;
    QString name_;
    int num_;
};

void buildSharedTree(const QSharedPointer<SharedItem> &source, int depth, int children)
{
    if (depth == 0)
        return;

    for (int c=0; c< children; ++c)
    {
        source->appendChild(SharedItem::create("Blah", c));
        buildSharedTree(source, depth-1, children);
    }
}


void buildRawTree(RawItem *source, int depth, int children)
{
    if (depth == 0)
        return;

    for (int c=0; c< children; ++c)
    {
        source->appendChild(new RawItem("Blah", c));
        buildRawTree(source, depth-1, children);
    }
}

int recurseSharedSum(const QSharedPointer<SharedItem> &node)
{
    int n = node->childCount();
    if (n == 0)
        return 0;

    int sum = 0;
    for (int i=0; i< n; ++i)
    {
        sum += node->childAt(i)->num();
        sum += recurseSharedSum(node->childAt(i));
    }

    return sum;
}

int recurseRawSum(RawItem *node)
{
    int n = node->childCount();
    if (n == 0)
        return 0;

    int sum = 0;
    for (int i=0; i< n; ++i)
    {
        sum += node->childAt(i)->num();
        sum += recurseRawSum(node->childAt(i));
    }

    return sum;
}

class BenchSharedVsRawTreePointer : public QObject
{
    Q_OBJECT

private slots:
    void shared_build()
    {
        QBENCHMARK
        {
            QSharedPointer<SharedItem> root(SharedItem::create("root", 0));
            buildSharedTree(root, 5, 20);
        }
    }

    void shared_traverse()
    {
        QSharedPointer<SharedItem> root(SharedItem::create("root", 0));
        buildSharedTree(root, 5, 20);

        QBENCHMARK
        {
            int sum = recurseSharedSum(root);
            qDebug() << "Shared sum" << sum;
        }
    }

    void raw_build()
    {
        QBENCHMARK
        {
            RawItem *root = new RawItem("root", 0);
            buildRawTree(root, 5, 20);
            delete root;
            root = 0;
        }
    }

    void raw_sum()
    {
        RawItem *root = new RawItem("root", 0);
        buildRawTree(root, 5, 20);

        QBENCHMARK
        {
            int sum = recurseRawSum(root);
            qDebug() << "Raw sum" << sum;
        }

        delete root;
        root = 0;
    }
};


QTEST_MAIN(BenchSharedVsRawTreePointer)
#include "BenchSharedVsRawTreePointer.moc"
