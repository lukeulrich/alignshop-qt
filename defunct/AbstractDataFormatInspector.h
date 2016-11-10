/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTDATAFORMATINSPECTOR_H
#define ABSTRACTDATAFORMATINSPECTOR_H

#include "global.h"

/**
  * AbstractDataFormatInspector defines the abstract interface for inspecting a text buffer and returning a
  * known specific DataFormatType or eUnknownType if it does not adhere to the concrete inspector instance
  * inspection process.
  *
  * Concrete classes must derive from this class and define the inpect function.
  *
  * It is important to distinguish between a simple "inspection" and all out validation. Here, inspection
  * refers to performing a limited analysis of a chunk of textual data and based on this determining a
  * specific data format. Validation would involve a more comprehensive and thorough analysis.
  *
  * Utilizes the strategy pattern.
  */
class AbstractDataFormatInspector
{
public:
    // ------------------------------------------------------------------------------------------------
    // Destructor
    virtual ~AbstractDataFormatInspector();                             //!< Trivial destructor

    // ------------------------------------------------------------------------------------------------
    // Pure-virtual functions
//    virtual AbstractDataFormatInspector *clone() const = 0;             //!< Pure virtual clone idiom
    virtual DataFormatType inspect(const QString &buffer) const = 0;    //!< Pure virtual function that inspects buffer for a particular and returns the respective File::Format
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Destructor
/**
  * Trivial inline definition of destructor
  */
inline
AbstractDataFormatInspector::~AbstractDataFormatInspector()
{
}


#endif // ABSTRACTDATAFORMATINSPECTOR_H
