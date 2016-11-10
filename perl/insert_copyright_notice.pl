#!/usr/bin/perl
#
# Program: isnert_copyright_notice.pl
# Author: Luke Ulrich
# Date: 29 Nov 2010
# Description: For each file specified, inserts the contents of the supplied
#              copyright file (or other desired file) before all other data
#              in the other files specified on the command line.

$| = 1;

use strict;
use warnings;

my $usage = <<USAGE;
Usage: $0 <copyright or notice file> <file to insert into> [...]

USAGE

my $g_NoticeFile = shift or die $usage;

# Make sure there is at least one other file
die $usage if (!@ARGV);

# Read in the copyright file contents
undef $/;
my $copyright_data;
open (IN, "< $g_NoticeFile") or die qq([$0] Unable to open file '$g_NoticeFile': $!\n);
$copyright_data = <IN>;
close(IN);

foreach my $file (@ARGV)
{
    next if (-d $file);
    if (!-e $file)
    {
	warn(qq('$file': does not exist\n));
	next;
    }
    if (!-f $file)
    {
	warn(qq('$file': not a plain text file\n));
	next;
    }
    if (!-r $file || !-w $file)
    {
	warn(qq('$file': missing read/write permissions\n));
	next;
    }

    open (IN, "< $file") or die qq([$0] Unable to open file $file\n");
    my $contents = <IN>;
    close (IN);

    open (OUT, "> $file") or die qq([$0] Unable to open file for writing: '$file'\n);
    print OUT $copyright_data;
    print OUT $contents;
    close (OUT);

    print qq(Processed $file\n);
}
