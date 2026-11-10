#!/usr/bin/env perl
# freeze.pl --label L [--jobs N]
#
# Freezes the current source as a seed/self pair in pa36/obj/bin:
#   seed-L  dev/cppgm++ built by the host compiler (`make build`)
#   self-L  the same source built by seed-L at the g++ build's flags,
#           linked exactly like the seed (same host libraries and malloc)
# and records the commit, dirty state and hashes in pa36/obj/bin/L.txt.
# The pair is immutable: later edits do not change it.
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use File::Copy qw(copy);
use File::Path qw(make_path remove_tree);
use Getopt::Long;
use PA36;

my %a;
GetOptions(\%a, 'label=s', 'jobs=i') && $a{label} && $a{label} =~ /^[\w.+-]+$/
	or die "usage: freeze.pl --label L [--jobs N]\n";
my $root = $PA36::ROOT;
my $bin = "$PA36::PA36/obj/bin";
make_path($bin);
print STDERR "make build ...\n";
PA36::run_or_die('make', '-C', $root, 'build');
my $seed = "$bin/seed-$a{label}";
copy("$root/dev/cppgm++", $seed) or die "copy: $!\n";
chmod 0755, $seed;
my $work = "$PA36::PA36/obj/build/self-$a{label}";
remove_tree($work);
print STDERR "self build with $seed ...\n";
my ($self) = PA36::build_with(compiler => $seed, out => $work, jobs => $a{jobs});
copy($self, "$bin/self-$a{label}") or die "copy: $!\n";
chmod 0755, "$bin/self-$a{label}";
remove_tree($work);
my $head = PA36::run_or_die('git', '-C', $root, 'rev-parse', 'HEAD');
chomp $head;
my $dirty = PA36::run_or_die('git', '-C', $root, 'status', '--porcelain', '--', 'dev');
my $info = "label $a{label}\ncommit $head\ndev dirty " . ($dirty =~ /\S/ ? 'yes' : 'no') . "\n"
	. "seed sha256 " . PA36::sha256_file($seed) . "\n"
	. "self sha256 " . PA36::sha256_file("$bin/self-$a{label}") . "\n";
PA36::spit("$bin/$a{label}.txt", $info);
print $info, "seed $seed\nself $bin/self-$a{label}\n";
