/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef INVALIDMPTTNODEERROR_H
#define INVALIDMPTTNODEERROR_H

#include "CodedError.h"

#include "../MpttNode.h"

class InvalidMpttNodeError : public CodedError
{
public:
    enum ErrorCode {
        eBadLeftCode = 1,           //!< Invalid left value
        eBadRightCode,              //!< Invalid right value
        eDuplicateLeftCode,         //!< Duplicate left value
        eDuplicateRightCode,        //!< Duplicate right value
        eUnexpectedLeftCode,        //!< Unexpected left value
        eUnexpectedRightCode,       //!< Unexpected right value
        eEvenOddCode,               //!< Left and right values cannot both be even or both be odd
        eNullTreeNodeCode,          //!< Tree node pointer has a null value
        eParentTreeNodeCode,        //!< Parent tree node pointer is not null
        eHasChildrenCode,           //!< TreeNode has one or more children
        eDuplicateTreeNodeCode,     //!< Duplicate TreeNode pointer

        eMissingValueCode           //!< Missing at least one left or right value
    };

    // ------------------------------------------------------------------------------------------------
    // Constructor
    InvalidMpttNodeError(int errorNumber, const QString &message, MpttNode *mpttNode = 0);

    // ------------------------------------------------------------------------------------------------
    // Public methods
    const MpttNode *mpttNode() const;

private:
    MpttNode *mpttNode_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
inline
InvalidMpttNodeError::InvalidMpttNodeError(int errorNumber, const QString &message, MpttNode *mpttNode)
    : CodedError(errorNumber, message), mpttNode_(mpttNode)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
inline
const MpttNode *InvalidMpttNodeError::mpttNode() const
{
    return mpttNode_;
}

#endif // INVALIDMPTTNODEERROR_H