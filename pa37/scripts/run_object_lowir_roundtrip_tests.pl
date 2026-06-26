#!/usr/bin/env perl
use strict;
use warnings;

use File::Find qw(find);
use File::Temp qw(tempdir);
use Text::ParseWords qw(shellwords);

sub usage {
  die "usage: run_object_lowir_roundtrip_tests.pl --app APP (--test-root DIR | --test FILE)...\n";
}

my $app = "../dev/cppgm++";
my @roots;
my @tests;

while(@ARGV) {
  my $arg = shift @ARGV;
  if($arg eq "--app") {
    usage() unless @ARGV;
    $app = shift @ARGV;
  } elsif($arg eq "--test-root") {
    usage() unless @ARGV;
    push @roots, shift @ARGV;
  } elsif($arg eq "--test") {
    usage() unless @ARGV;
    push @tests, shift @ARGV;
  } elsif($arg eq "-h" || $arg eq "--help") {
    usage();
  } else {
    die "unknown argument: $arg\n";
  }
}

sub collect_tests {
  my @out;
  for my $root (@roots) {
    if(-f $root) {
      push @out, $root;
      next;
    }
    next unless -d $root;
    my @found;
    find(
      {
        wanted => sub {
          return unless -f $_;
          return unless /\.cpp\z/;
          push @found, $File::Find::name;
        },
        no_chdir => 1,
      },
      $root);
    push @out, sort @found;
  }
  push @out, @tests;

  my %seen;
  return grep { !$seen{$_}++ } @out;
}

sub read_flags {
  my ($path) = @_;
  return () unless -f $path;
  open(my $fh, "<", $path) or die "open $path: $!\n";
  local $/;
  my $text = <$fh>;
  close($fh) or die "close $path: $!\n";
  return () unless defined($text) && $text =~ /\S/;
  return shellwords($text);
}

sub run_command {
  my (@cmd) = @_;
  system { $cmd[0] } @cmd;
  my $status = $?;
  return if $status == 0;
  print STDERR "command failed:\n  ", join(" ", @cmd), "\n";
  exit($status & 127 ? 128 + ($status & 127) : ($status >> 8));
}

sub read_bytes {
  my ($path) = @_;
  open(my $fh, "<:raw", $path) or die "open $path: $!\n";
  local $/;
  my $data = <$fh>;
  close($fh) or die "close $path: $!\n";
  return defined($data) ? $data : "";
}

sub object_summary {
  my ($path) = @_;
  open(my $fh, "-|", "nm", "-a", $path) or return "";
  my @lines = grep { /\S/ } <$fh>;
  close($fh);
  @lines = sort @lines;
  splice(@lines, 80) if @lines > 80;
  return join("", @lines);
}

sub safe_name {
  my ($path) = @_;
  $path =~ s/[^A-Za-z0-9]/_/g;
  return $path;
}

sub check_one {
  my ($source, $temp) = @_;
  die "missing object-roundtrip test source: $source\n" unless -f $source;

  my @extra_flags = read_flags("$source.compile.flags");
  my $name = safe_name($source);
  for my $opt ("-O0", "-O1", "-O2") {
    my $direct = "$temp/$name." . substr($opt, 1) . ".direct.o";
    my $roundtrip = "$temp/$name." . substr($opt, 1) . ".roundtrip.o";
    my @common = ($app, "-c", "-g0", $opt, @extra_flags);
    run_command(@common, "-o", $direct, $source);
    run_command(@common, "--roundtrip-object-lowir", "-o", $roundtrip, $source);

    my $direct_bytes = read_bytes($direct);
    my $roundtrip_bytes = read_bytes($roundtrip);
    next if $direct_bytes eq $roundtrip_bytes;

    print STDERR "object differs after LowIR text roundtrip: $source $opt\n";
    print STDERR "direct bytes: ", length($direct_bytes), "\n";
    print STDERR "roundtrip bytes: ", length($roundtrip_bytes), "\n";
    print STDERR "direct symbols:\n", object_summary($direct);
    print STDERR "roundtrip symbols:\n", object_summary($roundtrip);
    exit 1;
  }
}

my @selected = collect_tests();
die "no object-roundtrip tests selected\n" unless @selected;

print "pa37 object-roundtrip: running ", scalar(@selected), " test";
print "s" if @selected != 1;
print "\n";

my $temp = tempdir("cppgm-object-lowir-roundtrip.XXXXXX", TMPDIR => 1, CLEANUP => 1);
for my $source (@selected) {
  check_one($source, $temp);
}

print "pa37 object-roundtrip: PASS (", scalar(@selected), "/", scalar(@selected), ")\n";
