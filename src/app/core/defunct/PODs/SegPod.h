/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef SEGPOD_H
#define SEGPOD_H

struct SegPod
{
    int id_;
    int start_;
    int stop_;

    SegPod(int id = 0, int start = 0, int stop = 0) : id_(id), start_(start), stop_(stop)
    {
    }

    bool operator==(const SegPod &other)
    {
        return id_ == other.id_ &&
                start_ == other.start_ &&
                stop_ == other.stop_;
    }

    bool operator!=(const SegPod &other)
    {
        return !operator==(other);
    }
};

#endif // SEGPOD_H
