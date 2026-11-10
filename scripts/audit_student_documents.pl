#!/usr/bin/env perl
# Check reviewed student prose for private compiler implementation material.
use strict;
use warnings;
use FindBin;
use Getopt::Long;

my $source = "$FindBin::Bin/..";
my %o = (root => $source, manifest => "$source/scripts/student_export_documents.txt");
GetOptions(\%o, 'root=s', 'manifest=s') && !@ARGV or die "usage: audit_student_documents.pl [--root DIR] [--manifest FILE]\n";
sub read_text {
    my ($path) = @_;
    open(my $fh, '<', $path) or die "missing student document: $path\n";
    local $/; return <$fh>;
}
my @docs = grep { /\.md$/ } split /\n/, read_text($o{manifest});
my $overrides = "$o{root}/docs/student-export-root";
for my $path (glob("$source/docs/student-export-root/*.md")) {
    $path =~ s{.*/}{};
    push @docs, -d $overrides ? "docs/student-export-root/$path" : $path;
}
my $export = read_text("$source/scripts/export_student_repo.sh");
$export =~ /dev_support_files=\(\n(.*?)\n\)/s or die "missing scaffold inventory\n";
my %public = map { $_ => 1 } $1 =~ /^\s*(dev\/\S+)/mg;
my @rules = (
    ['private work path', qr{(?:/home/[^\s`]+|obj/wt-[^\s`]+|handoff\.md|research_notes/)}],
    ['private fixture or journal', qr{(?:tests/regression|pa\d+/maintainer/|optimization-course-plan\.md)}],
    ['private implementation API', qr{\b(?:FunctionContext|scratch_carried_reloads|pa33_opt_stats|read_only_functions|kUnroll\w*|kSearch\w*)\b|--inline-limit|\bonce-cap\b}],
    ['reference tuning policy', qr{course solution(?:'s)?\s+(?:uses|keeps|runs|chooses|decides|places|inlines|carries)\b}],
    ['benchmark development history', qr{\b(?:PENNANT|tramp3d-v4|fftbench|SmallPT)\b|\bfewer cycles\b|\bcycles off\b}],
);
my @errors;
for my $doc (@docs) {
    my $text = read_text("$o{root}/$doc");
    if ($doc =~ m{^pa(\d+)/README\.md$}) {
        my $owner = $1;
        my ($title) = $text =~ /^\#+ CPPGM Programming Assignment (\d+)\b/m;
        push @errors, "$doc:1: assignment title must identify PA$owner"
            if !defined($title) || $title != $owner;
    }
    for my $rule (@rules) {
        my $pattern = $rule->[1];
        while ($text =~ /$pattern/g) {
            my $line = 1 + (() = substr($text, 0, $-[0]) =~ /\n/g);
            push @errors, "$doc:$line: $rule->[0]";
        }
    }
    my @lines = split /\n/, $text;
    for my $n (0 .. $#lines) {
        my $line = $lines[$n];
        while ($line =~ m{\b(dev/src/[A-Za-z0-9_./+-]+\.(?:h|cpp))\b}g) {
            my $path = $1;
            push @errors, "$doc:" . ($n + 1) . ": private source path $path"
                if -f "$source/$path" && !$public{$path};
        }
    }
}
if (@errors) { print "Student document audit: FAIL\n", join("\n", @errors), "\n"; exit 1; }
print "Student document audit: PASS (" . scalar(@docs) . " reviewed prose files)\n";
