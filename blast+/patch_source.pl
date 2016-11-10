#!/usr/bin/perl
#
# Copies the source tree of directory1 to the source tree of directory2. More specifically,
# iterates through the directory structure of directory1 and copies every file prefixed with
# 'ag-' to directory2 if it has a similar tree structure and file name without 'ag-'.
#
# Skips all directories named defunct

$| = 1;

use strict;
use warnings;

use File::Copy;
use File::Find;
use File::Spec;
use Cwd 'abs_path';

my $usage = <<USAGE;
Usage: $0 <source directory> <dest directory>

USAGE

my $g_Dir1 = shift or die $usage;
my $g_Dir2 = shift or die $usage;

die qq(Invalid directory: $g_Dir1) if (!-e $g_Dir1 || !-d $g_Dir1);
die qq(Invalid directory: $g_Dir2) if (!-e $g_Dir2 || !-d $g_Dir2);

$g_Dir1 = abs_path($g_Dir1);
$g_Dir2 = abs_path($g_Dir2);

die qq(Source and destination directories should be not the same\n) if ($g_Dir1 eq $g_Dir2);

$g_Dir1 = File::Spec->abs2rel($g_Dir1);

$g_Dir2 .= '/' if (substr($g_Dir2, -1) ne '/');

find(\&copyFiles, $g_Dir1);

# Special handling for AgTickData
copy($g_Dir1 . '/include/algo/blast/core/AgTickData.h', $g_Dir2 . 'include/algo/blast/core/');
print "AgTickData [hard coded]\n";

sub copyFiles
{
    return if ($_ !~ /^ag-(.+\.(h|hpp|cpp|asn|c))$/);

    my $targetFileName = $1;
    my $targetDir = $g_Dir2 . $File::Find::dir . '/';
    my $targetFile = $targetDir . $targetFileName;

    return if ($targetDir =~ /\/defunct\/?$/);

    die qq(Target dir '$targetDir' does not exist\n) if (!-d $targetDir);
    die qq(Target file '$targetFile' does not exist\n) if (!-e $targetFile);

    die $! if (!copy($_, $targetFile));
    print "$_ --> $targetFile\n";
}
