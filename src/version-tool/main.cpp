/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <QtDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();
    QString executable = arguments.first();
    QFileInfo fileInfo(executable);
    QString shortName = fileInfo.baseName();

    QTextStream cerr(stderr);
    if (arguments.size() != 6)
    {
        cerr << "Usage: " << shortName << " major minor build-type build-number dest-directory\n";
        return 1;
    }

    // Attempt to read in the arguments
    bool ok = false;
    int majorNumber = arguments.at(1).toInt(&ok);
    if (!ok)
    {
        cerr << "Error with major number (expected integer)\n";
        return 1;
    }

    int minorNumber = arguments.at(2).toInt(&ok);
    if (!ok)
    {
        cerr << "Error with minor number (expected integer)\n";
        return 1;
    }

    QString buildType = arguments.at(3);
    if (buildType.isEmpty())
    {
        cerr << "Build type must not be empty\n";
        return 1;
    }

    int buildNumber = arguments.at(4).toInt(&ok);
    if (!ok)
    {
        cerr << "Error with build number (expected integer)\n";
        return 1;
    }

    QDir directory(arguments.at(5));
    if (directory.path().isEmpty())
    {
        cerr << "Invalid directory\n";
        return 1;
    }
    else if (!directory.exists())
    {
        cerr << "Build directory: " << directory.path() << " does not exist\n";
        return 1;
    }

    // Now attempt to get the revision number
    QProcess process;
    process.start("svnversion", QStringList() << "-n" << directory.path());
    if (!process.waitForStarted() || !process.waitForReadyRead())
    {
        cerr << "Error running svnversion\n";
        cerr << process.readAllStandardError();
        return 1;
    }

    QByteArray revision = process.readAllStandardOutput();
    process.waitForFinished();
    if (revision.isEmpty())
    {
        cerr << "No output received from svnversion\n";
        return 1;
    }
    if (revision == "Unversioned directory")
    {
        cerr << "Directory is not under version control\n";
        return 1;
    }

    // All systems go, now output the version data to stdout
    QTextStream cout(stdout);
    cout << "#ifndef VERSION_H\n";
    cout << "#define VERSION_H\n\n";
    cout << "namespace constants\n";
    cout << "{\n";
    cout << "    namespace version\n";
    cout << "    {\n";
    cout << "        static const int kMajorVersion = " << majorNumber << ";\n";
    cout << "        static const int kMinorVersion = " << minorNumber << ";\n";
    cout << "        static const char *kBuildType = \"" << buildType.toAscii() << "\";\n";
    cout << "        static const int kBuildVersion = " << buildNumber << ";\n";
    cout << "        static const char *kRevision = \"" << revision << "\";\n";
    cout << "    }\n";
    cout << "\n";
    cout << "    inline const char *versionString()\n";
    cout << "    {\n";
    cout << "        return \"" << majorNumber << '.' << minorNumber << '-' << buildType << buildNumber << "\";\n";
    cout << "    }\n";
    cout << "}\n";
    cout << "\n";
    cout << "#endif // VERSION_H\n";

    return 0;
}

