#ifndef VERSION_H
#define VERSION_H

namespace constants
{
    namespace version
    {
        static const int kMajorVersion = 0;
        static const int kMinorVersion = 5;
        static const char *kBuildType = "a";
        static const int kBuildVersion = 4;
        static const char *kRevision = "710M";
    }

    inline const char *versionString()
    {
        return "0.5-a4";
    }
}

#endif // VERSION_H
