/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/
#ifndef OBSERVABLESEQUENCE_H
#define OBSERVABLESEQUENCE_H

#include <QtCore>
#include "PrimerDesign/DnaSequence.h"

using namespace PrimerDesign;

namespace PrimerDesign
{
    /// Represents an observable sequence.
    class ObservableSequence : public QObject, public DnaSequence
    {
        Q_OBJECT

    public:
        /// Constructs an observable sequence.
        /// @param parent QObject * (Defaults to 0.)
        ObservableSequence(QObject *parent = 0);

        /// Sets the sequence name.
        /// @param name const QString &
        /// @return bool
        virtual bool setName(const QString &name);

    signals:
        /// The name changed event.
        /// @param sender ObservableSequence *
        /// @return void
        void nameChanged(ObservableSequence *sender);
    };
}

Q_DECLARE_METATYPE(ObservableSequence *);

#endif // OBSERVABLESEQUENCE_H
