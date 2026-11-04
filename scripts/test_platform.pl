#!/usr/bin/perl
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use CppgmTestPlatform;

if (@ARGV == 2 && $ARGV[0] eq '--summary')
{
	CppgmTestPlatform::summary($ARGV[1]);
	exit 0;
}
die "usage: test_platform.pl <linux|macos> <label> <reason> -- <command> [args...]\n"
	if @ARGV < 5 || $ARGV[3] ne '--' || $ARGV[0] !~ /\A(?:linux|macos)\z/;
my ($required, $label, $reason) = splice(@ARGV, 0, 4);
if (CppgmTestPlatform::platform() ne $required && ($ENV{CPPGM_TEST_INCLUDE_ALL} // '') ne '1')
{
	CppgmTestPlatform::skip($label, "requires $required: $reason");
	exit 0;
}
exec(@ARGV) or die "Cannot execute $ARGV[0]: $!\n";
