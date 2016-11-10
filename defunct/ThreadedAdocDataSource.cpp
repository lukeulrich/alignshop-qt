/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "ThreadedAdocDataSource.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QStringList>
#include <QtCore/QThread>

#include "SynchronousAdocDataSource.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
ThreadedAdocDataSource::ThreadedAdocDataSource(QObject *parent) : AbstractAdocDataSource(parent)
{
    // Cannot parent synchronousAdocDataSource_ or else we will not be able to move it to a different
    // thread.
    synchronousAdocDataSource_ = new SynchronousAdocDataSource;

    connect(synchronousAdocDataSource_, SIGNAL(opened()), SIGNAL(opened()));
    connect(synchronousAdocDataSource_, SIGNAL(openError(QString, QString)), SIGNAL(openError(QString,QString)));
    connect(synchronousAdocDataSource_, SIGNAL(createError(QString,QString)), SIGNAL(createError(QString,QString)));
    connect(synchronousAdocDataSource_, SIGNAL(createDone(QString)), SIGNAL(createDone(QString)));
    connect(synchronousAdocDataSource_, SIGNAL(closed()), SIGNAL(closed()));
    connect(synchronousAdocDataSource_, SIGNAL(dataTreeError(QString)), SIGNAL(dataTreeError(QString)));
    connect(synchronousAdocDataSource_, SIGNAL(dataTreeReady(AdocTreeNode*)), SIGNAL(dataTreeReady(AdocTreeNode*)));
    connect(synchronousAdocDataSource_, SIGNAL(saveAsDone(QString)), SIGNAL(saveAsDone(QString)));
    connect(synchronousAdocDataSource_, SIGNAL(saveAsError(QString,QString)), SIGNAL(saveAsError(QString,QString)));

    thread_ = new QThread(this);
    synchronousAdocDataSource_->moveToThread(thread_);      // I think that this thread_ becomes the QObject parent
                                                            // synchronousAdocDataSource_ and thus when thread_ is
                                                            // deleted, so does this instance.
    thread_->start();
}

/**
  * Close down the data source and free any allocated memory associated with the anonSeqFactories.
  */
ThreadedAdocDataSource::~ThreadedAdocDataSource()
{
    if (synchronousAdocDataSource_)
        close();

    QMetaObject::invokeMethod(thread_, "quit");

    // Give it a max of 30 seconds to complete and terminate it if it is not finished by then
    if (!thread_->wait(30))
        thread_->terminate();

    delete thread_;
    thread_ = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QHash<Alphabet, AnonSeqFactory *>
  */
QHash<Alphabet, DbAnonSeqFactory *> ThreadedAdocDataSource::anonSeqFactories() const
{
    ASSERT(synchronousAdocDataSource_);

    return synchronousAdocDataSource_->anonSeqFactories_;
}

/**
  * If possible, it is preferable to utilize one of the other methods for database interaction as these
  * are thread-safe
  *
  * Very dangerous to call outside of owning thread! Beware!
  *
  * @returns QSqlDatabase
  */
const QSqlDatabase ThreadedAdocDataSource::getDatabase() const
{
    return synchronousAdocDataSource_->database_;
}

/**
  * Locks the private database_ instance for reading and returns if it is open.
  *
  * @returns bool
  */
bool ThreadedAdocDataSource::isOpen() const
{
    ASSERT(synchronousAdocDataSource_);

    QMutexLocker locker(&mutex_);

    return synchronousAdocDataSource_->database_.isOpen();
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param alphabet [Alphabet]
  * @param anonSeqFactory [DbAnonSeqFactory *]
  */
void ThreadedAdocDataSource::setAnonSeqFactory(Alphabet alphabet, DbAnonSeqFactory *anonSeqFactory)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "setAnonSeqFactory", Q_ARG(Alphabet, alphabet), Q_ARG(DbAnonSeqFactory *, anonSeqFactory));
}

/**
  * @returns QString
  */
QString ThreadedAdocDataSource::sourceFile() const
{
    ASSERT(synchronousAdocDataSource_);

    return synchronousAdocDataSource_->sourceFile_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param file [const QString &]
  */
void ThreadedAdocDataSource::open(const QString &file)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "open", Q_ARG(QString, file));
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  */
void ThreadedAdocDataSource::close()
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "close");
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param file [const QString &]
  * @param ddlQueries [const QStringList &]
  */
void ThreadedAdocDataSource::create(const QString &file, const QStringList &ddlQueries)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "create", Q_ARG(QString, file), Q_ARG(QStringList, ddlQueries));
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  */
void ThreadedAdocDataSource::readDataTree()
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "readDataTree");
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  */
void ThreadedAdocDataSource::readMsa(int id, Alphabet alphabet, int tag)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "readMsa", Q_ARG(int, id), Q_ARG(Alphabet, alphabet), Q_ARG(int, tag));
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param file [const QString &]
  */
void ThreadedAdocDataSource::saveAs(const QString &file)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "saveAs", Q_ARG(QString, file));
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param sql [const QString &]
  * @param input [const QList<QVariant> &]
  * @param receiver [QObject *]
  * @param method [const char *]
  * @param errorMethod [const char *]
  * @param tag [int]
  */
void ThreadedAdocDataSource::select(const QString &sql, const QList<QVariant> &input, QObject *receiver, const char *method, const char *errorMethod, int tag)
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "select", Q_ARG(QString, sql), Q_ARG(QList<QVariant>, input), Q_ARG(QObject *, receiver), Q_ARG(const char *, method), Q_ARG(const char *, errorMethod), Q_ARG(int, tag));
}

/**
  * Delegated to the cognate synchronousAdocDataSource_ method.
  *
  * @param dataRow [const DataRow &]
  * @param tableName [const QString &]
  * @param fields [const QStringList &]
  * @param receiver [QObject *]
  * @param method [const char *]
  * @param errorMethod [const char *]
  * @param tag [int]
  */
void ThreadedAdocDataSource::update(DataRow &dataRow, const QString &tableName, const QStringList &fields, QObject *receiver, const char *method, const char *errorMethod, int tag) const
{
    ASSERT(synchronousAdocDataSource_);

    QMetaObject::invokeMethod(synchronousAdocDataSource_, "update", Q_ARG(DataRow &, dataRow), Q_ARG(QString, tableName), Q_ARG(QStringList, fields), Q_ARG(QObject *, receiver), Q_ARG(const char *, method), Q_ARG(const char *, errorMethod), Q_ARG(int, tag));
}
