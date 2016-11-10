#ifndef MACROS_H
#define MACROS_H

#define ASSERT_X(x,y) Q_ASSERT_X(x, Q_FUNC_INFO, y)
#define ASSERT(x) Q_ASSERT_X(x, Q_FUNC_INFO, "")

#ifdef Q_OS_WIN
  #define fdopen _fdopen
  // Note: #include <io.h> for _pipe
  //       #include <fcntl.h> for O_TEXT definition
  #define pipe(x) _pipe(x, 256, O_TEXT)
  #define sscanf sscanf_s
#endif

#endif // MACROS_H
