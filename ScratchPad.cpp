/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

/**
  * 13 July 2010
  *
  * Transform the following compressed tree
  * o Root
  * |---- Group PAS
  * |---- Seq A, Subseq 1
  * |---- Seq A, Subseq 2
  *
  * To:
  *
  * o Root (@)
  * |---- Group PAS (@)
  * |---- Seq A
  *       |---- Subseq 1
  *       |---- Subseq 2
  *
  * RowMaps would be created for those nodes marked with @
  *
  * The RowMap will be stored as the void * of the proxy QModelIndexes. Thus, in the various methods that provide
  * a proxy model index, it will be possible to utilize the rowmap to return the relevant data. A key thing to
  * remember is that the first time the various functions are called (columnCount, etc), it will be with an invalid
  * QModelIndex which makes it possible to build the first row map relative to the source model. Additional
  * RowMaps will be constructed on-demand.
  */
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtCore/QModelIndex>

struct ProxyRow
{
    int sourceRow_;
    bool isSequence_;

    ProxyRow(int sourceRow, bool isSequence = false) : sourceRow_(sourceRow), isSequence_(isSequence)
    {
    }
};

// Each QModelIndex from source that is not a subseq should have a corresponding RowMap
struct RowMap
{
    QVector<int> sourceRows_;               // Source row numbers; I think this is unnecessary since we are not filtering out any rows
    QVector<ProxyRow> proxyRows_;           // List of proxy rows; if the proxyRow is a Seq, then its source row index is the first subseq row encountered for this seq
    QHash<int, QVector<int> > subseqs_;     // Key = proxyRowIndex (e.g. proxyRows[key]); value = array of source rows, one for each subseq
};

QHash<QModelIndex, RowMap> sourceIndexMapping;  // Associates a QModelIndex with a RowMap, which should exist for every non-subseq row

RowMap buildRowMap(const QModelIndex &sourceParent)
{
    // If we have already mapped this node, continue
    if (sourceIndexMapping.contains(sourceParent))
        return;

    // Only map those source nodes that are not subseqs
   if (sourceParent is subseq)
       return;

   RowMap rowMap;
   QHash<int, int> seqId2ProxyRow;
   for (int i=0, z=sourceModel()->rowCount(sourceParent); i<z; ++i)
   {
       rowMap.sourceRows_.append(i);

       if (i is subseq)
       {
           int seqId = sourceModel()->data(i, seq_id);
           int proxyRowIndex = seqId2ProxyRow.value(seqId, -1);
           if (proxyRowIndex == -1)  // seqId was not found, thus this is the first time we have seen this seqId
           {
               // Add the sequence to the rowMap
               rowMap.proxyRows_.append(ProxyRow(i, true));

               // The proxy row index will be the index of the last added item
               proxyRowIndex = rowMap.proxyRows_.count() - 1;

               // Update the lookup hash
               seqId2ProxyRow[seqId] = proxyRowIndex;
           }

           // Add the subseq
           rowMap.subseqs_[proxyRowIndex].append(i);
       }
       else
       {
           // All other rows are added as normal
           rowMap.proxyRows_.append(ProxyRow(i, false));
       }
   }

   // Update the master sourceNodeIndex
   sourceIndexMapping.insert(sourceParent, rowMap);

   return rowMap;
}

RowMap rowMap(const QModelIndex &proxyIndex)
{
    // Fetch the Rowmap from the proxyIndex's internal void *
}

QModelIndex sourceIndex(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid())
        return QModelIndex();


}

QModelIndex index(int row, int column, const QModelIndex &parent)
{
    if (row < 0 || column < 0)
        return QModelIndex();

    // Proxy to source; index will be invalid the first time this is called
    QModelIndex sourceParent = sourceIndex(parent);

    // Find the parent
}

int rowCount(const QModelIndex &proxyParent)
{

}









#include <QHash>

#include "AdocTreeNode.h"
#include "DataRow.h"
#include "DbDataSource.h"

#include <QtCore/QTime>

class DbRowCacheManager
{
public:
    DbTableCacheManager(const DbDataSource &dataSource, const QString &table)
    {
        most_recent_ms = -1;
        timer_.start();
    }

    void empty();
    int size() const;
    void setBaseSize(int baseSize);
    void setDataSource(const DbDataSource &dataSource);
    void setOverflowSize(int overflowSize);

    void remove(int id) const;
    DataRow fetch(int id) const
    {
        // Update usage stats
        int elapsed = timer_.elapsed();

        if (most_recent_ms > elapsed)
        {
            // Timer has wrapped over the 24 hr limit. Reset all entries last access
            // time in ms to 0

        }
        most_recent_ms = elapsed;

        if (cache.contains(id))
        {
            usage_[id] = elapsed;
            return cache.value(id);
        }

        // Do we purge the cache?
        if (cacheSizeExceeded())
            removeCacheOverflow();

        DataRow row = fetchFromDatabase(id);
        cache_.insert(id, row);
        usage_[id] = elapsed;

        return row;
    }

protected:
    DataRow fetchFromDatabase(int id)
    {
        // Reads the row from the database with the given id and returns it
    }


private:
    bool cacheSizeExceeded() const
    {
        return cache_.size() >= baseSize_ + overflowSize_;
    }

    void removeCacheOverflow()
    {

    }


    DbDataSource dataSource_;

    QTime timer_;
    int most_recent_ms;

    // Maps id of primary key of table to a saved record
    QHash<int, DataRow> cache_;

    // Maps id of primary key of table to number of times this record has been accessed
    QHash<int, int> usage_;
    int baseSize_;
    int overflowSize_;

};



DbTableCacheManager aminoSeqCache(blah, constants::kTableAminoSeqs);
DbTableCacheManager aminoSubseqCache(blah, constants::kTableAminoSubseqs);



class AggregateDbTableCacheManager
{

};






#include <QtGui/QTableView>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QPainter>
#include <QtDebug>
#include "models/AdocTreeModel.h"
#include "AdocTreeNode.h"
class MyTableView : public QTableView
{
public:
    MyTableView(QWidget *parent = 0) : QTableView(parent)
    {
    }

    void startDrag2(Qt::DropActions supportedActions)
    {
        QModelIndexList indices = selectedIndexes();
        if (indices.count())
        {
            QMimeData *data = model()->mimeData(indices);
            if (!data)
                return;

            QRect rect;
            QPixmap pixmap = renderToPixmap(indices, &rect);
            drag = new QDrag(this);
            drag->setPixmap(pixmap);
            drag->setMimeData(data);
            drag->setHotSpot(viewport()->mapFromGlobal(QCursor::pos()) - rect.topLeft());
            drag->start(supportedActions);
        }
    }

    void dragMoveEvent2(QDragMoveEvent *event)
    {
        if (dragDropMode() == InternalMove
            && (event->source() != this || !(event->possibleActions() & Qt::MoveAction)))
            return;

        // ignore by default
        event->ignore();

        Qt::DropAction dropAction = (model()->supportedDropActions() & event->proposedAction())
                                            ? event->proposedAction() : Qt::IgnoreAction;

        QModelIndex index = indexAt(event->pos());
        AdocTreeNode *node = index.data(AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
        if (node && node->nodeType_ == AdocTreeNode::GroupType)
        {
            event->setDropAction(dropAction);
            event->accept();
            qDebug() << "Accept";
        }
        else
        {
            qDebug() << "Reject";
        }
//        viewport()->update();
//        event->ignore();
    }

private:
    QPixmap renderToPixmap(const QModelIndexList &indexes, QRect *r) const
    {
        QRect rect = visualRect(indexes.at(0));
        QList<QRect> rects;
        for (int i = 0; i < indexes.count(); ++i) {
            rects.append(visualRect(indexes.at(i)));
            rect |= rects.at(i);
        }
        rect = rect.intersected(viewport()->rect());
        QPixmap pixmap(rect.size());

        QColor c = palette().base().color();
        pixmap.fill(c);
        QPainter painter(&pixmap);
        QStyleOptionViewItem option = viewOptions();
        option.state |= QStyle::State_Selected;
        for (int j = 0; j < indexes.count(); ++j) {
            option.rect = QRect(rects.at(j).topLeft() - rect.topLeft(), rects.at(j).size());
            itemDelegate(indexes.at(j))->paint(&painter, option, indexes.at(j));
        }

//        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//        painter.fillRect(rect, QColor(0, 0, 0, 128));

        painter.end();
        if (r)
            *r = rect;


        return pixmap;
    }

    QDrag *drag;
};



// ------------------------------------------------------------------------------------
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>


// This class is in a separate thread and simply performs standard CRUD
class DataSource : public QObject
{
    Q_OBJECT

public:
    DataSource(QObject *parent = 0) : QObject(parent)
    {

    }

slots:
    void open(const QString &file)
    {
        if (database_.isOpen())
        {
            emit sourceOpenError(file, "Database is already open");
            return;
        }

        bool success = false;

        // Attempt to open the database...

        // Emit the proper signals
        if (success)
            emit sourceOpened(file);
        else
            emit sourceOpenError(file, "Error opening database");
    }

    void close()
    {
        if (!database_.isOpen)
        {
            return;
        }
    }

    void select(int queryId, const QString &selectSql, const QVariantList input = QVariantList())
    {
        // Run selectSql against the database with placeholders if there are any
        QSqlQuery query(database_);
        if (!query.prepare(selectSql))
        {
            emit selectError(queryId, DatabaseError("Unable to prepare select query", query.lastError(), selectSql));
            return;
        }

        if (!input.isEmpty())
        {
            foreach (const QVariant &variable, input)
                query.addBindValue(variable);
        }

        query.setForwardOnly(true);
        if (!query.exec())
        {
            emit selectError(queryId, DatabaseError("Unable to execute select query", query.lastError(), selectSql));
            return;
        }

        QList<DataRow> results;
        while (query.next())
            results << query.record();

        emit selectFinished(queryId, results);
    }

signals:
    void dataSourceOpened(const QString &file);
    void dataSourceOpenError(const QString &file, const DatabaseError &error);

    void selectFinished(int id, const QList<DataRow> &dataRows);
    void selectError(int id, const DatabaseError &error);

