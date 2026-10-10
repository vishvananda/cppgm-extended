#!/usr/bin/perl

# The LowIR contract audit: every public LowIR spelling has a reviewed ledger
# row (scripts/architecture/lowir-contract-ledger.tsv), is documented in
# pa8/lowir.md, and is both read and written by the implementation.
#
# The student-facing vocabulary is the ir_symbol_model.h scaffold's enums.
# The implementation spells them through the tables of
# dev/src/lowir/metadata_spellings.cpp, reads text with
# dev/src/lowir/text_parser*.cpp, writes it with dev/src/lowir/text_writer.cpp
# and spells scalar types in dev/src/lowir/program.cpp.

use strict;
use warnings;

use Cwd qw(abs_path);
use FindBin;

my $root = abs_path("$FindBin::Bin/..");

sub read_file
{
	my ($path) = @_;
	open(my $fh, '<', "$root/$path") or die "Unable to read $path: $!\n";
	local $/;
	my $data = <$fh>;
	close($fh) or die "Unable to close $path: $!\n";
	return defined($data) ? $data : '';
}

my $scaffold = read_file('dev/src/ir_symbol_model.h');
my $spellings = read_file('dev/src/lowir/metadata_spellings.cpp');
my $parser = read_file('dev/src/lowir/text_parser.cpp') .
	read_file('dev/src/lowir/text_parser_body.cpp');
my $writer = read_file('dev/src/lowir/text_writer.cpp');
my $program = read_file('dev/src/lowir/program.cpp');
my $documentation = read_file('pa8/lowir.md');
my $ledger = read_file('scripts/architecture/lowir-contract-ledger.tsv');

my @errors;
my @rows;
my %row_identity;
my @lines = split /\n/, $ledger;
my $header = shift @lines;
push @errors, "ledger header does not name the eight contract columns"
	if !defined($header) || $header ne
		"category\tsurface\tproducer\tconsumer\towner\tproperty_test\tdisposition\tevidence";

for (my $index = 0; $index < scalar(@lines); ++$index)
{
	next if $lines[$index] eq '';
	my @fields = split /\t/, $lines[$index], -1;
	my $line_number = $index + 2;
	if (scalar(@fields) != 8)
	{
		push @errors, "ledger line $line_number has " . scalar(@fields) .
			" fields instead of 8";
		next;
	}
	my %row;
	@row{qw(category surface producer consumer owner property disposition evidence)} =
		@fields;
	$row{line} = $line_number;
	my $identity = "$row{category}\t$row{surface}";
	push @errors, "duplicate ledger surface at line $line_number: $identity"
		if $row_identity{$identity}++;
	push @errors, "unresolved ledger disposition '$row{disposition}' at line $line_number"
		if $row{disposition} ne 'keep' && $row{disposition} ne 'remove';
	push @rows, \%row;
}

my @retained = grep { $_->{disposition} eq 'keep' } @rows;
for my $row (@retained)
{
	push @errors, "retained $row->{category} '$row->{surface}' has no semantic producer"
		if $row->{producer} eq '' || $row->{producer} eq 'none' ||
			$row->{producer} eq 'removed' ||
			$row->{producer} eq 'explicit LowIR only';
	push @errors, "retained $row->{category} '$row->{surface}' has no non-transport consumer"
		if $row->{consumer} eq '' || $row->{consumer} eq 'none' ||
			$row->{consumer} =~ /^transport\b/;

	for my $owner (split /;/, $row->{owner})
	{
		push @errors, "retained $row->{category} '$row->{surface}' has missing owner $owner"
			if $owner eq '' || !-f "$root/$owner";
	}
	for my $property (split /;/, $row->{property})
	{
		push @errors, "retained $row->{category} '$row->{surface}' has missing property $property"
			if $property eq '' || $property eq 'MISSING' ||
				!-f "$root/$property";
	}
}

my %count = (values => 0, keys => 0, roles => 0, instructions => 0, types => 0);

# The string literals of `name[] = { ... };` in $text.
sub table_strings
{
	my ($text, $name, $where) = @_;
	if ($text !~ /\b\Q$name\E\[\]\s*=\s*\{(.*?)\};/s)
	{
		push @errors, "cannot locate the spelling table $name in $where";
		return ();
	}
	my $body = $1;
	return $body =~ /"([^"]*)"/g;
}

