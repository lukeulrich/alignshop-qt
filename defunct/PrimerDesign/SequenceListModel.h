/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef SEQUENCELISTMODEL_H
#define SEQUENCELISTMODEL_H

#include <QtCore>
#include <QtGui>
#include <PrimerDesign/PrimerPairGroup.h>
#include <PrimerDesign/ListModel.h>

namespace PrimerDesign
{
    /// The model for a sequence list
    class SequenceListModel : public ListModel
    {
        Q_OBJECT

    public:
        /// The default constructor.
        SequenceListModel();

        /// Binds to the specified list.
        /// @param list ObservableListBase *
        /// @return void
        virtual void bind(ObservableListBase *list);


        /// Gets the specified data.
        /// @param index const QModelIndex &
        /// @param role int (Defaults to Qt::DisplayRole.)
        /// @return QVariant
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        /// Sets the specified data.
        /// @param index const QModelIndex &
        /// @param value const QVariant &
        /// @param role int (Defaults to Qt::EditRole.)
        /// @return bool
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    private slots:
        /// Binds to the currently selected sequence.
        /// @return void
        void bindCurrent();

        /// Triggered when the current sequence's name changes.
        /// @param sequence ObservableSequence *
        /// @return void
        void onNameChanged(ObservableSequence *sequence);

        /// Unbinds from the currently selected sequence.
        /// @return void
        void unbindCurrent();
    };
}

#endif // SEQUENCELISTMODEL_H
