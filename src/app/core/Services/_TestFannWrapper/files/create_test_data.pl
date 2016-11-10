#!/usr/bin/perl
#
# Takes as input a PSSM and dumps a C++ formatted list of inputs and expected
# outputs for a single stage for the eigth character:
#
#        *
# 123456789012345
# ABCDEFGHIJKLMNO
#
# The rationale is that the NN takes a window of 15 PSSM rows and returns
# the result for the middle character. By working with the 8th character, we
# avoid having to deal with creating zeros for the portion that would extend
# over the terminus.
#
# If an error is encountered while running this script because of missing FANN
# shared libraries, simply use the shared library compiled in with AlignShop:
#
# LD_LIBRARY_PATH={AlignShop root}/fann-2.1.0/src/.libs ./create_test_data.pl
#
# Note: the PSSM file should be formatted as follows:
# 
#        A     C     D ...
# 1 M -342   -23   993 ...


$| = 1;

use strict;
use warnings;
use AI::FANN ':all';

my $usage = <<USAGE;
Usage: $0 <neural net file> <pssm>

USAGE

my $g_NetFile = shift or die $usage;
my $g_PssmFile = shift or die $usage;

my $g_BlastAlphSize = 20;
my $g_Stride = $g_BlastAlphSize + 1;
my $g_WindowSize = 15;
my $g_Inputs = ($g_BlastAlphSize + 1) * $g_WindowSize;

my $g_PositScaleFactor = 200;

my $fann = AI::FANN->new_from_file($g_NetFile);

# Load the PSSM data
my @pssm = ();
open (IN, "< $g_PssmFile") or die qq([$0] Unable to open file '$g_PssmFile': $!\n);
my $header = <IN>;
$header =~ s/^\s+|\s+$//g;
my @letters = split(/\s+/, $header);
die qq(Invalid number of letters\n) if (@letters != 20);
while (my $line = <IN>)
{
    chomp($line);
    last if ($line =~ /^\/\//);
    next if ($line !~ /^\s*\d+ \w *(.*)/);

    my $scores = $1;
    my @scores = split(/\s+/, $scores);
    if (@scores != 20)
    {
	die qq(Unexpected number of scores: $line\n);
    }

    # Scale all the scores by the posit scale factor
    @scores = map { $_ / $g_PositScaleFactor } @scores;
    
    # Now scale between 0 and 1 using the logistic function
    @scores = map { 1.0 / (1. + exp(-.5 * $_)) } @scores;
    
    push @pssm, \@scores;

    last if (@pssm == 15);
}
close (IN);

die qq([$0] Too short pssm\n) if (@pssm < 15);

# ------------------------------------------------------------
# Part B: Format the inputs
my $cpp_array = "double inputs[315] = {\n";
my @inputs = ();
for (my $i=0; $i< 15; ++$i)
{
    my $line = "\t";
    for (my $j=0; $j< $g_BlastAlphSize; ++$j)
    {
	my $val = sprintf("%.4f", $pssm[$i]->[$j]); 
#	$line .= sprintf("%.4f, ", $pssm[$i]->[$j]);
	$line .= "$val, ";
	push @inputs, $val;
    }
    $line .= "0.0000,\n";
    push @inputs, 0.;

    $cpp_array .= $line;
}

chop($cpp_array);
chop($cpp_array);
$cpp_array .= "\n};\n";


# ------------------------------------------------------------
# Part C: run the NN
my $output = $fann->run(\@inputs);

my $cpp_out_array = "double outputs[3] = {";
$cpp_out_array .= sprintf("%.4f, ", $output->[0]);
$cpp_out_array .= sprintf("%.4f, ", $output->[1]);
$cpp_out_array .= sprintf("%.4f};\n", $output->[2]);

print $cpp_array;
print $cpp_out_array;