    void databaseError(const QSqlError error);

private:
    QSqlDatabase database_;
};




// This class would be in the main thread and serve as the single point of access to the datasource
#include "exceptions/DatabaseError.h"
#include <QtCore/QThread>

/// Thread creation and deletion directly corresponds to the opening and closing of a database connection
class DataBroker : public QObject
{
    Q_OBJECT

public:
    DataBroker(QObject *parent = 0) : QObject(parent)
    {
        dataSource_ = 0;
        queryIdVar_ = 0;
    }

    void
    {

    }

slots:
    void openDatabaseFile(const QString &file)
    {
        // Already have an open file
        if (dataSource_)
        {
            emit dataSourceOpenError(DatabaseError("Already have open file"));
            return;
        }

        // Create the datasource
        dataSource_ = new DataSource;
        dataSource_->moveToThread(&dbThread_);

        // Connect its signals
        QObject::connect(dataSource_, SIGNAL(dataSourceOpened(QString)), this, SIGNAL(dataSourceOpened(QString)));
        QObject::connect(dataSource_, SIGNAL(dataSourceOpenError(const QString &, const DatabaseError &)), this, SIGNAL(dataSourceOpenError(const QString &, const DatabaseError &)));

        QObject::connect(dataSource_, SIGNAL(selectError(int,DatabaseError)), this, SLOT(selectError(int,DatabaseError)));

        // Begin the thread
        dbThread_.start();

        // Open the database file
        QMetaObject::invokeMethod(dataSource_, "open", Q_ARG(QString, file));
    }

    virtual void getDataTree()
    {
        if (!dataSource_)
        {
            emit dataTreeError(DatabaseError("Database file not open"));
            return;
        }

        // Compose the SQL to fetch the data tree
        ++queryIdVar_;
        QMetaObject::invokeMethod(dataSource_, "select", Q_ARG(int, queryIdVar_), Q_ARG(QString, QString("SELECT * FROM %1 ORDER BY lft").arg(constants::kTableDataTree)));
    }

    virtual void getAminoSeqRows(QList<int> ids)
    {
        if (!dataSource_)
        {
            emit aminoSeqRowsError(DatabaseError("Database file not open"));
            return;
        }

        ++queryIdVar_;
        QMetaObject::invokeMethod(dataSource_, "select", Q_ARG(int, queryIdVar_), Q_ARG(QString, QString("SELECT * FROM %1 WHERE id = ?").arg(constants::kTableAminoSeqs)), Q_ARG(QVariantList, ids));
    }

    virtual void getDnaSeqRows(QList<int> ids) = 0;
    virtual void getRnaSeqRows(QList<int> ids) = 0;

private slots:
    virtual void select(const QString &query, QObject *requester, const QString &slotCallback) = 0;



signals:
    void dataSourceOpened(const QString &file);
    void dataSourceOpenError(const QString &file, const DatabaseError &error);

    void dataTreeReady(AdocTreeNode *root);
    void dataTreeError(const DatabaseError &error);

    void aminoSeqRowsReady(const QList<DataRow> &aminoSeqRows);
    void aminoSeqRowsError(const DatabaseError &error);

    void dnaSeqRowsReady(const QList<DataRow> &dnaSeqRows);
    void dnaSeqRowsError(const DatabaseError &error);

    void rnaSeqRowsReady(const QList<DataRow> &rnaSeqRows);
    void rnaSeqRowsError(const DatabaseError &error);

private:
    QThread dbThread_;
    DataSource *dataSource_;        // Owned and belongs to dbThread_
    int queryIdVar_;
};



class AminoSeqModel : public QAbstractItemModel
{
public:
    int rowCount(const QModelIndex &index)
    {
        return rows_.count();
    }

    // ...

slots:
    void queryIds(const QList<int> amino_seq_ids)
    {
        QList<int> newIds;
        foreach (const int &amino_seq_id, amino_seq_ids)
            if (!idLookup_.contains(amino_seq_id))
                newIds << amino_seq_id;

        if (newIds.isEmpty())
            return;

        broker_->
    }


signals:

private:
    QList<DataRow> rows_;
    QHash<int, int> idLookup_;      // {id} -> row index

    DataBroker *broker_;
};




class QMainWindow;

int main()
{
    QMainWindow mw;

    DataBroker broker;

    // Setup signals for responding the events
    QObject::connect(&broker, SIGNAL(dataSourceOpened(QString)), &mw, SLOT(dataSourceOpened(QString)));
    QObject::connect(&broker, SIGNAL(dataSourceOpenError(QString)), &mw, SLOT(dataSourceOpenError(QString)));

    broker.openDatabaseFile("blah.db");
}










class ThreadedAdocDbDataSource : public AdocDataSource
{
    Q_OBJECT

public:
    ThreadedAdocDbDataSource() : AdocDataSource()
    {
        dataSource_.moveToThread(&thread_);

        // Connect the signals
        QObject::connect(dataSource_, )
        QObject::connect(dataSource_, SIGNAL(dataTreeReady(AdocTreeNode*)), this, SIGNAL(dataTreeReady(AdocTreeNode*)));

        thread_.start();
    }

slots:

    virtual void openDataFile(const QString &file)
    {
        // Use queued connection to perform open on threaded instance
        QMetaObject::invokeMethod(dataSource_, "openDataFile", Q_ARG(QString, file));
    }

    virtual void getDataTree()
    {
        QMetaObject::invokeMethod(dataSource_, "getDataTree");
    }

    virtual void select(const QString &sql, QObject *object, const char *method, const char *errorMethod)
    {
        QMetaObject::invokeMethod(dataSource_, "select", Q_ARG(QObject *, object), Q_ARG(const char *, method), Q_ARG(const char *, errorMethod));
    }

private:
    QThread thread_;
    AdocDbDataSource dataSource_;
};









SliceProxyModel::createSlice(...)
{
    for (int i=start; i< end; ++i)
    {
        if (filterAcceptsNode(node->childAt(i)))
        {

        }
    }
}


class SubseqSeqProxyModel : public SliceProxyModel
{
    Q_OBJECT

public:
    void setSourceTables(RelatedTableModel *subseqTable, TableModel *seqTable)
    {
        if (subseqTable_)
        {
            QObject::disconnect(subseqTable_, SIGNAL(loadDone(int)), this, SLOT(subseqsLoadDone(int)));
            QObject::disconnect(subseqTable_, SIGNAL(loadError(int)), this, SLOT(subseqsLoadError(QString,int)));
        }

        subseqTable_ = subseqTable;
        seqTable_ = seqTable;

        if (subseqTable_)
        {
            QObject::connect(subseqTable_, SIGNAL(loadDone(int)), this, SLOT(subseqsLoadDone(int)));
            QObject::connect(subseqTable_, SIGNAL(loadError(int)), this, SLOT(subseqsLoadError(QString,int)));
        }

        clear();
    }

    int columnCount() const
    {
        return 9;
    }



    void taggedSliceCreated(int tag)
    {
        ASSERT(taggedSlices_.contains(tag));

        Slice groups;

        QList<int> subseqIds;
        Slice &slice = taggedSlices_[tag];
        for (int i=slice.count()-1; i>= 0; --i)
        {
            if (slice.sourceNodes_.at(i).nodeType_ == AdocTreeNode::GroupType)
            {
                groups.sourceNodes_ << slice.sourceNodes_.takeAt(i);
                groups.sourceRowIndices_ << slice.sourceRowIndices_.takeAt(i);
            }
            else if (slice.sourceNodes_.at(i).nodeType == AdocTreeNode::SubseqAminoType)
            {
                subseqIds << slice.sourceNodes_.at(i).fkId_;
            }
        }

        if (groups.count())
            appendSlice(groups);

        if (subseqIds.isEmpty())
        {
            taggedSlices_.remove(tag);
            return;
        }

        subseqTable_.load(subseqIds, tag);
    }


private slots:
    void subseqsLoadDone(int tag)
    {
        if (!taggedSlices_.contains(tag))
            return;

        appendSlice(taggedSlices_.take(tag));
    }

    void subseqsLoadError(const QString &error, int tag)
    {
        if (!taggedSlices_.contains(tag))
            return;

        // Remove from list and report error somehow...
        taggedSlices_.take(tag);
    }

protected:
    bool filterAcceptsNode(AdocTreeNode *node)
    {
        if (!subseqTable_ || !seqTable_)
            return false;

        if (node->nodeType_ == AdocTreeNode::GroupType)
            return true;

        return false;
    }

private:
    RelatedTableModel *subseqTable_;
    TableModel *seqTable_;
};
















class Adoc : public QObject
{
    Q_OBJECT

public:
    // Be sure to initialize pointers to zero
    explicit Adoc(QObject *parent = 0);
    ~Adoc()
    {
        close();
    }

    AdocDataSource *adocDataSource() const;
    AdocTreeModel *adocTreeModel() const;
    TableModel *aminoSeqModel() const;
    TableModel *aminoSubseqModel() const;

public slots:
    void open(const QString &file);
    {
        // Lazily load adocDataSource
        if (!adocDataSource_)
        {
            adocDataSource_ = new AdocDataSource(this);

            QObject::connect(adocDataSource_, SIGNAL(opened()), adocDataSource_, SLOT(getDataTree()));
            QObject::connect(adocDataSource_, SIGNAL(dataTreeReady(AdocTreeNode *)), this, SLOT(dataTreeReady(AdocTreeNode *)));
            QObject::connect(adocDataSource_, SIGNAL(dataTreeReady(AdocTreeNode *)), this, SIGNAL(finalSetup()));
            QObject::connect(adocDataSource_, SIGNAL(closed()), this, SIGNAL(closed()));

            // Setup error handlers
            QObject::connect(adocDataSource_, SIGNAL(openError(QString,QString)), this, SIGNAL(openError(QString,QString)));
            QObject::connect(adocDataSource_, SIGNAL(dataTreeError(DatabaseError)), this, SLOT(onDataTreeError(DatabaseError)));
        }

        // No need to check if we are already open, because the IAdocDataSource should return an openError for us
        // if its already open

        adocDataSource_->open(file);
    }
    void close()
    {
        if (aminoSubseqModel_)
        {
            delete aminoSubseqModel_;
            aminoSubseqModel_ = 0;
        }
        if (aminoSeqModel_)
        {
            delete aminoSeqModel_;
            aminoSeqModel_ = 0;
        }
        if (adocTreeModel_)
        {
            delete adocTreeModel_;
            adocTreeModel_ = 0;
        }

        if (adocDataSource_)
            // No need to delete the adoc data source, because it is part of the QObject class hierarchy
            adocDataSource_->close();
    }

private slots:
    void dataTreeReady(AdocTreeNode *root)
    {
        Q_ASSERT(root);
        Q_ASSERT(adocTreeModel == 0);
        Q_ASSERT(adocDataSource_);

        adocTreeModel_ = new AdocTreeModel(this));  // The constructor of AdocTreeModel creates a bare root node
        adocTreeModel_->setRoot(root);
    }

