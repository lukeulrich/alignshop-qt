#!/usr/bin/perl
#
# Finds all directories anywhere beneath the current directory that begin
# with Test and contain a Makefile and executes clean on them

$| = 1;

use strict;
use warnings;

use Cwd;
use File::Find;

find(\&make_it_clean, getcwd());

sub make_it_clean
{
    return if ($File::Find::dir !~ /(_?Test[^\/]+)$/ ||
	       $_ ne 'Makefile');

    my $exe = $1;

    print "--> Cleaning: $File::Find::dir -- $exe\n";
    system('make clean');

    unlink($exe);
    $exe =~ s/^_//;
    unlink($exe);
}
