/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef COILPOD_H
#define COILPOD_H

struct CoilPod
{
    int id_;
    int start_;
    int stop_;

    CoilPod(int id = 0, int start = 0, int stop = 0) : id_(id), start_(start), stop_(stop)
    {
    }

    bool operator==(const CoilPod &other)
    {
        return id_ == other.id_ &&
                start_ == other.start_ &&
                stop_ == other.stop_;
    }

    bool operator!=(const CoilPod &other)
    {
        return !operator==(other);
    }
};

#endif // COILPOD_H