    void finalSetup()
    {
        Q_ASSERT(adocDataSource_);
        Q_ASSERT(aminoSeqModel_ == 0);
        Q_ASSERT(aminoSubseqModel_ == 0);

        aminoSeqModel = new TableModel(adocDataSource_, "amino_seqs", this);
        aminoSubseqModel = new TableModel(adocDataSource_, "amino_subseqs", this);

        emit opened(adocDataSource_->sourceFile());
    }

    void onDataTreeError(const DatabaseError &error)
    {
        Q_ASSERT(adocDataSource_);
        emit openError(adocDataSource_->sourceFile(), error.message_);
        adocDataSource_->close();
    }

signals:
    void opened(const QString &file);
    void openError(const QString &file, const QString &error);
    void closed();

private:
    AdocDataSource *adocDataSource_;

    AdocTreeModel *adocTreeModel_;

    TableModel *aminoSeqModel_;
    TableModel *aminoSubseqModel_;
};




MainWindow mw;

SliceModel aminoSliceModel;
Adoc adoc;
adoc.open("blah.db");

--> SLOT onOpen()
{
    aminoSliceModel->setup(adoc.adocTreeModel(), adoc.aminoSeqModel(), adoc.aminoSubseqModel());

    // Hook up the UI
    ui_->containerFilterModel->setModel(adoc.adocTreeModel());
    // QObject::connect(ui_->containerFilterModel, SIGNAL(clicked(QModelIndex)), this, SLOT(...));

}

--> SLOT onContainerClicked(QModelIndex &index)
{
    aminoSliceModel->setSlice(index);
}


aminoSliceModel::setSlice(const QModelIndex &index)
{
    AdocTreeNode *parent = adocTreeModel_->nodeFromIndex(index);
    if (parent == parent_)
        return;

    createSlice(parent);        // Calls pure virtual function defined in subclasses

    beginResetModel();
    subseq_ids.clear();
    endResetModel();
}



struct CharCount
{
    char c_;
    int count_;

    CharCount(char c, int count = 0) : c_(c), count_(count) { }
};

typedef QList<QList<CharCount> > ColumnCharDistribution;

typedef QList<QHash<char, int> > ColumnCharDistribution;

class MsaColumnCharDistributionStrategy
{
public:
    ColumnCharDistribution calculateMsaDistribution(const Msa &msa, int startColumn = -1, int stopColumn = -1) const;
};

class LiveMsaColumnCharDistribution
{
public:
    LiveMsaColumnCharDistribution(MsaColumnCharDistributionStrategy strategy);

    void setMsa(Msa *msa)
    {
        msa_ = msa;
        dist = strategy_.calculateMsaDistribution(*msa_);

        connect(msa_, SIGNAL(msaRegionSlid(int,int)), this, SLOT(recomputeDistribution(int,int)));
    }
    ColumnCharDistribution distribution() const;

signals:
    void distributionChanged(int startColumn, int stopColumn);

private slots:
    void recomputeDistribution(int startColumn = -1, int stopColumn = -1)
    {
        strategy_.calculateMsaDistribution(msa_, startColumn, stopColumn);

        // Merge update

        emit distributionChanged(startColumn, stopColumn);
    }

private:
    Msa *msa_;
    ColumnCharDistribution dist;
    MsaColumnCharDistributionStrategy strategy_;
};


class PositionalMsaColorScheme
{
public:
    (QColor, QColor) = color(msa, i, j)
    {
        return default colors;
    }
};

class ClustalMsaColorScheme
{
public:
    (QColor, QColor) = color(msa, i, j)
    {
        if (msa_ == msa)
        {
            // Determine color based on symbolString_.at(i) and msa.characterAt(i, j)
        }
        else
        {
            // Return default color
        }
    }

    void setMsa(Msa *msa)
    {
        symbolString_  = algo_->compute(msa);
    }

private:
    Algo *algo_;
    Msa *msa_;
    QString symbolString_;
};



class MsaView
{
public:
    void paint()
    {

    }
    void setMsa(Msa *msa)
    {

    }

private:

};


class LiveResult
{
    LiveResult(Algorithm);
    void setMsa(Msa *msa);
};

class ColorProvider
{
    void setR
};





namespace Algorithms
{
    QList<QHash<char, int> > calculateMsaCharCountDistribution(const Msa &msa, const QRect &region = QRect());
}



class MsaCharCountDistribution
{
public:
    MsaCharCountDistribution(Msa *msa, const QRect &region);

    bool subtract(const MsaCharCountDistribution &other, int offset = 1);
    bool add(const MsaCharCountDistribution &other, int offset = 1);
    void insertBlanks(int position, int count);
    void remove(int position, int count);

    QList<QHash<char, int> > charCounts() const;
    const Msa *msa() const;
    QRect region() const;

private:
    Msa *msa_;
    QRect region_;
    QList<QHash<char, int> > charCounts_;
};




class LiveMsaCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    LiveMsaCharCountDistribution(Msa *msa, QObject *parent = 0) : QObject(parent), charCountDistribution_(msa, region)
    {
        if (!msa)
            return;

        // Hook up signals
        connect(msa, SIGNAL(msaRegionSlid(QRect,int), SLOT(onRegionSlid(QRect,int))));
    }

    MsaCharCountDistribution charCounts() const;

signals:
    void changed(int from, int to);

private slots:
    void onRegionSlid(QRect rect, int delta)
    {
        MsaCharCountDistribution block(charCountDistribution_.msa(), rect + delta);
        charCountDistribution_.subtract(block, rect.left());
        charCountDistribution_.add(block, rect.left() + delta);

        emit changed(qMin(rect.left(), rect.left() + delta), qMax(rect.right(), rect.right() + delta));
    }

private:
    CharCountDistribution charCountDistribution_;
};




class CharacterPixmapRenderer
{
public:
    CharacterPixmapRenderer(QFont font = const QFont());

    QPixmap renderGlyph(char character, QColor foregroundColor) const;

    void setFont(QFont font);
    QFont font() const;

private:
    QFont font_;
};


class CharacterPixmapProvider
{
public:
    CharacterPixmapProvider(const CharacterPixmapRenderer &characterPixmapRenderer = CharacterPixmapRenderer());

    Pixmap glyph(const character, QColor foregroundColor);
    Pixmap cachedGlyph(const character, QColor foregroundColor);
    void clearCache();

private:
    CharacterPixmapRenderer characterPixmapRenderer_;
    QHash<QColor, QHash<char, Pixmap> > cachedGlyphs_;
};


class MsaView
{
public:

    void setCharacterPixmapProvider(const CharacterPixmapProvider &characterPixmapProvider);

private:
    CharacterPixmapProvider characterPixmapProvider_;
}



class BioSymbol
{
public:
    BioSymbol(char symbol, const QString &characters, qreal threshold);

    void addCharacters(const QString &characters);
    QString characters() const;
    QSet<char> characterSet() const;
    bool hasCharacter(char character) const;
    void removeCharacters(const QString &characters);
    void setCharacters(const QString &characters);
    void setSymbol(char symbol);
    void setThreshold(qreal threshold);
    char symbol() const;
    qreal threshold() const;

private:
    QSet<char> characters_;
    qreal threshold_;
    char symbol_;
};

class BioSymbolGroup
{
public:
    void addBioSymbol(const BioSymbol &bioSymbol);
    QHash<char, BioSymbol> bioSymbols() const;
    void clear();
    bool isCharAssociatedWithSymbol(char character, char symbol) const;
    void removeBioSymbol(char symbol);

    BioSymbolGroup &operator<<(const BioSymbol &bioSymbol);

private:
    QHash<char, BioSymbol> bioSymbols_;
};

// -----------------------------------------------------
// Consenus without thresholds applied
BioSymbolGroup consensus;
consensus << BioSymbol('G', "G")
          << BioSymbol('A', "A")
          << BioSymbol('I', "I")
          << BioSymbol('V', "V")
          << BioSymbol('L', "L")
          << BioSymbol('M', "M")
          << BioSymbol('F', "F")
          << BioSymbol('Y', "Y")
          << BioSymbol('W', "W")
          << BioSymbol('H', "H")
          << BioSymbol('C', "C")
          << BioSymbol('P', "P")
          << BioSymbol('K', "K")
          << BioSymbol('R', "R")
          << BioSymbol('D', "D")
          << BioSymbol('E', "E")
          << BioSymbol('Q', "Q")
          << BioSymbol('N', "N")
          << BioSymbol('S', "S")
          << BioSymbol('T', "T");
BioSymbol aromatic('a', "FYWH");
consensus << aromatic;
BioSymbol aliphatic('l', "IVL");
consensus << aliphatic;
BioSymbol hydrophobic('h', aromatic.characters() + aliphatic.characters() + "AGMCKRT");
consensus << hydrophobic;
BioSymbol positive('+', "HKR");
consensus << positive;
BioSymbol negative('-', "DE");
consensus << negative;
BioSymbol charged('c', positive.characters() + negative.characters);
consensus << charged;
BioSymbol polar('p', charged.characters() + "QNSTC");
consensus << polar;
BioSymbol alcohol('o', "ST");
consensus << alcohol;
BioSymbol tiny('u', "GAS");
consensus << tiny;
BioSymbol small('s', tiny.characters() + "VTDNPC");
consensus << small;
BioSymbol turnlike('t', tiny.characters() + polar.characters());
consensus << turnlike;
BioSymbol any('.', "GAVILMNFYWHCPKRDEQNST");
consensus << any;

