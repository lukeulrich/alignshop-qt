#!/usr/bin/perl
#
# Copyright (C) 2011 Agile Genomics, LLC
# All rights reserved.
# Author: Luke Ulrich
#
# Description: Recursively loops through all Test* directories and runs the
#              corresponding unit tests. The default directory is the current
#              directory if none is specified.

$| = 1;

use strict;
use warnings;
use Cwd;
use File::Find;
use Getopt::Long;

my $usage = <<USAGE;
Usage: $0 [options] [root directory]

  Available options:
  ------------------
    -h, --help                    : Show this message
    -j <int>                      : Number of processors to use during
                                    compiling (default 1)

USAGE

# ------------------------------------------------------------
# Globals
my $nModulesPassed = 0;

# Failed data hash contains all the information about each failed test:
# [ { name: name of test module
#     qmake_error: error running qmake; null otherwise
#     make_error: error running make; null otherwise
#     run_error: error when running excutable; null otherwise
#     totals: string describing number of methods passed, failed, and skipped
#     failures: [ list of method names that have failed ]
#   },
# ]
my @failedModules = ();

# ------------------------------------------------------------
# Options
my $g_Help;
my $g_Processors;
GetOptions("h|help", \$g_Help,
	   "j=i", \$g_Processors);
die $usage if ($g_Help);

$g_Processors = 1 if (!$g_Processors);
die qq(Number of processors (-j) must be greater than zero\n) if ($g_Processors && $g_Processors < 0);

# ------------------------------------------------------------
# Usage and arguments
my $g_Directory = shift;
$g_Directory = '.' if (!$g_Directory);

die qq($g_Directory is not a valid directory!) if (!-d $g_Directory);

# ------------------------------------------------------------
# Recurse through every directory and for all directories that begin with
# Test and contain a .pro, build and run the unit test
find(\&dirFileController, $g_Directory);

print "\n";
print '-'x60, "\n";
print '-'x60, "\n";
print '>> ', scalar(@failedModules), " failed module(s)\n";
foreach my $failedModule (@failedModules)
{
    if ($failedModule->{qmake_error})
    {
	print "[$failedModule->{name}] QMAKE: $failedModule->{qmake_error}\n";
    }
    elsif ($failedModule->{make_error})
    {
	print "[$failedModule->{name}] MAKE: $failedModule->{make_error}\n";
    }
    elsif ($failedModule->{run_error})
    {
	print "[$failedModule->{name}] RUN: $failedModule->{run_error}\n";
    }
    else
    {
	print "[$failedModule->{name}] $failedModule->{totals}\n";
	foreach my $failedMethod (@{ $failedModule->{failures} })
	{
	    print "\t$failedMethod\n";
	}
    }
}

print "\n";
print '-'x60, "\n";
print '-'x60, "\n";
print '>> ', $nModulesPassed, " module(s) completely passed\n";

# ------------------------------------------------------------
# ------------------------------------------------------------
# Subroutines
sub dirFileController
{
    return if (!-d $_ || $_ !~ /^Test/ || $_ =~ /^Test.*-build-*/);

    # Check for .pro file and warn if it is missing
    if (!-e "$_/$_.pro")
    {
	warn qq([Warning] Skipping directory: missing .pro file - $_\n);
	return;
    }

    # Check for Makefile and make one if not there
    my $qmake_error;
    chdir($_);
    if (!-e 'Makefile')
    {
	my $retval = system('qmake > /dev/null 2>&1');
	if ($? == -1)
	{
	    $qmake_error = qq(Failed to execute qmake! Please make sure that qmake is in the PATH environment variable);
	}
	elsif ($? & 127)
	{
	    $qmake_error = qq(qmake process died with signal ) . ($? & 127);
	}
	elsif ($retval != 0)
	{
	    $qmake_error = qq(qmake encountered an error);
	}
	elsif (!-e 'Makefile')
	{
	    $qmake_error = qq(qmake succesful; however, Makefile was not created);
	}

	if ($qmake_error)
	{
	    push @failedModules, { name => $_,
				   qmake_error => $qmake_error };
	    print "[FAIL] $_: $failedModules[-1]->{qmake_error}\n";
	    chdir('..');
	    return;	
	}
    }
    
    &runTest($_);
    chdir('..');
}

sub runTest
{
    my $testDirectory = shift;

    chdir($testDirectory);
    my $testExe = $testDirectory;

    my $make_error;
 
    # Make the unit test
    my $retval = system("make -j$g_Processors > /dev/null 2>&1");
    if ($? == -1)
    {
	$make_error = qq(Failed to execute make! Please make sure that make is in the PATH environment variable);
    }
    elsif ($? & 127)
    {
	$make_error = qq(make process died with signal ) . ($? & 127);
    }
    elsif ($retval != 0)
    {
	$make_error = qq(make encountered an error);
    }

    if ($make_error)
    {
	push @failedModules, { name => $testExe,
			       make_error => $make_error };
	print "[FAIL] $testExe: $failedModules[-1]->{make_error}\n";
	return;
    }

    # Run the unit test
    my $run_error;
    if (!-e $testExe)
    {
	$run_error = qq(Test executable file '$testExe' does not exist);
    }
    elsif (-s $testExe == 0)
    {
	$run_error = qq(Test executable exists, but has a zero size);
    }

    if ($run_error)
    {
	push @failedModules, { name => $testExe,
			       run_error => $run_error };
	print "[FAIL] $testExe: $failedModules[-1]->{run_error}\n";
	return;
    }


    my @failures = ();
    my $totals;
	
    # Attempt to run the test and capture its output
    if (open(IN, "./$testExe |"))
    {
	# Open command worked successfully, start reading its output
	while (<IN>)
	{
	    if (/^FAIL.*?:\s+${testExe}::(.*)/)
	    {
		push @failures, $1;
	    }
	    elsif (/^Totals: (.*)/)
	    {
		$totals = $1;
	    }
	}
	close (IN);
	
	# Check that the open command did not fail or terminate
	if ($? & 127)
	{
	    $run_error = qq($testExe process died with signals ) . ($? & 127);
	}
    }
    else
    {
	$run_error = $!;
    }

    if ($run_error)
    {
	push @failedModules, { name => $testExe,
			       run_error => $run_error };
	print "[FAIL] $testExe: $failedModules[-1]->{run_error}\n";
	return;
    }

    if (@failures == 0)
    {
	print "[pass] $testExe\n";
	++$nModulesPassed;
    }
    else
    {
	push @failedModules, { name => $testExe,
			       totals => $totals,
			       failures => \@failures };
	print "[FAIL] $testExe: $totals\n";
    }
}
