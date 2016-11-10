/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef TAGGENERATOR_H
#define TAGGENERATOR_H

/**
  * TagGenerator simply generates monotonically increasing numbers starting from 1 and is
  * effectively singleton in behavior.
  */
class TagGenerator
{
public:
    static int nextValue()
    {
        return ++currentValue_;
    }

private:
    // Disable all forms of object instantiation
    TagGenerator();
    TagGenerator(const TagGenerator &);
    TagGenerator &operator=(const TagGenerator &);

    static int currentValue_;
};

#endif // TAGGENERATOR_H