# 1. Each public scaffold enum and its spelling table agree value for value:
# the default enumerator has no spelling (it is the omitted metadata) and
# every other one is spelled as its lower-cased name without the prefix.
my %table_for_enum = (
	SymbolRole => 'kRoles',
	LanguageLinkageMode => 'kLinkages',
	SymbolBindingMode => 'kBindings',
	GlobalStorageMode => 'kStorages',
	CallArityMode => 'kArities',
	CallEffectsMode => 'kEffects',
	CallUnwindMode => 'kUnwinds',
	CallReturnMode => 'kReturns',
	CallQueryMode => 'kQueries',
	ParamPassingMode => 'kPasses',
	ParamAliasMode => 'kAliases',
	IndexProjectionKind => 'kProjections',
);
my %enum_spellings;
while ($scaffold =~ /enum\s+(\w+)\s*\{(.*?)\};/sg)
{
	my ($enum, $body) = ($1, $2);
	my $table = $table_for_enum{$enum};
	next if !defined($table);
	my @names = $body =~ /\b([A-Z]+_[A-Z0-9_]+)\b/g;
	my @spelled = table_strings($spellings, $table, 'lowir/metadata_spellings.cpp');
	next if !@spelled;
	push @errors, "$enum has " . scalar(@names) . " enumerators but $table has " .
		scalar(@spelled) . " spellings"
		if scalar(@names) != scalar(@spelled);
	push @errors, "$enum default $names[0] is spelled '$spelled[0]'; it must be omitted"
		if $spelled[0] ne '';
	for my $index (1 .. $#names)
	{
		(my $want = lc $names[$index]) =~ s/^[a-z]+_//;
		my $have = $index < scalar(@spelled) ? $spelled[$index] : '<none>';
		push @errors, "$names[$index] is spelled '$have' in $table; the scaffold implies '$want'"
			if $have ne $want;
		++$count{values};
	}
	$enum_spellings{$enum} = [@spelled[1 .. $#spelled]];
}
for my $enum (sort keys %table_for_enum)
{
	push @errors, "ir_symbol_model.h has no enum $enum" if !$enum_spellings{$enum};
}

# 2. Metadata keys: the key table and the words the parser recognizes as keys
# against the ledger, pa8/lowir.md and the writer.
my %key_source = map { $_ => 'table' }
	table_strings($spellings, 'kKeys', 'lowir/metadata_spellings.cpp');
while ($parser =~ /(?:at_word\("|memcmp\([^,]+,\s*")([a-z_]+)"/g)
{
	$key_source{$1} //= 'parser';
}
my %ledger_key;
for my $row (@retained)
{
	next if $row->{category} ne 'metadata';
	(my $key = $row->{surface}) =~ s/=.*//;
	$ledger_key{$key} = 1;
}
for my $key (sort keys %ledger_key)
{
	push @errors, "retained ledger metadata key '$key' is not recognized by the parser"
		if !$key_source{$key};
}
for my $key (sort keys %key_source)
{
	# Other words the parser matches (`zero`, `copy`, ...) are not keys.
	next if $key_source{$key} eq 'parser' && !$ledger_key{$key};
	push @errors, "metadata key '$key' has no retained ledger row"
		if !$ledger_key{$key};
	push @errors, "metadata key '$key' is absent from pa8/lowir.md"
		if $documentation !~ /\b\Q$key\E\b/;
	push @errors, "metadata key '$key' is not written by lowir/text_writer.cpp"
		if $key_source{$key} eq 'parser' && $writer !~ /\Q$key\E=/;
	++$count{keys};
}

# 3. Roles: the scaffold's SymbolRole spellings against the ledger and
# pa8/lowir.md.
my %role = map { $_ => 1 } @{$enum_spellings{SymbolRole} || []};
my %ledger_role = map { $_->{surface} => $_->{disposition} }
	grep { $_->{category} eq 'role' } @rows;
for my $name (sort keys %role)
{
	push @errors, "role '$name' has no retained ledger row"
		if ($ledger_role{$name} // '') ne 'keep';
	push @errors, "role '$name' is absent from pa8/lowir.md"
		if $documentation !~ /\b\Q$name\E\b/;
	++$count{roles};
}
for my $name (sort keys %ledger_role)
{
	push @errors, "retained ledger role '$name' is not a SymbolRole of ir_symbol_model.h"
		if $ledger_role{$name} eq 'keep' && !$role{$name};
}

# 4. Instructions: every word of a retained instruction row is a parser
# keyword and is written by the writer.
my %keyword = map { $_ => 1 }
	table_strings($parser, 'kKeywordSpellings', 'lowir/text_parser.cpp'),
	table_strings($parser, 'kUnarySpellings', 'lowir/text_parser.cpp');
for my $row (@retained)
{
	next if $row->{category} ne 'instruction';
	for my $form (split /\|/, $row->{surface})
	{
		for my $word (split /\s+/, $form)
		{
			next if $word eq 'clause';
			push @errors, "ledger instruction word '$word' is not a parser keyword"
				if !$keyword{$word};
			push @errors, "ledger instruction word '$word' is not written by lowir/text_writer.cpp"
				if $writer !~ /\b\Q$word\E\b/;
		}
		++$count{instructions};
	}
}

# 5. Types: every retained type row has a scalar type spelling.
my %type;
if ($program =~ /scalar_type_spelling.*?\{(.*?)\};/s)
{
	my $body = $1;
	%type = map { $_ => 1 } $body =~ /"([^"]+)"/g;
}
else
{
	push @errors, "cannot locate scalar_type_spelling in lowir/program.cpp";
}
for my $row (@retained)
{
	next if $row->{category} ne 'type';
	(my $name = $row->{surface}) =~ s/<.*//;
	push @errors, "ledger type '$name' has no spelling in lowir/program.cpp"
		if !$type{$name};
	++$count{types};
}

if (@errors)
{
	print STDERR "LowIR contract audit: FAIL (" . scalar(@errors) .
		" issue(s))\n";
	print STDERR "  $_\n" for @errors;
	exit 1;
}

print "LowIR contract audit: PASS (" . scalar(@rows) . " ledger rows, " .
	scalar(@retained) . " retained, $count{values} public enum values, " .
	"$count{keys} metadata keys, $count{roles} roles, " .
	"$count{instructions} instruction forms, $count{types} types)\n";
