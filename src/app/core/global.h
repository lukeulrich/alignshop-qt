#ifndef GLOBAL_H
#define GLOBAL_H

// MSVC 2010 defines the nullptr keyword. Thus there is no need to define in this case.
#if ! defined (_MSC_VER) || _MSC_VER < 1600
static const int nullptr = 0;
#endif

#endif