// ---------------------------------------------------
BioSymbolGroup clustalSymbols;
clustalSymbols << BioSymbol('%', "WLVIMAFCYHP", .6)
               << BioSymbol('#', "WLVIMAFCYHP", .8)
               << BioSymbol('-', "DE", .5)
               << BioSymbol('+', "KR", .6)
               << BioSymbol('g', "G", .5)
               << BioSymbol('n', "N", .5)
               << BioSymbol('q', "QE", .5)
               << BioSymbol('p', "P", .5)
               << BioSymbol('t', "ST", .5)
               << BioSymbol('A', "A", .85)
               << BioSymbol('C', "C", .85)
               << BioSymbol('D', "D", .85)
               << BioSymbol('E', "E", .85)
               << BioSymbol('F', "F", .85)
               << BioSymbol('G', "G", .85)
               << BioSymbol('H', "H", .85)
               << BioSymbol('I', "I", .85)
               << BioSymbol('K', "K", .85)
               << BioSymbol('L', "L", .85)
               << BioSymbol('M', "M", .85)
               << BioSymbol('N', "N", .85)
               << BioSymbol('P', "P", .85)
               << BioSymbol('Q', "Q", .85)
               << BioSymbol('R', "R", .85)
               << BioSymbol('S', "S", .85)
               << BioSymbol('T', "T", .85)
               << BioSymbol('V', "V", .85)
               << BioSymbol('W', "W", .85)
               << BioSymbol('Y', "Y", .85);

BioSymbolGroup clustalAaCharacterSymbols;
// G = always orange
// P = always yellow
clustalAaCharacterSymbols << BioSymbol('T', "tST%#")
                          << BioSymbol('S', "tST#")
                          << BioSymbol('N', "nND")
                          << BioSymbol('Q', "qQE+KR")
                          << BioSymbol('W', "%#ACFHILMVWYPp")
                          << BioSymbol('L', "%#ACFHILMVWYPp")
                          << BioSymbol('V', "%#ACFHILMVWYPp")
                          << BioSymbol('I', "%#ACFHILMVWYPp")
                          << BioSymbol('M', "%#ACFHILMVWYPp")
                          << BioSymbol('A', "%#ACFHILMVWYPpTSsG")  //ASD
                          << BioSymbol('F', "%#ACFHILMVWYPp")
                          << BioSymbol('C', "%#ACFHILMVWYPp")
//                          << BioSymbol('C', "C")
                          << BioSymbol('H', "%#ACFHILMVWYPp")
                          << BioSymbol('Y', "%#ACFHILMVWYPp")
                          << BioSymbol('E', "-DEqQ")
                          << BioSymbol('D', "-DEnN")
                          << BioSymbol('K', "+KRQ")
                          << BioSymbol('R', "+KRQ");

SymbolColorScheme clustalColorScheme;
clustalColorScheme.setCharSymbolsColorStyle('C', "%#ACFHILMVWYPp", blue);
clustalColorScheme.setCharSymbolsColorStyle('C', "C", pink);


QColor red(.9 * 255, .2 * 255, .1 * 255);
QColor blue(.1 * 255, .5 * 255, .9 * 255);
QColor green(.1 * 255, .8 * 255, .1 * 255);
QColor cyan(.1 * 255, .7 * 255, .7 * 255);
QColor pink(.9 * 255, .5 * 255, .5 * 255);
QColor magenta(.8 * 255, .3 * 255, .8 * 255);
QColor yellow(.8 * 255, .8 * 255, 0);
QColor orange(.9 * 255, .6 * 255, .3 * 255);



class LiveSymbolString : public QObject
{
    Q_OBJECT

public:
    LiveSymbolString(const LiveCharCountDistribution *liveCharCountDistribution, const BioSymbolGroup &bioSymbolGroup);

    BioSymbolGroup bioSymbolGroup() const;
    const LiveCharCountDistribution *liveCharCountDistribution() const;
    QString symbolString() const;

signals:
    void symbolsInserted(int from, int to);                             //!< Emitted after the symbols between from and to (inclusive, 1-based) have been inserted
    void symbolsRemoved(int from, int to);                              //!< Emitted after the symbols between from and to (inclusive, 1-based) have been removed)
    void dataChanged(int startColumn, int stopColumn);                  //!< Emitted when any of the symbols between startColumn and stopColumn (inclusive, 1-baed) have changed

private slots:
    void sourceDistributionColumnsInserted(int from, int to);
    void sourceDistribtuionCOlumnsRemvoed(int from, int to);
    void sourceDataChanged(int startColumn, int stopColumn);

private:
    QString symbolString_;
    LiveCharCountDistribution *liveCharCountDistribution_;
    BioSymbolGroup bioSymbolGroup_;
};


class CharColorScheme
{
public:
    CharColorScheme(const TextColorStyle &defaultTextColorStyle = TextColorStyle());
    TextColorStyle characterTextColorStyle() const;
    TextColorStyle defaultTextColorStyle() const;
    void setDefaultTextColorStyle(const TextColorStyle &defaultTextColorStyle);
    void setCharacterTextColorStyle(char character, const TextColorStyle &textColorStyle);
    virtual TextColorStyle characterColor(char character) const
    {
        if (characterColorStyles_.contains(character))
            return characterColorStyles_.value(character);

        return defaultTextColorStyle_;
    }

protected:
    QHash<char, TextColorStyle> characterColorStyles_;

private:
    TextColorStyle defaultTextColorStyle_;
};

// Once set, a color scheme may not be changed apart from overwriting it. In other words, it may not be
// deleted.
class SymbolColorScheme : public CharColorScheme
{
public:
    SymbolColorScheme(const TextColorStyle &defaultTextColorStyle = TextColorStyle());

    void setCharSymbolsColorStyle(char character, const QString &symbols, const TextColorStyle &textColorStyle)
    {
        // Populate charSymbolsColorStyles_
    }

    virtual TextColorStyle characterColor(char character, char symbol) const
    {
        if (charSymbolColorStyles_.contains(character))
            return charSymbolColorStyles_.value(character).value(symbol, CharColorScheme::defaultTextColorStyle());

        return CharColorScheme::defaultTextColorStyle();
    }

private:
    QHash<char, QHash<char, TextColorStyle> > charSymbolColorStyles_;
};

class BioSymbolColorScheme : public SymbolColorScheme
{
public:
    BioSymbolColorScheme(const BioSymbolGroup &bioSymbolGroup = BioSymbolGroup(), const TextColorStyle &defaultTextColorStyle = TextColorStyle());

    BioSymbolGroup bioSymbolGroup() const;
    void setBioSymbolGroup(const BioSymbolGroup &bioSymbolGroup)
    {

    }

    void setSymbolStyle(char ch, const TextColorStyle &textColorStyle)
    {
        symbolColorStyles_.insert(ch, textColorStyle);
        if (bioSymbolGroup_.bioSymbols().contains(ch))
            SymbolColorScheme::setCharacterTextColorStyle(ch, bioSymbolGroup_.bioSymbols().value(ch).characters(), textColorStyle);

    }

    virtual TextColorStyle symbolColor(char character, char symbol) const
    {
        if (characterColorStyles_.contains(character))
            return characterColorStyles_.value(character);

        if (symbolColorStyles_.contains(symbol)
            && bioSymbolGroup_.isCharAssociatedWithSymbol(character, symbol))
        {
            return symbolColorStyles_.value(symbol);
        }

        return defaultTextColorStyle_;
    }
    QHash<char, TextColorStyle> symbolStyles() const;

protected:
    BioSymbolGroup bioSymbolGroup_;
    QHash<char, TextColorStyle> symbolColorStyles_;
};




// http://ekhidna.biocenter.helsinki.fi/pfam2/colprot.par
// http://ekhidna.biocenter.helsinki.fi/pfam2/clustal_colours
class ClustalColorScheme : public SymbolColorScheme
{
    TextColorStyle symbolColor(char character, char symbol) const
    {
        switch (character)
        {
        case 'G':
            return orange_;
        case 'P':
            return yellow_;
        case 'T':
            switch(symbol)
            {
            case 't':
            case 'S':
            case 'T':
            case '%':
            case '#':
                return green_;
            }
            break;
        case 'S':
            switch(symbol)
            {
            case 't':
            case 'S':
            case 'T':
            case '#':
                return green_;
            }
            break;
        }
        case 'N':
            switch(symbol)
            {
            case 'n':
            case 'N':
            case 'D':
                return green_;
            }
            break;
        case 'Q':
            switch(symbol)
            {
            case 'q':
            case 'Q':
            case 'E':
            case '+':
            case 'K':
            case 'R':
                return green_;
            }
            break;
        case 'W':
        case 'L':
        case 'V':
        case 'I':
        case 'M':
        case 'F':
        case 'H':
        case 'Y':
            switch(symbol)
            {
            case '%':
            case '#':
            case 'A':
            case 'C':
            case 'F':
            case 'H':
            case 'I':
            case 'L':
            case 'M':
            case 'V':
            case 'W':
            case 'Y':
            case 'P':
            case 'p':
                return blue_;
            }
            break;
        case 'A':
            switch(symbol)
            {
            case '%':
            case '#':
            case 'A':
            case 'C':
            case 'F':
            case 'H':
            case 'I':
            case 'L':
            case 'M':
            case 'V':
            case 'W':
            case 'Y':
            case 'P':
            case 'p':
            case 'T':
            case 'S':
            case 's':
            case 'G':
                return blue_;
            }
            break;
        case 'C':
            switch(symbol)
            {
            case '%':
            case '#':
            case 'A':
            case 'C':
            case 'F':
            case 'H':
            case 'I':
            case 'L':
            case 'M':
            case 'V':
            case 'W':
            case 'Y':
            case 'P':
            case 'p':
                return blue_;
            case 'C':
                return pink_;
            }
            break;
        case 'E':
            switch(symbol)
            {
            case '-':
            case 'D':
            case 'E':
            case 'q':
            case 'Q':
                return magenta_;
            }
            break;
        case 'D':
            switch(symbol)
            {
            case '-':
            case 'D':
            case 'E':
            case 'n':
            case 'N':
                return magenta_;
            }
            break;
        case 'K':
        case 'R':
            switch(symbol)
            {
            case '+':
            case 'K':
            case 'R':
            case 'Q':
                return red_;
            }
            break;
    }

private:
    QColor red_;
    QColor blue_;
    QColor green_;
    QColor cyan_;
    QColor pink_;
    QColor magenta_;
    QColor yellow_;
    QColor orange_;
};


class SymbolColorProvider : public PositionalMsaColorProvider
{
public:
    SymbolColorProvider()
    TextColorStyle color(const Msa &msa, int i, int j)
    {
        return symbolColorScheme_.symbolColor(msa.at(i).bioString().sequence().mid(j, 1), liveSymbolString_.symbolAt(j));
    }

private:
    SymbolColorScheme symbolColorScheme_;
    LiveSymbolString liveSymbolString_;
}


