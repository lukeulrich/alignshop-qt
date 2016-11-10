#!/usr/bin/perl
#
# Copyright (c) 2010 - Agile Genomics, LLC
#
# Program: sql2js.pl
# Author: Luke Ulrich
# Date: 15 Feb 2010
# Synopsis: Read in SQL DDL file and output a custom JSON structure that
#           represents the SQL structure. Currently targets only SQLite
#           compatible DDL.
#
# Intended to only function as a utility script for rapidly producing a
# JSON ddl file and not so much a robust, reusable script.

$| = 1;

use strict;
use warnings;

use JSON;

my $usage = <<USAGE;
Usage: $0 <sql file>

USAGE

my $g_SqlFile = shift or die $usage;

my $js = {
    version => 0.1
};

my %name2row = ();
my $indices = [];

# Slurp in the whole file
my $g_Sql;
open (IN, "< $g_SqlFile") or die qq([$0] Unable to open file '$g_SqlFile': $!\n);
$g_Sql .= $_ while(<IN>);
close (IN);

# Remove all C-style comments
$g_Sql =~ s/\/\*(?:.|\s)*?\*\///g;

while ($g_Sql =~ /(.*)/g)
{
    my $line = $1;

    if ($line =~ /^create table (\S+)/i)
    {
	my $table = {name => $1};

	# Read through table definition until we find the closing
	# );
	while ($g_Sql =~ /(.*)/g)
	{
	    $line = $1;

	    # Take the first comment that has no leading whitespace
	    # as the table comment
	    if ($line =~ /^-- (.*)?/ &&
		!$table->{comment})
	    {
		$table->{comment} = $1;
	    }
	    elsif ($line =~ /^\);/)  # This indicates end of create table
	    {
		last;
	    }

	    # Read in the columns
	    if ($line =~ /^\s+(\S+) +(integer|int|text)\b/i)
	    {
		my $field = {name => $1, type => $2};
		if ($line =~ /\bnot null\b/i)
		{
		    $field->{nullable} = 0;
		}
		if ($line =~ /\bprimary key\b/i &&
		    !$table->{primary_key})
		{
		    $table->{primary_key} = [ $field->{name} ];
		}
		if ($line =~ /\bautoincrement\b/i)
		{
		    $field->{autoincrement} = 1;
		}
		if ($line =~ /-- (.*)/)
		{
		    $field->{comment} = $1;
		}

		push @{ $table->{fields} }, $field;
	    }
	    # Primary key
	    elsif ($line =~ /^\s*primary key *\(([^)]*)\)/i &&
		   !$table->{primary_key})
	    {
		my $fields = $1;
		$fields =~ s/\s+//g;
		$table->{primary_key} = [ split(/,/, $fields) ];
	    }
	    # Foreign key
	    elsif ($line =~ /^\s*foreign key *\(([^)]*)\) +references +(\w+) *\(([^)]*)\)/i)
	    {
		my $fields = $1;
		my $ref_table = $2;
		my $ref_fields = $3;

		$fields =~ s/\s+//g;
		$ref_fields =~ s/\s+//g;

		my $fk = {
		    fields => split(/,/, $fields),
		    ref_table => $ref_table,
		    ref_fields => split(/,/, $ref_fields)
		};
		if ($line =~ /on update (cascade|restrict|set null)/i)
		{
		    $fk->{update} = $1;
		}
		else
		{
		    $fk->{update} = 'cascade';
		}

		if ($line =~ /on delete (cascade|restrict|set null)/i)
		{
		    $fk->{delete} = $1;
		}
		else
		{
		    $fk->{delete} = 'cascade';
		}

		push @{ $table->{foreign_keys} }, $fk;
	    }
	    elsif ($line =~ /^\s*check *\((.*)\)/i)
	    {
		push @{ $table->{checks} }, $1;
	    }
	} # while ($g_Sql =~ /(.*)/g)

	push @{ $js->{tables} }, $table;

	$name2row{ $table->{name} } = scalar(@{ $js->{tables} }) - 1;
    } # if (/create table.../)
    elsif ($line =~ /\s*create index (if not exists)? (\S+) on ([^)]*) *\(([^)]*)/i)
    {
	my $index = {
	    name => $2,
	    table => $3,
	    fields => $4
	};
	$index->{ifexists} = 1 if ($1);

	$index->{fields} =~ s/\s+//g;
	$index->{fields} = [ split(/,/, $index->{fields}) ];

	push @$indices, $index;
    }
    elsif ($line =~ /\s*(pragma.*\S *;)/i)
    {
	push @{ $js->{pragmas} }, $1;
    }
}

foreach my $index (@$indices)
{
    my $row = $js->{tables}->[ $name2row{ $index->{table} } ];

    delete $index->{table};
    push @{ $row->{indices} }, $index;
}

print to_json($js, {utf8 => 1, pretty => 1, space_before => 0, canonical => 1});
