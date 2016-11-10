#!/usr/bin/perl
#
# Copyright (C) 2011 - Agile Genomics, LLC
# All rights reserved.
# Primary author: Luke Ulrich
#
# Description: Takes a class name is input and creates the corresponding
#              test directory for this class along with its .pro and .cpp
#              file prepopulated with the basic essentials.

$| = 1;

use strict;
use warnings;

my $usage = <<USAGE;
Usage: $0 <copyright preamble> <class name>

USAGE

my $g_CopyRightFile = shift or die $usage;
my $g_ClassName = shift or die $usage;

# Cleanse the name
$g_ClassName =~ s/^\s+|\s$//g;
if (length($g_ClassName) == 0)
{
    print STDERR qq(Invalid class name\n);
}

# Check that the directory does not exist
my $testName = 'Test' . $g_ClassName;
my $dirName = '_' . $testName;
if (-e $dirName)
{
    print STDERR qq(Directory: $dirName already exists\n);
    exit;
}

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
$year += 1900;   # localtime returns number of years since 1900 :)

# Slurp in the copyright file
$/ = undef;
open (IN, "< $g_CopyRightFile") or die qq([$0] Unable to open file '$g_CopyRightFile': $!\n);
my $CopyRight = <IN>;
close (IN);
$/ = "\n";
$CopyRight =~ s/\%YEAR\%/$year/;

print qq(This will create the directory, $dirName, under the current directory. Is this ok? (y/n): );
my $response = <STDIN>;
chomp($response);
exit if ($response ne 'y');

mkdir($dirName);

my $when = localtime;

# Now create the .pro file
my $proFile = $dirName . '/' . $testName . '.pro';
open (OUT, "> $proFile") or die qq([$0] Unable to create file '$proFile': $!\n);
print OUT <<PROTEXT;
# ----------------------------------------------------------
# Test project file created with create_test_scaffold.pl ($when)
#
# Copyright (C) $year  Agile Genomics, LLC
# All rights reserved.
# ----------------------------------------------------------

CONFIG += qtestlib debug
QT -= gui
TARGET = $testName
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += ../$g_ClassName.h
SOURCES += Test$g_ClassName.cpp \\
           ../$g_ClassName.cpp

DEFINES += TESTING
PROTEXT
close (OUT);


# Now create the cpp file
my $cppFile = $dirName . '/' . $testName . '.cpp';

open (OUT, "> $cppFile") or die qq([$0] Unable to create file '$cppFile': $!\n);
print OUT <<CPPTEXT;
$CopyRight
#include <QtTest/QtTest>

#include "../$g_ClassName.h"

class Test$g_ClassName : public QObject
{
    Q_OBJECT

private slots:
    void test1();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void Test${g_ClassName}::test1()
{

}


QTEST_APPLESS_MAIN(Test$g_ClassName)
#include "Test$g_ClassName.moc"
CPPTEXT
close (OUT);