class VissaColorProvider : public PositionalMsaColorProvider
{
public:
    TextColorSet color(const Msa &msa, int i, int j)
    {
        char msa_char = msa.at(i).bioString().sequence().mid(j, 1);
        char ss = store_->fetch(msa.at(i).id());
        if (ss == alpha helix)
        {
            foreground = black;
            background = red;
        }
        else
        {
            // ...
        }
    }

private:
    SecondaryStructureStore *store_;
};









class CharPixmapProvider : public QObject
{
    Q_OBJECT

public:
    CharPixmapProvider(QObject *parent = 0) : QObject(parent)
    {
        caching_ = true;
    }

    QPixmap glyph(char ch, QColor color)
    {
        QString key = glyphKey(ch, color);
        if (cachedGlpyhs_.contains(key))
            return cachedGlyphs_.value(key);

        QPixmap renderedGlyph = renderGlyph(ch, color);
        cachedGlyphs_.insert(key, renderedGlyph);
        return renderedGlyph;
    }
    bool isCaching() const;

public slots:
    void setCaching(bool cache);
    void clearCache()
    {
        cachedGlyphs_.clear();
        emit cacheCleared();
    }

signals:
    void cacheCleared();

protected:
    virtual QPixmap renderGlyph(char ch, QColor color) = 0;

private:
    QString glyphKey(char ch, QColor color)
    {
        return ch + color.name();
    }

    QHash<QString, QPixmap> cachedGlpyhs_;
    bool caching_;
};

class FontCharPixmapProvider : public CharPixmapProvider
{
    Q_OBJECT

public:
    FontCharPixmapProvider(QFont font, qreal scale = 1.0f);
    qreal height();
    qreal width(char ch);

    QPixmap renderGlyph(char ch, QColor color);

public slots:
    void setFont(QFont font);
    void setScale(qreal scale);

signals:
    void fontChanged();
    void scaleChanged();

private:
    QFont font_;
    qreal scale_;
};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Optimization of font drawing routines
// The following provides a zoomable option and then combined zoomable font class that only updates the actual glyphs
// when the zoom/scale has stabilized.

class ProxyCharPixmapProvider : CharPixmapProvider
{
public:
    void setSourceProvider(CharPixmapProvider *);
    CharPixmapProvider *sourceProvider() const;

private:
    CharPixmapProvider *sourceProvider_;
}

class ZoomCharPixmapProvider : public ProxyCharPixmapProvider
{
public:
    qreal zoom() const;
    QPixmap renderGlyph(char ch, QColor color);

public slots:
    void setZoom(qreal zoom);

signals:
    void zoomChanged();
    void zoomStabilized();
};


class DynamicFontCharPixmapProvider : public FontCharPixmapProvider
{
    Q_OBJECT

public:
    DynamicFontCharPixmapProvider()
    {
        zoomCharPixmapProvider_.setSourceProvider(this);
        connect(&zoomCharPixmapProvider_, SIGNAL(zoomStabilized()), SLOT(onZoomStabilized()));

        currentProvider_ = this;
    }

    QPixmap renderGlyph(char ch, QColor color)
    {
        if (currentProvider_ == this)
            return FontCharPixmapProvider::glyph(ch, color);

        return zoomCharPixmapProvider_.glyph(ch, color);
    }

    void setScale(qreal scale)
    {
        if (scale == scale())
            return;

        currentProvider_ = &zoomCharPixmapProvider_;
        zoomCharPixmapProvider_.setZoom(scaleToZoom(scale));
        emit scaleChanged();
    }

    qreal scale()
    {
        if (currentProvider_ == this)
            return FontCharPixmapProvider::scale();

        return FontCharPixmapProvider::scale() * zoomToScale(zoomCharPixmapProvider_.zoom());
    }

signals:
    void scaleStabilized();

private slots:
    void onZoomStabilized()
    {
        FontCharPixmapProvider::setScale(scale());
        currentProvider_ = this;
        emit scaleStabilized();
    }

private:
    qreal scaleToZoom(qreal scale);
    qreal zoomToScale(qreal zoom);

    ZoomCharPixmapProvider zoomCharPixmapProvider_;
    CharPixmapProvider *currentProvider_;
};
// End optimization of font
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


struct CharRenderData
{
    uchar character_;
    uchar[4] color_;
    uchar[4] backgroundColor_;
};

typedef QList<QList<CharRenderData> > CharRenderDataMatrix;

CharRenderDataMatrix colors(const Msa &msa, const QRect &rect);

// In repaint
{
    // A. Determine rectangle to render
    // B. Grab its color data
    drawCharacters(colors(*msa, QRect(...)));
}


class BioString : public QByteArray
{
public:
    BioString
};

class AnonSeq
{
public:
    AnonSeq(const BioString &bioString) : bioString_(bioString.ungapped())
    {
    }

    const BioString bioString_;
    int id_;
};

class Subseq
{
public:
    Subseq(const AnonSeq &anonSeq) : anonSeq_(anonSeq)
    {

    }

    const AnonSeq anonSeq_;
    Seq *seq_;
    QString name_;
    QString description_;
    QString custom_;
    QString notes_;
    QList<QString> tags_;

private:
    BioString bioString_;
};

class SimpleSeq
{
public:
private:

};

class Seq
{
public:
    Seq(const AnonSeq &anonSeq);

    int id_;
    const AnonSeq anonSeq_;
    QString name_;
    QString accession_;
    QString description_;
    QString source_;
    QString genus_;
    QString species_;
    QString organism_other_;
    QString custom_;
    QString notes_;
    QString organism() const { return genus_ + species_ + other_; }
    QString setOrganism(const QString &organism);
    QList<QString> tags_;
    QHash<QString, QList<Feature *> > features_;

    QString type() const {
        switch(anonSeq_.bioString_.alphabet)
        {
        case Protein:
            return "Protein";
            ...
        }
    }
    int length() const { return anonSeq_.bioString_.length(); }
    BioString sequence() const { return anonSeq_.bioString_; }
};

class DnaSeq : public Seq
{
public:
    DnaSeq(const AnonSeq &anonSeq);

    double gc() const;

};

class ProteinModel
{
public:
    void load(QList<int> ids);
    const ProtSeq *fetch(int id);

signals:
    void nameChanged(ProtSeq *protSeq);

private:
    QList<ProtSeq> protSeqs_;
    QHash<int, int> idLookup_;
};

class DisplayModel : public QAbstractItemModel
{
public:


private:
    ProteinModel *proteinModel_;

};


namespace Blargh
{
    class bob;

}



template<class T>
class ClosedRange
{
public:
    ClosedRange(T begin = 0, T end = 0) : begin_(begin), end_(end)
    {
    }

    bool isEmpty() const
    {
        return end_ < begin_;
    }

    T length() const
    {
        return qMax(0, end_ - begin_ + 1);
    }

    T begin_;
    T end_;
};

template<class T>
class HalfOpenRange
{
public:
    HalfOpenRange(T begin = 0, T end = 0) : begin_(begin), end_(end)
    {
    }

    bool isEmpty() const
    {
        return end_ == begin_;
    }

    T length() const
    {
        return qMax(0, end_ - begin_);
    }

    T begin_;
    T end_;
};

typedef ClosedRange<int> BioSegment;
typedef HalfOpenRange<double> PlotRange;

struct ClosedRange
{
    int begin_;
    int end_;

    ClosedRange(begin = 0, end = 0) : begin_(begin), end_(end)
    {

    }
};



QList<ClosedIntRange> ranges;
ranges.reserve(rect.height());
for (int i=top; i<= bottom; ++i)
{
    const Subseq *subseq = subseqs_[i-1];
    // Compute range to be updated for this sequence
    ranges << ClosedIntRange(4, 5);
}

emit aboutToExtendLeft(column, ranges);
for (int i=top, j=0; i<= bottom; ++i, ++j)
{
    Subseq *subseq = subseqs_[i-1];

    const char *seqX = subseq->parentSeq_.constData() + start_ - ranges[j].length() - 1;
    char *x = subseq->data() + ranges[j].begin_ - 1;

    memcpy(x, seqX, ranges[j].length());
}

emit extendedLeft(column, ranges);


class AgObject;

class AbstractRepo : public QObject
{
    Q_OBJECT

public:
    virtual int columnCount() const = 0;
    bool setData(AgObject *object, int column, QVariant value) = 0;

    virtual AgObject *find(int id) = 0;
    virutal void add(AgObject *object) = 0;
    virtual void remove(AgObject *object) = 0;
//    virtual void save(AgObject *object) = 0;      ? maybe

signals:
    void dataChanged(AgObject *object, int column);

private:
    QVector<AgObject *> objects_;
};

class AbstractAgTableModel : public QAbstactItemModel
{
    Q_OBJECT

public:
    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        return repo_->setData(static_cast<AgObject *>(index.internalPointer()), index.column(), value);
    }

    QVariant data(const QModelIndex &index)
    {
        AgObject *object = objects_.at(index.row());
        return data(object, index.column());
    }

protected:
    QVariant data(AgObject *object, int column) const = 0;

slots:
    void onAgObjectChanged(AgObject *object, int column)
    {
        int i = objects_.indexOf(object);
        if (i == -1)
            return;

        // Assume column is valid
        emit dataChanged(createIndex(i, column, objects_.at(i)));
    }

private:
    AbstractRepo *repo_;
    QVector<AgObject *> objects_;
};







class AbstractEntity
{
};


class AbstractAnonSeq : AbstractEntity
{
public:
    AbstractAnonSeq(int id, Seq seq_);

    int id() const;
    Seq seq() const;

private:
    int id_;
    Seq seq_;
};

class Domain
{
public:
    Domain(const QString &name, ClosedIntRange location, double score, double evalue);

    const QString name_;
    const ClosedIntRange location_;
    const double score_;
    const double evalue_;
};

class AminoAnonSeq : public AbstractAnonSeq
{
public:
    AminoAnonSeq(int id, Seq seq_);
    void setDomains(QVector<Domain> domains);
    void setSegs(QVector<ClosedIntRange> segs);

    QVector<Domain> domains() const;
    QVector<ClosedIntRange> segs() const;

private:
    QVector<Domain> domains_;
    QVector<ClosedIntRange> segs_;
};


