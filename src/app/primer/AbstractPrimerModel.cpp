/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QFont>

#include "AbstractPrimerModel.h"
#include "IPrimerMutator.h"
#include "../core/misc.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
AbstractPrimerModel::AbstractPrimerModel(IPrimerMutator *primerMutator, QObject *parent)
    : QAbstractTableModel(parent),
      primerMutator_(primerMutator),
      bioString_(eDnaGrammar)
{
    observePrimerMutator();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
BioString AbstractPrimerModel::bioString() const
{
    return bioString_;
}

int AbstractPrimerModel::columnCount(const QModelIndex & /* parent */) const
{
    return eNumberOfColumns;
}

QVariant AbstractPrimerModel::data(const QModelIndex &index, int role) const
{
    const Primer *primer = primerPointerFromIndex(index);
    if (primer == nullptr)
        return QVariant();

    int primerColumn = index.column();

    switch (role)
    {
    case Qt::DisplayRole:           return displayRoleData(*primer, primerColumn);
    case Qt::EditRole:              return editRoleData(*primer, primerColumn);
    case Qt::FontRole:              return fontRoleData(primerColumn);
    case Qt::TextAlignmentRole:     return textAlignmentRole(primerColumn);

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
        if (role == Qt::DisplayRole)
            return verticalHeaderDisplayRoleData(section);

        return QVariant();
    }

    // Assume horizontal
    switch (role)
    {
    case Qt::DisplayRole:   return horizontalHeaderDisplayRoleData(section);
    case Qt::ToolTipRole:   return horizontalHeaderToolTipRoleData(section);

    default:
        return QVariant();
    }
}

bool AbstractPrimerModel::indexHasPrimerSearchParameters(const QModelIndex &index) const
{
    const Primer *primer = primerPointerFromIndex(index);
    return primer != nullptr && primer->primerSearchParameters() != nullptr;
}

bool AbstractPrimerModel::isValidRow(const int row) const
{
    return row >= 0 && row < rowCount();
}

Primer AbstractPrimerModel::primerFromIndex(const QModelIndex &index) const
{
    const Primer *primer = primerPointerFromIndex(index);
    if (primer == nullptr)
        return Primer();

    return *primer;
}

IPrimerMutator *AbstractPrimerModel::primerMutator() const
{
    return primerMutator_;
}

void AbstractPrimerModel::setBioString(const BioString &newBioString)
{
    if (newBioString == bioString_)
        return;

    bioString_ = newBioString;
}

bool AbstractPrimerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Primer *primer = primerPointerFromIndex(index);
    if (primer == nullptr)
        return false;

    if (role != Qt::EditRole)
        return false;

    int primerColumn = index.column();
    if (primerColumn != eNameColumn)
        return false;

    if (primerMutator_ == nullptr)
        return false;

    return primerMutator_->setPrimerName(*primer, value.toString());
}

int AbstractPrimerModel::primerId(const int row) const
{
    const Primer *primer = primerPointerFromRow(row);
    if (primer == nullptr)
        return 0;

    return primer->id();
}

Primer AbstractPrimerModel::primerFromRow(const int row) const
{
    const Primer *primer = primerPointerFromRow(row);
    if (primer == nullptr)
        return Primer();

    return *primer;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
Primer *AbstractPrimerModel::primerPointerFromIndex(const QModelIndex &index)
{
    return primerPointerFromRow(index.row());
}

const Primer *AbstractPrimerModel::primerPointerFromIndex(const QModelIndex &index) const
{
    return primerPointerFromRow(index.row());
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
void AbstractPrimerModel::onPrimerMutatorNameChanged(int primerId)
{
    int primerRow = Primer::primerRowById(primers(), primerId);
    if (primerRow == -1)
        return;

    QModelIndex primerNameIndex = index(primerRow, eNameColumn);
    emit dataChanged(primerNameIndex, primerNameIndex);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
void AbstractPrimerModel::observePrimerMutator()
{
    if (primerMutator_ == nullptr)
        return;

    connect(primerMutator_, SIGNAL(primerNameChanged(int)), SLOT(onPrimerMutatorNameChanged(int)));
}

QVariant AbstractPrimerModel::displayRoleData(const Primer &primer, int column) const
{
    switch (column)
    {
    case eNameColumn:
        return primer.name();
    case eRestrictionEnzymeNameColumn:
        return primer.restrictionEnzyme().name();
    case eRestrictionEnzymeSequenceColumn:
        return primer.restrictionEnzyme().recognitionSite().asByteArray();
    case eCoreSequenceColumn:
        return primer.coreSequence().asByteArray();
    case eSequenceColumn:
        return primer.sequence().asByteArray();
    case eTmColumn:
        return ::round(primer.tm(), 1);
    case eStrandColumn:
        return primerStrand(primer);
    case eMessageColumn:
        {
            int primerMatches = primer.countCoreSequenceMatchesIn(bioString_);
            if (primerMatches == 0)
                return "Primer sequence not found within target sequence.";
            else if (primerMatches > 1)
                return "Warning: primer sequence matches found in multiple locations.";
        }
        return QVariant();

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::editRoleData(const Primer &primer, int column) const
{
    switch(column)
    {
    case eNameColumn:
        return primer.name();

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::fontRoleData(int column) const
{
    switch (column)
    {
    case eRestrictionEnzymeSequenceColumn:
    case eCoreSequenceColumn:
    case eSequenceColumn:
        return QFont("monospace");

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::textAlignmentRole(int column) const
{
    switch (column)
    {
    case eStrandColumn:
    case eTmColumn:
        return Qt::AlignCenter;

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::verticalHeaderDisplayRoleData(int section) const
{
    return QString("%1.").arg(section + 1);
}

QVariant AbstractPrimerModel::horizontalHeaderDisplayRoleData(int section) const
{
    switch (section)
    {
    case eNameColumn:
        return "Name";
    case eRestrictionEnzymeNameColumn:
        return "RE";
    case eRestrictionEnzymeSequenceColumn:
        return "RE Site";
    case eCoreSequenceColumn:
        return "Core sequence";
    case eSequenceColumn:
        return "Sequence";
    case eTmColumn:
        return "Tm (°C)";
    case eStrandColumn:
        return "Strand";
    case eMessageColumn:
        return "Message";

    default:
        return QVariant();
    }
}

QVariant AbstractPrimerModel::horizontalHeaderToolTipRoleData(int section) const
{
    switch (section)
    {
    case eRestrictionEnzymeNameColumn:
        return "Restriction Enzyme";
    case eRestrictionEnzymeSequenceColumn:
        return "Restriction Enzyme recognition site";
    case eTmColumn:
        return "Melting temperature of primer sequence";

    default:
        return QVariant();
    }
}

QString AbstractPrimerModel::primerStrand(const Primer &primer) const
{
    int forwardStart = primer.locateCoreSequenceStartIn(bioString_);
    int reverseStart = primer.locateCoreSequenceStartInCognateStrand(bioString_);

    if (forwardStart > 0 && reverseStart > 0)
        return "+/-";
    else if (forwardStart == -1 && reverseStart == -1)
        return QString();
    else if (forwardStart == -1)
        return "-";
    else if (reverseStart == -1)
        return "+";

    // Technically, control should be never reach here
    return QString();
}
