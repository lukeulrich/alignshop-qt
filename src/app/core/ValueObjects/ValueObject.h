/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef VALUEOBJECT_H
#define VALUEOBJECT_H

/**
  * IValueObject defines the methods common to all value objects.
  *
  * The id is merely used for input/output purposes - it is not intended to be used to test for equivalence.
  */
class ValueObject
{
public:
    static const int kNewId = 0;

    // ------------------------------------------------------------------------------------------------
    // Constructor
    explicit ValueObject(const int id = kNewId)
        : id_(id)
    {
    }


    // ------------------------------------------------------------------------------------------------
    // Public methods
    bool isNew() const
    {
        return id_ == kNewId;
    }

    void clearId()
    {
        id_ = kNewId;
    }


    // ------------------------------------------------------------------------------------------------
    // Public members
    int id_;
};

#endif // VALUEOBJECT_H