class AbstractSeqRecord : public AbstractEntity
{
public:
    AbstractSeqRecord(int id, AbstractAnonSeq *abstractAnonSeq, const QString &name, const QString &description);
    int id() const;

    QString name_;
    QString description_;

private:
    int id_;
    AbstractAnonSeq *abstractAnonSeq_;
};

class AminoSeqRecord : public AbstractSeqRecord
{
public:
    AminoSeqRecord(int id, AminoAnonSeq *aminoAnonSeq, const QString &name, const QString &description);

    AminoAnonSeq *aminoAnonSeq() const;

private:
    AminoAnonSeq *aminoAnonSeq_;
};

class AbstractDataSource
{
public:

};


class AbstractDataMapper
{
public:
    AbstractDataMapper(AbstractDataSource *abstractDataSource);

    AbstractEntity *find(int id) const = 0;
    void save(AbstractEntity *abstractEntity) const = 0;
    void erase(AbstractEntity *abstractEntity) const = 0;

private:
    AbstractDataSource *abstractDataSource_;
};

class AminoAnonSeqMapper : public AbstractDataMapper
{
public:
    AminoAnonSeqMapper(AbstractDataSource *abstractDataSource);

    AminoAnonSeq *find(int id) const;
    void save(AminoAnonSeq *aminoAnonSeq) const;
    void erase(AminoAnonSeq *aminoAnonSeq) const;
};



template<typename T>
class Repository
{
public:
    Repository(AbstractDataMapper *abstractDataMapper);

    virtual T *find(int id)
    {
        if (!map_.contains(id))
        {
            T *object  = abstractDataMapper_->find(id);
            if (!object)
                return 0;

            map_[object.id()] = object;

            return object;
        }

        return map_.value(id);
    }

    virtual void save(T *object);
    virtual void remove(T *object);
    virtual void add(T *object);

private:
    AbstractDataMapper *abstractDataMapper_;
    QHash<int, T*> map_;
};


class AminoRecord; // public AbstractEntity

class IAminoRecordRepository : Repository<AminoRecord>
{
public:
    IAminoRecordRepository(Repository<AminoAnonSeq> aminoAnonSeqRepo);

    AminoRecord *find(int id)
    {

    }

private:
    Repository<AminoAnonSeq> amionAnonSeqRepo_;
};



template<typename T>
class IRepositoryModel : public Repository<T>
{
public:
    int columnCount() const = 0;
    bool setData(AbstractEntity *entity, int column, const QVariant &value) = 0;
    QVariant data(AbstractEntity *entity, int column) const = 0;

signals:
    void dataChanged(AbstractEntity *entity, int column);
    void entityRemoved(AbstractEntity *entity);
}

class AminoAnonSeqRepository : public IRepositoryModel<AminoAnonSeq>
{
public:
    enum Columns
    {
        eIdColumn = 0,
        eLengthColumn
    };

    AminoAnonSeqRepository(AbstractDataMapper *abstractDataMapper);

    int columnCount()
    {
        return 2;
    }

    bool setData(AminoAnonSeq *aminoAnonSeq, int column, const QVariant &value)
    {
        return false;

        // If there were properties that could be changed we would do that here
        switch(column)
        {
        case 0:
            aminoAnonSeq->setDomains(...);
            emit dataChanged(aminoAnonSeq, column);
            break;
        }
    }

    QVariant data(AminoAnonSeq *aminoAnonSeq, int column) const
    {
        switch (column)
        {
        case 0:
            return aminoAnonSeq->id_;
        case 1:
            break;
        }
    }
};


class GenericTableModel : public QAbstractItemModel
{
public:
    GenericTableModel(AbstractRepository *abstractRepository)
    {
        connect(abstractRepository_, SIGNAL(dataChanged(AbstractEntity*,int)), this, SLOT(onDataChanged(AbstractEntity*,int)));
        connect(abstractRepository_, SIGNAL(entityRemoved(AbstractEntity*)), this, SLOT(onEntityRemovedFromRepo(AbstractEntity*)));
    }

    int columnCount() const
    {
        return abstractRepository_->columnCount();
    }

    int rowCount() const
    {
        return entities_.count();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        return abstractRepository_->data(entities_.at(index.row()), index.column());
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        return abstractRepository_->setData(entities_.at(index.row()), index.column(), value);
    }

    void setEntities(QVector<AbstractEntity *> entities)
    {
        entities_ = entities;
        reset();
    }

private slots:
    void onDataChanged(AbstractEntity *entity, int column)
    {
        int i = entities_.indexOf(entity);
        if (i == -1)
            return;

        QModelIndex index = createIndex(i, column, entity);
        emit dataChanged(index, index);
    }

    void onEntityRemovedFromRepo(AbstractEntity *entity)
    {
        QVector<AbstractEntity *>::const_iterator it = qFind(entities_, entity);
        if (it == entities_.end())
            return;

        int i = it - entities_.begin();
        emit rowsAboutToBeRemoved(QModelIndex(), i, i);
        entities_.remove(i);
        emit rowsRemoved(QModelIndex(), i, i);
    }

private:
    QVector<AbstractEntity *> entities_;
    AbstractRepository *abstractRepository_;
};





Repository<AminoAnonSeq> aminoAnonSeqFactory(new AminoAnonSeqMapper(new AbstractDataSource));





// The above should work; however, it's a lot of work. How about, simply updating the relevant views whenever they are
// focused?
template<typename T>
class Repository
{
public:
    Repository(AbstractDataMapper *abstractDataMapper);

    T *find(id) = 0;
    QVector<T *> find(QVector<int> ids) = 0;
    void save(T *entity) = 0;
    void remove(T *entity) = 0;

protected:
    QVector<T *> entities_;
    QHash<int, T *> lookup_;
    AbstractDataMapper *abstractDataMapper_;
};


class ProtSeqRepoModel : public Repository<ProtSeq>
{
public:
    int columnCount() const
    {
        return 4;
    }

    Qt::ItemFlags flags(int column)
    {
        switch(column)
        {

        }
    }

    QVariant data(ProtSeq *protSeq, int column) const
    {
        switch (column)
        {
        case 0:
            return protSeq->id();
        case 1:
            return protSeq->name();
        default:
            return QVariant();
        }
    }

    bool setData(ProtSeq *protSeq, int column, QVariant value)
    {
        switch(column)
        {
        case 1:
            protSeq->setName(value.toString());
            emit dataChanged(protSeq, column);
            return true;
        }

        return false;
    }

signals:
    void dataChanged(ProtSeq *protSeq, column);
};


// Responsible for managing a collection of objects and communicating those changes
class ProtSeqSubsetModel : public QAbstractTableModel
{
public:
    ProtSeqSubsetModel(ProtSeqRepoModel *sourceModel);

    void setProtSeqs(QVector<ProtSeq *> protSeqs);

    int columnCount() const
    {
        return sourceModel->columnCount();
    }
    int rowCount() const
    {
        return protSeqs_.count();
    }
    QVariant data(const QModelIndex &index, int role) const
    {
        ProtSeq *protSeq = protSeqs_.at(index.row());
        return sourceModel_->data(protSeq, index.column());
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role)
    {
        ProtSeq *protSeq = protSeqs_.at(index.row());
        return sourceModel_->setData(protSeq, index.column(), value);
    }


private:
    QVector<ProtSeq *> protSeqs_;
    ProtSeqRepoModel *sourceModel_;
};




// New idea: have the entity classes contain static repository references
// All repos = singletons


class AminoRecordRepo
{
public:
    AminoRecordRepo()
    {
    }

    void load(QVector<int> ids, int token)
    {
        ds_->readAminoSeqs(ids, token, "onAminoSeqsRead");
    }

signals:
    void loaded(int token);

private slots:
    void onAminoSeqsRead(int token, QVector<DataRow> data)
    {
        // Return if not a recognized token

        QVector<int> astring_ids;
        QVector<AminoRecord *> newRecords;

        foreach (DataRow row, data)
        {
            AminoRecord *r = new AminoRecord;
            r->id_ = data.field("id");

            astring_ids << r->astring_id;
            newRecords << r;
        }

        processing_.insert(token, newRecords);

        int subtoken = aminoAnonSeqRepo_->load(astring_ids);
    }

    void onAnonSeqDataRead(int token)
    {

    }

private:
    AbstractDataSource *ds_;
    QHash<int, QVector<AminoRecord *> > processing_;
    QVector<AminoRecord *> aminoRecords_;
    AminoAnonSeqRepo *aminoAnonSeqRepo_;
};



// Complex objects that include hierarchy of value objects?

struct Domain
{
    int start;
    int stop;
    qreal score;
    QString name;
};

class IAdocDataSource
{
public:
    virtual void begin() {}

    virtual QVector<DataRow> readAminoAnonSeqs(QVector<int> ids) const = 0;
    virtual QVector<DataRow> readPfamDomains(QVector<int> anonSeqIds) const = 0;
    virtual QVector<DataRow> readAminoSeqs(QVector<int> ids) const = 0;

    virtual void end() {}   // For wrapping transactions

};



struct RepoRequest
{
    IRepository *repository_;
    QVector<int> ids_;
};

typedef int Token;

class IRequestChainHandler : public QObject
{
    Q_OBJECT

public:
    IRequestChainHandler(QObject *parent = 0);

signals:
    void requestDone(Token token);
    void requestError(Token token, QString error);
};

class RequestChainHandler : public IRequestChainHandler
{
public:
    RequestChainHandler(QObject *parent = 0);

    void load(QVector<RepoRequest> requests, Token parentToken)
    {
        sendNextRequest(RequestSet(requests, parentToken));
    }

private slots:
    void repoRequestDone(Token token)
    {
        ASSERT(requestSets_.contains(token));

        RequestSet &rs = requestSets_.take(token);
        sendNextRequest(rs);
    }

    void repoRequestError(Token token)
    {
        ASSERT(requestSets_.contains(token));

        RequestSet &rs = requestSets_.take(token);
        emit requestError(token, "Blargh!");
    }

private:
    struct RequestSet
    {
        QVector<RepoRequest<T> > requests_;
        typename QVector<RepoRequest<T> >::ConstIterator it_;
        Token parentToken_;

        RequestSet(Token parentToken) : parentToken_(parentToken)
        {
            it_ = requests_.begin();
        }

