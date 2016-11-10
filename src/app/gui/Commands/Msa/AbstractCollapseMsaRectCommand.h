/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTCOLLAPSEMSARECTCOMMAND_H
#define ABSTRACTCOLLAPSEMSARECTCOMMAND_H

#include <QtCore/QVector>

#include <QtGui/QUndoCommand>

#include "../../../core/ObservableMsa.h"
#include "../../../core/PODs/SubseqChangePod.h"
#include "../../../core/global.h"
#include "../../../core/util/PosiRect.h"
#include "../CommandIds.h"

/**
  * AbstractCollapseMsaRectCommand is the base class for collapse commands that provides support for merging adjacent
  * collapse commands.
  *
  * Merging adjacent collapse commands is slightly complicated because the subseq change pods returned by collapse{Left,
  * Right} are minimal in nature and thus do not always span the entire msa rectangle width. This becomes a problem when
  * the user immediately follows one collapse with another collapse in the opposite direction. It is not clear how to
  * tweak the change pod information when undoing the merged command.
  *
  * The simple solution is to observe when merging commands if the collpase is in the opposite direction (using subIds).
  * If this is true, then when undo is called, simply restore the original collapse before calling undo with the
  * original change pod vector.
  */
class AbstractCollapseMsaRectCommand : public QUndoCommand
{
public:
    // -------------------------------------------------------------------------------------------------
    // Constructor
    AbstractCollapseMsaRectCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand = nullptr);

    // -------------------------------------------------------------------------------------------------
    // Public methods
    virtual int id() const;                                 //!< Returns the command id
    virtual bool mergeWith(const QUndoCommand *other);      //!< Returns true if other was successfully merged with this command; false otherwise
    virtual int subId() const = 0;                          //!< Returns the sub command id
    virtual void undo();                                    //!< Undoes this command

protected:
    ObservableMsa *msa_;
    PosiRect msaRect_;
    SubseqChangePodVector changePodVector_;

private:
    //! Indicates if this command has been merged with a compatible collapse in the opposite direction
    bool reverseCollapse_;
};


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param msa [ObservableMsa *]
  * @param msaRect [const PosiRect &]
  * @param parentCommand [QUndoCommand *]
  */
inline
AbstractCollapseMsaRectCommand::AbstractCollapseMsaRectCommand(ObservableMsa *msa, const PosiRect &msaRect, QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      msa_(msa),
      msaRect_(msaRect),
      reverseCollapse_(false)
{
    ASSERT(msa_ != nullptr);
}

/**
  * @param other [QUndoCommand *]
  */
inline
bool AbstractCollapseMsaRectCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const AbstractCollapseMsaRectCommand *otherCommand = static_cast<const AbstractCollapseMsaRectCommand *>(other);
    if (msa_ != otherCommand->msa_ || msaRect_ != otherCommand->msaRect_)
        return false;

    setText(otherCommand->text());
    reverseCollapse_ = otherCommand->subId() != subId();

    return true;
}

/**
  * @returns int
  */
inline
int AbstractCollapseMsaRectCommand::id() const
{
    return Ag::eCollapseMsaRectCommandId;
}

/**
  */
inline
void AbstractCollapseMsaRectCommand::undo()
{
    if (reverseCollapse_)
    {
        if (subId() == Ag::eCollapseMsaRectLeftCommandId)
            msa_->collapseLeft(msaRect_);
        else // if (subId() == Ag::eCollapseMsaRectRightCommandId)
            msa_->collapseRight(msaRect_);
    }

    msa_->undo(changePodVector_);
}


#endif // ABSTRACTCOLLAPSEMSARECTCOMMAND_H
