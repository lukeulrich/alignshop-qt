/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H

#include <QtCore/QString>

//!< Returns true if fileName is not empty, first character is not a period or hyphen or space, does not end with a space, and solely contains the allowed characters: 0-9, A-z, '.', '_', '-', and ' '; false otherwise
bool portableFileName(const QString &fileName);

#endif // UTILITY_H
