#!/usr/bin/perl
#
# Copyright (c) 2010 - Agile Genomics, LLC
#
# Program: sql2xml.pl
# Author: Luke Ulrich
# Date: 25 Feb 2010
# Synopsis: Read in SQL DDL file and outputs a corresponding XML data structure.
#           Currently targets only SQLite compatible DDL.
#
# Intended to only function as a utility script for rapidly producing a
# XML file and not so much a robust, reusable script.
#
# [8 July 2010]
# o Added support for dumping unique keys in XML
# o Added support for single-word field defaults (e.g. default current_timestamp)

$| = 1;

use strict;
use warnings;

use Data::Dumper;
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
	    if ($line =~ s/^\s+(\S+) +(integer|int|text)\b//i)
	    {
		my $field = {name => $1, type => $2};
		if ($line =~ s/-- (.*)//)
		{
		    $field->{comment} = $1;
		}
		if ($line =~ /\bnot null\b/i)
		{
		    $field->{not_null} = 1;
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
		if ($line =~ /\bunique\b/i)
		{
		    push @{ $table->{uniques} }, [$field->{name}];
		}
		if ($line =~ /\bdefault +(\S+)\b/)
		{
		    $field->{default} = $1;
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
		    fields => [ split(/,/, $fields) ],
		    ref_table => $ref_table,
		    ref_fields => [ split(/,/, $ref_fields) ]
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
	    # Unique
	    elsif ($line =~ /^\s*unique *\(([^)]*)\)/i)
	    {
		my $fields = $1;
		$fields =~ s/\s+//g;

		push @{ $table->{uniques} }, [ split(/,/, $fields) ];
	    }
	    # Checks
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

# Now dump out to XML :)

print <<XML;
<?xml version="1.0" encoding="UTF-8"?>

<db_schema xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="db_schema.xsd">

<version>$js->{version}</version>
XML

my $indent = 0;
foreach my $table (@{ $js->{tables} })
{
    print qq(<table name="$table->{name}");
    if ($table->{comment})
    {
	print qq( comment="$table->{comment}");
    }
    print ">\n";

    $indent += 2;
    
    # Walk through fields
    foreach my $field (@{$table->{fields}})
    {
	print qq(  <field name="$field->{name}" type="$field->{type}");
	if ($field->{not_null})
	{
	    print qq( not_null="true");
	}
	if ($field->{autoincrement})
	{
	    print qq( autoincrement="true");
	}
	if ($field->{default})
	{
	    print qq( default="$field->{default}");
	}

	if ($field->{comment})
	{
	    print qq( comment="$field->{comment}");
	}
	print " />\n";
    }
    
    # Primary key
    if ($table->{primary_key})
    {
	print "  <primaryKey>", join(', ', @{ $table->{primary_key} }), "</primaryKey>\n";
    }

    # Foreign keys
    if ($table->{foreign_keys})
    {
	foreach my $fk (@{ $table->{foreign_keys} })
	{
	    print "  <foreignKey";
	    if ($fk->{update})
	    {
		print qq( onUpdate="$fk->{update}");
	    }
	    if ($fk->{delete})
	    {
		print qq( onDelete="$fk->{delete}");
	    }
	    print ">\n";

	    print "    <srcFields>", join(', ', @{ $fk->{fields} }), "</srcFields>\n";;
	    print "    <refTable>$fk->{ref_table}</refTable>\n";
	    print "    <refFields>", join(', ', @{ $fk->{ref_fields} }), "</refFields>\n";;
	    print "  </foreignKey>\n";
	}
    }

    # Unique constraints
    if ($table->{uniques})
    {
	foreach my $unique (@{ $table->{uniques} })
	{
	    print "  <unique>", join(', ', @$unique), "</unique>\n";
	}
    }

    # Checks
    if ($table->{checks})
    {
	foreach my $check (@{ $table->{checks} })
	{
	    print "  <check>$check</check>\n";
	}
    }

    # Indices
    if ($table->{indices})
    {
	foreach my $index (@{ $table->{indices} })
	{
	    print qq(  <index name="$index->{name}">), join(', ', @{ $index->{fields} }), "</index>\n";
	}
    }

    # TODO: queries

    print "</table>\n";
}

# Pragmas
if ($js->{pragmas})
{
    foreach my $pragma (@{ $js->{pragmas} })
    {
	print qq(<pragma>$pragma</pragma>\n);
    }
}

print "</db_schema>\n";

