/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef MULTISEQTABLEMODEL_H
#define MULTISEQTABLEMODEL_H

#include <QtGui/QFont>

#include "AbstractMultiEntityTableModel.h"

class MultiSeqTableModel : public AbstractMultiEntityTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        eNameColumn = 0,
        eIdColumn,
        eEntityTypeColumn,
        eSourceColumn,
        eStartColumn,
        eStopColumn,
        eLengthColumn,
        eDescriptionColumn,
        eNotesColumn,
        eSequenceColumn,

        eNumberOfColumns
    };

    MultiSeqTableModel(QObject *parent = nullptr) : AbstractMultiEntityTableModel(parent)
    {
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);

        return eNumberOfColumns;
    }

    int groupLabelColumn() const
    {
        return eNameColumn;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role == Qt::FontRole && index.column() == eSequenceColumn)
        {
            QFont font("monospace");
            font.setStyleHint(QFont::TypeWriter);
            return font;
        }

        return AbstractMultiEntityTableModel::data(index, role);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        if (orientation != Qt::Horizontal)
            return AbstractMultiEntityTableModel::headerData(section, orientation, role);

        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case eIdColumn:
                return "System ID";
            case eEntityTypeColumn:
                return "Type";
            case eNameColumn:
                return "Name";
            case eSourceColumn:
                return "Organism";
            case eStartColumn:
                return "Start";
            case eStopColumn:
                return "Stop";
            case eLengthColumn:
                return "Length";
            case eDescriptionColumn:
                return "Description";
            case eNotesColumn:
                return "Notes";
            case eSequenceColumn:
                return "Sequence";

            default:
                return QVariant();
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignLeft;
        }

        return QVariant();
    }

public Q_SLOTS:
    /**
      * Usually called after the document has been saved and some entity ids have been potentially updated.
      */
    void refreshEntityIds()
    {
        // Cheat by refreshing the view
        sort();

//        emit dataChanged(index(0, eIdColumn), index(rowCount(), eIdColumn));
    }
};

#endif // MULTISEQENTITYTABLEMODEL_H