        RequestSet(QVector<RepoRequest<T> > requests, Token parentToken)
            : requests_(requests), parentToken_(parentToken)
        {
            it_ = requests_.begin();
        }
    };

    void sendNextRequest(const RequestSet &rs)
    {
        if (rs.it_ == rs.requests_.end())
        {
            emit requestDone((*rs.it_)->parentToken_);
            return;
        }
        ++rs.it_;

        connect((*rs.it_)->repository_, SIGNAL(loadDone(Token)), SLOT(repoRequestDone(Token)), Qt::UniqueConnection);
        connect((*rs.it_)->repository_, SIGNAL(loadError(Token)), SLOT(repoRequestError(Token)), Qt::UniqueConnection);

        int token = rand() * 100.;
        requestSets_.insert(token, rs);
        (*rs.it_)->repository_->load((*rs.it_)->ids_, token);
    }

    QHash<Token, RequestSet> requestSets_;
};


// Usage:
IRepository *aminoSeqRecordRepo = new IRepository();
IRepository *dnaSeqRecordRepo = new IRepository();

QVector<int> aminoSeqIds;
aminoSeqIds << 1 << 2 << 3;
QVector<int> dnaSeqIds;
dnaSeqIds << 4 << 5 << 6;

int myToken = rand() * 1000;

RequestChainHandler handler;
handler.load(QVector<RepoRequest>()
             << RepoRequest(aminoSeqRecordRepo, aminoSeqIds)
             << RepoRequest(dnaSeqRecordRepo, dnaSeqIds), myToken);



class GenericMapper : public QObject
{
    Q_OBJECT

public:
    GenericMapper(QObject *parent = 0) : QObject(parent)
    {
        connect(&handler, SIGNAL(requestDone(Token)), SLOT(handlerRequestDone(Token)));
        connect(&handler, SIGNAL(requestError(Token,QString)), SLOT(handlerRequestError(Token)));
    }

private slots:
    virtual void handlerRequestDone(Token token)
    {
    }

    virtual void handlerRequestError(Token token, QString error)
    {
    }

protected:
    RequestChainHandler handler_;
};

struct Pod
{};

class Obj
{
};

class Mapper : public GenericMapper
{
public:
    Mapper(IAdocSource *source, IRepository *relatedRepo, QObject *parent = 0) : GenericMapper(parent), source_(source)
    {
        connect(source, SIGNAL(loaded(QVector<Pod>, Token)), SLOT(sourceDataLoaded(QVector<Pod>,Token)));
        connect(&handler_, SIGNAL(handlerRequestDone(Token)), SLOT(handlerRequestDone(Token)));
    }


    void load(QVector<int> ids, Token token)
    {
        source_->loadPodData(ids, token);
    }

signals:
    void objectsReady(QVector<Obj *> objs, Token token);

private slots:
    void sourceDataLoaded(QVector<Pod> pods, Token token)
    {
        requests_.insert(token, pods);

        QVector<int> uniqueRelatedPodIds;   // Get these from the pods just retrieved from the database
        QVector<RepoRequest> requestage;
        requestage << RepoRequest(relatedRepo_, uniqueRelatedPodIds);

        handler_.load(requestage, token);
    }

    void handlerRequestDone(Token token)
    {
        ASSERT(requests_.contains(token));
        QVector<Pod> pods = requests_.take(token);
        // Build the actual objects using the relatedRepo find methods
        QVector<Obj *> objs;
        emit objectsReady(objs, token);
    }

private:
    IAdocSource *source_;
    QHash<Token, QVector<Pod> > requests_;
    IRepository *relatedRepo_;
};



class IPod
{
public:
    virtual int columnCount() const = 0;
    virtual QVariant data(int column) const = 0;
    virtual void setData(int column, QVariant value) = 0;
    virtual int type() const = 0;

    int id_;
};

class AminoAnonSeqPod : public IPod
{
public:
    AminoAnonSeqPod(int id, const QByteArray &digest, const QByteArray &sequence)
        : id_(id), digest_(digest), sequence_(sequence)
    {
    }

    virtual int columnCount() const
    {
        return 3;
    }
    virtual QVariant data(int column) const
    {
        switch (column)
        {
        case 0:
            return id_;
        case 1:
            return digest_;
        case 2:
            return sequence_;
        default:
            return QVariant();
        }
    }
    virtual void setData(int column, QVariant value)
    {
        switch(column)
        {
        case 0:
            id_ = value.toInt();
            break;
        case 1:
            digest_ = value.toByteArray();
            break;
        case 2:
            sequence_ = value.toByteArray();
            break;
        }
    }

    virtual int type() const
    {
        return 0;
    }

    QByteArray digest_;
    QByteArray sequence_;
};

class PodSqlBuilder
{
public:
    QString sql(int podType, int operation) const
    {
        switch(podType)
        {
        case 0:
            switch(operation)
            {
            case 0:
                return "INSERT INTO astrings (digest, sequence) VALUES (?, ?)";
            case 1:
                return QString();   // No updating astrings
            case 2:
                return "DELETE FROM astrings WHERE id = ?";
            case 3:
                return "SELECT id, digest, sequence FROM astrings WHERE id = ?";
            }
        case 1:

        }
    }

    QString insertSql(IPod *pod)
    {
        switch(pod->type())
        {
        case 0:
            return "INSERT INTO astrings (digest, sequence) VALUES (?, ?)";

        default:
            return QString();
        }
    }
    QString updateSql(IPod *pod)
    {
        switch(pod->type())
        {
        case 0:
            return QString();   // There is no updating astrings
        default:
            return QString();
        }
    }
    QString eraseSql(IPod *pod)
    {
        switch(pod->type())
        {
        case 0:
            return "DELETE FROM astrings WHERE id = ?";
        default:
            return QString();
        }
    }
    QString findSql(IPod *pod)
    {
        switch (pod->type())
        {
        case 0:
            return "SELECT id, digest, sequence FROM astrings WHERE id = ?";
        default:
            return QString();
        }
    }
};

class DbAdocSource
{
public:
    enum Operations
    {
        InsertOperation,
        EraseOperation,
        UpdateOperation,
        FindOperation
    };

    DbAdocSource(PodSqlBuilder sqlBuilder) : sqlBuilder_(sqlBuilder)
    {
    }

    void insert(IPod *pod)
    {
        QSqlQuery query = getPreparedQuery(pod->type(), InsertOperation, sqlBuilder_.insertSql(pod));

        for (int i=1, z=pod->columnCount(); i<z; ++i)
            query.bindValue(i-1, pod->data(i));

        if (!query.exec())
            throw 0;

        pod->id_ = query.lastInsertId().toInt();
    }

    void insert(QVector<IPod *> &pods)
    {

    }

    int erase(int podType, int id)
    {
        QSqlQuery query = getPreparedQuery(podType, EraseOperation, sqlBuilder_.eraseSql(podType));

        query.bindValue(0, id);

        if (!query.exec())
            throw 0;

        return query.numRowsAffected();
    }

    void find(IPod *pod, int id)
    {
        QSqlQuery query = getPreparedQuery(pod.type(), FindOperation, sqlBuilder_.findSql(pod.type()));

        query.bindValue(0, id);
        if (!query.exec())
            throw 0;

        if (!query.next())
            return;

        for (int i=0, z=pod->columnCount(); i<z; ++i)
            pod->setData(i, query.value(i));
    }

    void update(IPod *pod)
    {
        QSqlQuery query = getPreparedQuery(pod.type(), UpdateOperation, sqlBuilder_.findSql(pod.type()));

        for (int i=1, z=pod->columnCount(); i<z; ++i)
            query.bindValue(i-1, pod->data(i));

        query.bindValue(pod->columnCount(), pod->id_);
        if (!query.exec())
            throw 0;
    }

private:
    QSqlDatabase database();
    QSqlQuery getPreparedQuery(int key, int operation, const QString &sql)
    {
        ASSERT(sql.isEmpty() == false);
        switch (operation)
        {
        case InsertOperation:
            if (preparedInserts_.contains(key))
                return preparedInserts_.value(key);

            // Prepare the query
            QSqlQuery query(database());
            if (!query.prepare(sql))
                throw 0;

            preparedInserts_.insert(key, sql);

            return query;
        }
    }

    PodSqlBuilder sqlBuilder_;
    QHash<int, QSqlQuery> preparedInserts_;
    QHash<int, QSqlQuery> preparedErases_;
    QHash<int, QSqlQuery> preparedUpdates_;
    QHash<int, QSqlQuery> preparedFinds_;
};




struct Pod
{
    int type_;
    int id_;
    int columns_;
    QVector<QVariant> data_;
};


class DbAdocSource
{
public:
    enum Operations
    {
        InsertOperation,
        EraseOperation,
        UpdateOperation,
        FindOperation
    };

    DbAdocSource(PodSqlBuilder sqlBuilder) : sqlBuilder_(sqlBuilder)
    {
    }

    void insert(Pod &pod)
    {
        QSqlQuery query = getPreparedQuery(pod.type_, InsertOperation, sqlBuilder_.insertSql(pod));

        for (int i=1; i<pod.columns_; ++i)
            query.bindValue(i-1, pod.data_.at(i));

        if (!query.exec())
            throw 0;

        pod->id_ = query.lastInsertId().toInt();
    }

    void insert(QVector<Pod> &pods)
    {
        if (pods.isEmpty())
            return;

        QSqlQuery query = getPreparedQuery(pods.at(0).type_, InsertOperation, sqlBuilder_.insertSql(pods.at(0)));

        foreach (Pod &pod, pods)
        {
            for (int i=1; i<pod.columns_; ++i)
                query.bindValue(i-1, pod.data_.at(i));

            if (!query.exec())
                throw 0;

            pod->id_ = query.lastInsertId().toInt();
        }
    }

    int erase(int podType, int id)
    {
        QSqlQuery query = getPreparedQuery(podType, EraseOperation, sqlBuilder_.eraseSql(podType));

        query.bindValue(0, id);

        if (!query.exec())
            throw 0;

        return query.numRowsAffected();
    }

    Pod find(int podType, int id)
    {
        QSqlQuery query = getPreparedQuery(pod.type(), FindOperation, sqlBuilder_.findSql(pod.type()));

        query.bindValue(0, id);
        if (!query.exec())
            throw 0;

        if (!query.next())
            return;

        for (int i=0, z=pod->columnCount(); i<z; ++i)
            pod->setData(i, query.value(i));
    }

