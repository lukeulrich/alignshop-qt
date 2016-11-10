#!/usr/bin/perl
#
# Program: cpp_skeleton.pl
# Author: Luke Ulrich
# Date: 19 October 2010
# Description: Given a class header file, create a cpp skeleton with doxygen comments and section headers
#
# 16 Feb 2011
# -- Pure virtual methods are no longer output
#
# 17 Jun 2011
# -- Extension support added for use with Qt Creator

$| = 1;

use strict;
use warnings;
use Getopt::Long;

use Data::Dumper;

my $usage = <<USAGE;
Usage: $0 [options] <header file>

  Available options:
  -----------------
    -o, --output=<filename>    : Name of file to cpp skeleton to
    -j, --javadoc              : Output javadoc-style comment headers for
                                 params and return values
    -e, --extension=<string>   : Use the following extension

** Limitations:
   o Only works on first defined class in header
   o Most statements should not be spread across multiple lines
USAGE

my $g_OutputFile;
my $g_JavaDoc;
my $g_Extension;
GetOptions("o|output=s", \$g_OutputFile,
	   "j|javadoc", \$g_JavaDoc,
	   "e|extension=s", \$g_Extension);

my $g_HeaderFile = shift or die $usage;


# -----------------------------------------------------------------------------
# Parse the header file into its components
my $className;
my @parentClasses;
my @constructors;
my $destructor;
my $copyright_notice = '';
my @member_names = ();     # Simply ordered list of names as they occur in the header file; use as key in %members
my %members = ();

my %methods = ();

my $state;   # Contains public, public slots, private

