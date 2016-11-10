/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SERIALGENERATOR_H
#define SERIALGENERATOR_H

/**
  * Serial number generator
  *
  * Generates integral serial numbers similar to that used in relational database systems. The default
  * starting value is 1, although this may be changed by either passing a different value to the
  * constructor or calling setValue. The boolean called_ member indicates whether the current value
  * has already been fetched. See setValue(...) documentation for more details.
  */
class SerialGenerator
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    //! Construct a SerialGenerator that defaults to a currentValue of 1
    SerialGenerator(int currentValue = 1, bool isCalled = false);
    SerialGenerator(const SerialGenerator &other);              //!< Construct a copy of other

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    SerialGenerator &operator=(const SerialGenerator &other);   //!< Assign other to this SerialGenerator

    // ------------------------------------------------------------------------------------------------
    // Public methods
    int currentValue();                                         //!< Return the currentValue
    int nextValue();                                            //!< Advance the currentValue by 1 and return this value
    void setValue(int currentValue);                            //!< Set the currentValue
    void setValue(int currentValue, bool isCalled);             //!< Set the currentValue and indicate whether to advance the value on the next nextval request
    bool isCalled();                                            //!< Returns whether the sequence has been called
    void setCalled(bool isCalled);                              //!< Specify whether the current value has been fetched

protected:
    int currentValue_;      //!< Current serial value
    bool isCalled_;         //!< Indicates whether to advance the value on the next nextval request

private:
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param currentValue an integer value specifying the starting point for this sequence
  * @param isCalled a boolean flag denoting whether this sequence has already been called
  */
inline
SerialGenerator::SerialGenerator(int currentValue, bool isCalled) : currentValue_(currentValue), isCalled_(isCalled)
{
}

/**
  * @param other reference to SerialGenerator to be copied
  */
inline
SerialGenerator::SerialGenerator(const SerialGenerator &other) : currentValue_(other.currentValue_),
    isCalled_(other.isCalled_)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Assignment operators
/**
  * Assigns other to the current object
  * @param other reference to SerialGenerator to be assigned
  */
inline
SerialGenerator &SerialGenerator::operator=(const SerialGenerator &other)
{
    // Gracefully handle self-assignment
    if (this == &other)
        return *this;

    currentValue_ = other.currentValue_;
    isCalled_ = other.isCalled_;

    return *this;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Returns the current sequence value, which depending on the isCalled property, may or may not be the next sequence
  * value.
  * @return integer, current sequence value
  * @see isCalled(), nextval()
  */
inline
int SerialGenerator::currentValue()
{
    return currentValue_;
}

/**
  * @param currentValue integer value of the current sequence value
  * @see currentValue()
  */
inline
void SerialGenerator::setValue(int currentValue)
{
    currentValue_ = currentValue;
}

/**
  * The isCalled parameter indicates whether nextval() will return the currentValue or increment it.
  * @return boolean, whether the current value has been fetched
  * @see nextval()
  */
inline
bool SerialGenerator::isCalled()
{
    return isCalled_;
}

/**
  * Specifies whether nextValue() should return the currentValue [isCalled = false] or increment currentValue and
  * return this value [isCalled = true]
  * @param isCalled boolean
  * @see nextValue()
  */
inline
void SerialGenerator::setCalled(bool isCalled)
{
    isCalled_ = isCalled;
}

#endif // SERIALGENERATOR_H