    Pod findRelated(int )

    void update(IPod *pod)
    {
        QSqlQuery query = getPreparedQuery(pod.type(), UpdateOperation, sqlBuilder_.findSql(pod.type()));

        for (int i=1, z=pod->columnCount(); i<z; ++i)
            query.bindValue(i-1, pod->data(i));

        query.bindValue(pod->columnCount(), pod->id_);
        if (!query.exec())
            throw 0;
    }

    void sync(int podtype, int foreignKeyId, QVector<Pod> &pods)
    {
        // Build list of valid ids
        QString validIds;
        foreach (const Pod &pod, &pods)
        {
            if (pod.id_ > 0)
            {
                if (!validIds.isEmpty())
                    validIds += QString(",%1").arg(pod.id_);
                else
                    validIds = QString::number(pod.id_);
            }
        }

        // Step B: Delete those entries that are not associated with this object
        if (validIds.isEmpty() == false)
        {
            QSqlQuery query(database());
            if (!query.exec(QString("DELETE FROM coils WHERE astring_id = %1 AND NOT id IN (%2)").arg(aminoAnonSeqId).arg(validIds)))
                throw 0;
        }
        else
        {
            // Remove all coils
            QSqlQuery query = getPreparedQuery("deleteCoilsForAstring",
                                                "DELETE FROM coils WHERE astring_id = ?");

            query.bindValue(0, aminoAnonSeqId);
            if (!query.exec())
                throw 0;
        }


    }

private:
    QSqlDatabase database();
    QSqlQuery getPreparedQuery(int key, int operation, const QString &sql)
    {
        ASSERT(sql.isEmpty() == false);
        switch (operation)
        {
        case InsertOperation:
            if (preparedInserts_.contains(key))
                return preparedInserts_.value(key);

            // Prepare the query
            QSqlQuery query(database());
            if (!query.prepare(sql))
                throw 0;

            preparedInserts_.insert(key, sql);

            return query;
        }
    }

    PodSqlBuilder sqlBuilder_;
    QHash<int, QSqlQuery> preparedInserts_;
    QHash<int, QSqlQuery> preparedErases_;
    QHash<int, QSqlQuery> preparedUpdates_;
    QHash<int, QSqlQuery> preparedFinds_;
};



// Normal signature with static method
QHash<AdocNodeType, AdocTreeNodeVector> extractFlat(const AdocTreeNodeVector &nodeVector, bool (*acceptNode)(AdocTreeNode *node));

// C++ signature for using arbitrary object
QHash<AdocNodeType, AdocTreeNodeVector> extractFlat(const AdocTreeNodeVector &nodeVector,
                                                    void *object,
                                                    bool (*acceptNode)(void *object, AdocTreeNode *node))
{
    while (1)
    {
        // Iterating over tree
        AdocTreeNode *node = nextNode();
        if ((*acceptNode)(object, node))
        {
            // Keep the node
        }
        else
        {
            // Ignore the node
        }
    }
}

// Example using a specific class instance
class Model
{
public:
    bool acceptNode(AdocTreeNode *adocTreeNode) const;
    static bool acceptNode(void *object, AdocTreeNode *adocTreeNode)
    {
        ASSERT(dynamic_cast<Model *>(object) != 0);
        Model *self = static_cast<Model *>(object);
        return self->acceptNode(adocTreeNode);
    }
};

Model model;
extractFlat(..., (void *)&model, Model::acceptNode)




class IFilterColumnAdapter : public IColumnAdapter
{
public:
    virtual void setSourceAdapter(IColumnAdapter *sourceAdapter) = ;
    virtual IColumnAdapter *sourceAdapter() const = 0;

protected:
    virtual bool filterAcceptsColumn(int column) = 0;
};

class FilterColumnAdapter : public IFilterColumnAdapter
{
public:
    FilterColumnAdapter(QVector<int> columnsToExclude);

    void exclude(int column)
    {
        excludedColumns_ << column;
        updateMapping();
    }

    void include(int column)
    {
        excludedColumns_.remove(column);
        updateMapping();
    }

    int columnCount() const
    {
        return mapping_.size();
    }

    int mapToSource(int column) const
    {
        ASSERT(column >= 0 && column < mapping_.size());
        return mapping_.at(column);
    }

    int mapFromSource(int column) const
    {
        ASSERT(column >= 0 && column < sourceAdapter_->columnCount());
        return mapping_.indexOf(column);
    }

    QVariant data(int column)
    {
        return sourceAdapter_->data(mapToSource(column));
    }

private Q_SLOTS:
    void onSourceDataChanged(IEntity *entity, int column);

private:
    void updateMapping()
    {
        mapping_.resize(columnCount());
        mapping_.clear();
        for (int i=0; i< sourceAdapter_->columnCount(); ++i)
        {
            if (excludedColumns_.contains(i))
                continue;

            mapping_ << i;
        }
    }

    IColumnAdapter *sourceAdapter_;
    QVector<int> mapping_;
    QSet<int> excludedColumns_;
};


namespace Ag
{
    enum TaskStatus
    {
        NotStarted = 0,
        Starting,
        Running,
        Paused,
        Finished,
        Error,
        Killed
    };
}


class ITask : public QObject
{
    Q_OBJECT

public:
    virtual QString name() const = 0;
    virtual QString note() const = 0;                   // This is for individual tasks to supply information to the task manager (e.g. errors, etc.)
    virtual Ag::TaskStatus status() const = 0;
    virtual double progress() const = 0;
    virtual double timeInSeconds() const = 0;
    virtual int minThreads() const = 0;
    virtual int maxThreads() const = 0;
    virtual ~ITask() {}

Q_SIGNALS:
    void taskDone(ITask *self);
    void taskError(ITask *self);

public Q_SLOTS:
    virtual void setName(const QString &name) = 0;
    virtual void setNote(const QString &note) = 0;
    virtual void setStatus(const Ag::TaskStatus taskStatus) = 0;
    virtual void setMinThreads(const int minThreads) = 0;
    virtual void setMaxThreads(const int maxThreads) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;        // Also behaves as pause
    virtual void kill() = 0;

protected:
    ITask(ITask *parentTask = 0) {}
};

class Task : public ITask
{
    Q_OBJECT

public:
    Task(const QString &name, ITask *parentTask = 0);

    virtual QString name() const;
    virtual QString note() const;
    virtual Ag::TaskStatus status() const;
    virtual double progress() const;
    virtual double timeInSeconds() const;
    virtual int minThreads() const;
    virtual int maxThreads() const;

public Q_SLOTS:
    virtual void setName(const QString &name);
    virtual void setNote(const QString &note);
    virtual void setStatus(const Ag::TaskStatus taskStatus);
    virtual void setMinThreads(const int minThreads);
    virtual void setMaxThreads(const int maxThreads);

    virtual void start();
    virtual void stop();        // Also behaves as pause
    virtual void kill();

private:
    QString name_;
    QString note_;
    Ag::TaskStatus status_;
    double progress_;
    double timeInSeconds_;
    int minThreads_;
    int maxThreads_;

    QList<ITask *> childTasks_;
};

class PsiblastStructureTask : public Task
{
public:
    PsiblastStructureTask(PsiBlastConfig &config, Adoc *adoc, int aminoSeqId, ITask *parentTask);

    void start()
    {
        // Get the amino seq
        AminoSeq *aminoSeq = adoc_.aminoSeqRepository()->findOne(aminoSeqId_);
        Astring *astring = aminoSeq->anonSeq();
        astringId_ = astring->id();
        sequence_ = astring->seq_.asByteArray();
        adoc_.aminoSeqRepository()->unfindOne(aminoSeq);

        psitool_ = new PsiblastStructureTool(...);
        connect(...);

        psitool_->predictSecondary(astringId_, sequence_);
    }

    // There is no stopping an individual psiblast in the middle :\

private Q_SLOTS:
    void onPsiblastError();
    void onPsiBlastDone()
    {

    }

private:
    Adoc *adoc_;
    int aminoSeqId_;
    int astringId_;
    BioString sequence_;

    PsiBlastConfig psiConfig_;
    PsiBlastStructureTool psitool_;
};

// Unnecessary group class apart from possibly aggregating the overall progress and time consumed
//class PsiBlastStructureGroup : public Task
//{
//    PsiblastStructureTask(PsiBlastConfig &config, Adoc *adoc, const QVector<int> &aminoSeqIds, ITask *parentTask);

//};

#include <QtCore/QQueue>

class TaskManager : public QObject
{
    Q_OBJECT

public:

public Q_SLOTS:
    void start();
    void stop();

private:
    QList<ITask *> activeTasks_;
    QQueue<ITask *> queuedTasks_;

    bool autoStartTasks_;

    int maxThreads_;
    int maxTasks_;
};


class IIdentifiable
{
public:
    virtual const int id() const = 0;
    virtual bool isDirty() const = 0;
    virtual const int dirtyFlags() const = 0;
    virtual void setDirty(int dirtyFlag, bool dirty) const = 0;
    virtual const int type() const = 0;
};

class ValueObject : public IIdentifiable
{
};

class Coil : public ValueObject
{
};


class IValueObjectColumnAdapter
{
public:
    virtual bool setData(const ValueObject &valueObject, int column, const QVariant &newValue) = 0;


Q_SIGNALS:
    void dataChanged(const int id, const int column);
};

class AbstractValueObjectColumnAdapter: public IValueObjectColumnAdapter
{
public:
    virtual bool setData(const ValueObject &valueObject, int column, const QVariant &newValue)
    {
        return setDataPrivate(valueObject, column, newValue);
    }

protected:
    virtual bool setDataPrivate(const ValueObject &valueObject, int column, const QVariant &newValue) = 0;
};

class CoilColumnAdapter : public AbstractValueObjectColumnAdapter
{
public:
    enum Columns
    {
        eStartColumn = 0,
        eStopColumn,

        eNumberOfColumns
    };

protected:
    virtual bool setDataPrivate(const ValueObject &valueObject, int column, const QVariant &newValue)
    {
        ASSERT(valueObject.type() == eCoilType);

        const Coil &coil = static_cast<const Coil &>(valueObject);
        switch (column)
        {
        case eStartColumn:
            coil.setStart(newValue.toInt());
            emit dataChanged(coil.id(), eStartColumn);
            return true;
        }
    }
};