open (IN, "< $g_HeaderFile") or die qq([$0] Unable to open file '$g_HeaderFile': $!\n);
my $first_lines = 1;
while (<IN>)
{
    # Reached the end of the class
    last if (/^};\s*/);

    # Check for copyright preamble
    if ($first_lines &&
	(/\/\*/ || length($copyright_notice)))
    {
	$copyright_notice .= $_;
	$first_lines = 0 if (/\*\//);
	next;
    }

    # Ignore comments
    next if (/^\s*\/\//);


    if (/^(class|struct)\s*(\S+)/ && !/;/)
    {
	my $kind = $1;
        $className = $2;

	# Set the default state in the event that it is not specified (as is typically the
	# case for structs
	$state = ($kind eq 'class') ? 'private' : 'public';

        # Check for inherited classes
        if (/$className\s*:\s*(.*)/)
        {
            @parentClasses = split(/\s*,\s*/, &trim($1));

	    map { $_ =~ s/^(?:public|private|protected)\s*// } @parentClasses;
        }
    }

    next if (!$className);

    # Check for state
    # Note: we process the signal signatures, but do not output them
    if (/^\s*(public|private|protected|(?:public|protected|private) (?:slots|Q_SLOTS)|signals|Q_SIGNALS)\s*:/)
    {
	$state = $1;
	$state =~ s/Q_//;
	$state = lc($state);
    }
    # Check for a constructor
    elsif (/^\s*(?:explicit\s+)?$className\s*\((.*?)\);/o)
    {
	push @constructors, {arguments => &prepArguments($1)};
    }
    # Check for a destructor
    elsif (/^\s*(virtual \s*)?~$className\s*\(\)/o)
    {
	$destructor = 1;
    }
    # Check for methods and members
    elsif (/^\s*(.*?);/)
    {
	my $text = $1;
	next if ($text =~ /\{|\}/);

	# Methods
	if ($text =~ /^(.*?)(\(.*\))\s*(.*)/)
	{
	    my ($return_type, $arguments, $extra) = (&trim($1), &trim($2), &trim($3));
	    my $tmp_state = $state;

	    # Skip if pure virtual
	    next if ($extra && $extra =~ /=\s*0$/);
	    
	    $arguments =~ s/^\(//;
	    $arguments =~ s/\)$//;
	    
	    $extra =~ s/\s*=.*// if ($extra);
	    
	    # Strip off any "virtual"
	    $return_type =~ s/^virtual \s*//;
	    
	    # Strip off any static and classify as static where necessary
	    if ($return_type =~ s/^static \s*//)
	    {
		$tmp_state = $state . ' static';
	    }
	    
	    # Tease out the method name
	    if ($return_type !~ s/([A-Za-z_]\S*)$//)
	    {
		die qq([$0] Unable to parse method name: $return_type\n);
	    }
	    
	    my $method_name = $1;
	    $tmp_state = 'operator' if ($method_name =~ /^operator/);
	    
	    # For some reason, cannot assign result of prepArguments directly in anonymous hash; returns
	    # unexpected result. Thus we use a temporary variable here.
	    my $tmp = &prepArguments($arguments);
	    push @{ $methods{$tmp_state} }, { 'name' => $method_name,
					      'return_type' => &trim($return_type),
					      'arguments' => $tmp,
					      'extra' => $extra };
	}
	# Non-pointer members
	elsif ($text =~ /^(.*)\s+([A-Za-z][A-Za-z0-9_]*)$/)
	{
	    my ($type, $name) = ($1, $2);
	    $members{$name} = { type => $type,
				name => $name };

	    push @member_names, $name;
	}
	# Special case to catch pointers
	elsif ($text =~ /^(.*\s+\*)\s*([A-Za-z][A-Za-z0-9_]*)$/)
	{
	    my ($type, $name) = ($1, $2);
	    $members{$name} = { type => $type,
				name => $name };

	    push @member_names, $name;
	}
	
    }
}
close (IN);


# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
# Dump the result
if ($g_OutputFile)
{
    if ($g_Extension)
    {
	$g_OutputFile =~ s/(.*)\..*/$1/;
	$g_OutputFile .= '.' . $g_Extension;
    }

    open(OUT, "> $g_OutputFile") or die qq([$0] Unable to open file '$g_OutputFile': $!\n);
    select OUT;
}

$g_HeaderFile =~ s/.*\///;
if (length($copyright_notice))
{
    print $copyright_notice, "\n";
}
print qq(#include "$g_HeaderFile"\n\n);

if (@constructors || $destructor)
{
    &doubleSectionMarker("Constructors and destructor");
}

foreach my $constructor (@constructors)
{
    if ($g_JavaDoc)
    {
	&openMethodComments();
	&methodArgumentComments($constructor->{arguments});
	&closeMethodComments();
    }

    # Now display the function
    print "${className}::${className}(";
    &printMethodArguments($constructor->{arguments});
    print ')';

    if (@parentClasses)
    {
	my @parents = @parentClasses;
	map { $_ .= '()' } @parents;
	print "\n    : ", join(', ', @parents);
    }
    else
    {
	print "\n";
    }
    

    # Also configure any default values
    my @defaults = ();
    foreach my $member_name (@member_names)
    {
	die qq(Expected $member_name but it is not present\n) if (!exists $members{$member_name});

	my $member = $members{$member_name};
	
	my $defaultValue;

	# Check for an argument in the constructor that matches the name of the member except for the
	# suffix.
	if (ref($constructor->{arguments}) eq 'ARRAY')
	{
	    foreach my $argument (@{ $constructor->{arguments} })
	    {
		if ($member->{name} eq $argument->{name} . '_')
		{
		    $defaultValue = $argument->{name};
		    last;
		}
	    }
	}

	if ($defaultValue) {}   # In case it was defined via a constructor argument
	elsif ($member->{type} =~ /^(?:QSet|QVector|QString|QList|QHash|QMap|QQueue|QStack)/) {}
	elsif ($member->{type} eq 'bool')                       { $defaultValue = 'false';   }
	elsif ($member->{type} =~ /\b(int|short|long)\b/)       { $defaultValue = 0;         }
	elsif ($member->{type} =~ /\b(float|qreal|double)\b/)   { $defaultValue = '0.';      }
	elsif ($member->{type} =~ /\*$/)                        { $defaultValue = 'nullptr'; }
	else
	{
	    next;
	}

	push @defaults, qq($member->{name}($defaultValue)) if ($defaultValue);
    }

    if (@defaults)
    {
	if (@parentClasses)
	{
	    print ",\n      ";
	}
	else
	{
	    print "    : ";
	}
	print join(",\n      ", @defaults);
    }

    print "\n";
    print "{\n}\n\n";
}

if ($destructor)
{
    if ($g_JavaDoc)
    {
	&openMethodComments();
	&closeMethodComments();
    }
    print "${className}::~${className}()\n";
    print "{\n";
    print "}\n\n";
}

print "\n";

my @states = ('operator', 'public', 'public slots', 'public static', 'protected', 'protected slots', 'protected static', 'private static', 'private slots', 'private');
my @non_empty_states = grep(exists $methods{$_}, @states);
foreach my $state (@non_empty_states)
{
    next if (!$methods{$state});

    my $section_name = ucfirst($state);
    if ($section_name !~ /slots/)
    {
	$section_name .= ' methods';
    }

    &doubleSectionMarker($section_name);

    foreach my $method (@{$methods{$state}})
    {
	if ($g_JavaDoc)
	{
	    &openMethodComments();
#	&blankCommentLines(2);
	    &methodArgumentComments($method->{arguments});
	    &methodReturnComments($method->{return_type});
	    &closeMethodComments();
	}
	
	print $method->{return_type};
	my $last_char = substr($method->{return_type}, -1);
	if ($last_char ne '*' && $last_char ne '&')
	{
	    print ' ';
	}
	print $className, '::', $method->{name}, '(';
	&printMethodArguments($method->{arguments});
	print ')';
	if ($method->{extra} && length($method->{extra}))
	{
	    print ' ', $method->{extra};
	}
	print "\n";
	print "{\n";

	# -----------------------------------
	# Special handling for operator== and operator !=
	if ($method->{name} =~ /^operator==/)
	{
	    die qq(Unexpected return value for operator==\n) if ($method->{return_type} ne 'bool');
	    die qq(Unexpected argument list for operator==\n) if (!$method->{arguments} || @{$method->{arguments}} != 1);

	    if (@member_names)
	    {
		my $other = $method->{arguments}->[0]->{name};

		my @comparisons = ();
		foreach my $member_name (@member_names)
		{
		    if ($members{$member_name}->{type} =~ /^(float|double|qreal)$/)
		    {
			push @comparisons, qq(qFuzzyCompare($member_name, $other.$member_name));
		    }
		    else
		    {
			push @comparisons, qq($member_name == $other.$member_name);
		    }
		}
		print "    return ", join(" &&\n           ", @comparisons), ";\n";
	    }
	    else
	    {
		print "    return false;\n";
	    }
	}
	elsif ($method->{name} =~ /^operator!=/)
	{
	    die qq(Unexpected return value for operator!=\n) if ($method->{return_type} ne 'bool');
	    die qq(Unexpected argument list for operator!=\n) if (!$method->{arguments} || @{$method->{arguments}} != 1);

	    print "     return !operator==(", $method->{arguments}->[0]->{name}, ");\n";
	}

	# -----------------------------------
	# First check if there is a member with the same name + '_' suffix and
	# with the same return type. If so, use this instead of the default value.
	elsif (exists $members{$method->{name} . '_'} &&
	    $method->{return_type} eq $members{$method->{name} . '_'}->{type})
	{
	    print "    return ", $method->{name}, '_;', "\n";
	}
	# Bools always return true or false and nothing else
	elsif ($method->{return_type} eq 'bool')
	{
	    print "    return false;\n";
	}
	elsif ($state eq 'operator' || $method->{return_type} =~ /^$className\s+&$/o)
	{
	    print "    return *this;\n";
	}
	elsif ($method->{return_type} =~ /^(int|Qt::ItemFlags|Qt::DropActions|char)$/)
	{
	    print "    return 0;\n";
	}
	elsif ($method->{return_type} =~ /^(float|double|qreal)$/)
	{
	    print "    return 0.;\n";
	}
	elsif ($method->{return_type} =~ /^(QString|QVariant|QModelIndex|QStringList|QHash|QSet|QPair|QVector|QMap)$/)
	{
	    print "    return $method->{return_type}();\n";
	}
	elsif ($last_char eq '*')
	{
	    print "    return nullptr;\n";
	}
	elsif ($method->{return_type} ne 'void')
	{
	    print "    return $method->{return_type}();\n";
	}
	# Limited support for setting member values
	elsif ($method->{return_type} eq 'void')
	{
	    # Check if this is a setter method with one argument
	    if ($method->{arguments} &&
		@{ $method->{arguments} } == 1 &&
		$method->{name} =~ /^set(\S+)/)
	    {
		my $targetMember = $1;
		$targetMember = lcfirst($targetMember) . '_';
		if (exists ($members{$targetMember}))
		{
		    print "    ", $targetMember, " = ", $method->{arguments}->[0]->{name}, ";\n";
		}
	    }
	}

	print "}\n";
	# Print extra new line, if not at the last method of the last state
	if ($state ne $non_empty_states[-1] ||
	    $method != ${$methods{$state}}[-1])
	{
	    print "\n";
	}
    }

    # Print extra new line, if not at the last state
    if ($state ne $non_empty_states[-1])
    {
	print "\n";
    }
}

if ($g_OutputFile)
{
    close(OUT);
}

# -----------------------------------------------------------------------------
# Subroutines
sub openMethodComments
{
    print qq(/**\n);
}

sub blankCommentLines
{
    my $n = shift;
    $n = 1 if (!$n);

    print "  *\n"x$n;
}

sub methodArgumentComments
{
    my $arguments = shift or return;
    foreach my $argument (@$arguments)
    {
	print qq(  * \@param $argument->{name} [$argument->{return_type}]\n);
    }
}

sub methodReturnComments
{
    my $return_type = shift or return;

    return if (!length($return_type) || $return_type eq 'void');

    print qq(  * \@returns $return_type\n);
}

sub closeMethodComments
{
    print qq(  */\n);
}

sub printMethodArguments
{
    my $arguments = shift or return;

    my @arguments = ();
    foreach my $argument (@$arguments)
    {
	my $last_char = substr($argument->{return_type}, -1);
	if ($last_char ne '*' && $last_char ne '&')
	{
	    push @arguments, qq($argument->{return_type} $argument->{name});
	}
	else
	{
	    push @arguments, qq($argument->{return_type}$argument->{name});
	}
    }

    print join(', ', @arguments);
}

sub doubleSectionMarker
{
    my $section_name = shift;

    print '// ', '-'x97, "\n";
    print '// ', '-'x97, "\n";
    if ($section_name)
    {
	print "// ", $section_name, "\n";
    }
}

sub trim
{
    my $value = shift or return;

    $value =~ s/^\s+//g;
    $value =~ s/\s+$//g;

    return $value;
}

sub prepArguments
{
    my $arguments = shift or return '';

    return '' if (length($arguments) == 0);

    # Make all double spaces into one
    $arguments =~ s/\s{2,}/ /g;

    my @arguments = split(/\s*,\s*/, &trim($arguments));
    @arguments = map { &trim($_) } @arguments;

    # Nuke any default parameters
    map { $_ =~ s/\s*=.*// } @arguments;

    # Tease out the name and type
    my @result = ();
    foreach my $argument (@arguments)
    {
	my $type = $argument;
	if ($type !~ s/([A-Za-z_]\S*)$//)
	{
	    die qq([$0] Unable to parse argument name: $argument\n);
	}
	my $name = $1;
	
	push @result, { original => $argument,
			return_type => &trim($type),
			name => $name };
    }

    return \@result;
}

sub defaultValueForType
{
    my $type = shift or die;

    
}
