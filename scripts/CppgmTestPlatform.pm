package CppgmTestPlatform;

use strict;
use warnings;
use Cwd qw(getcwd);
use Fcntl qw(LOCK_EX);

sub platform
{
	my $host = $ENV{CPPGM_TEST_PLATFORM} // ($^O eq 'darwin' ? 'macos' : $^O);
	die "Unknown test platform '$host'\n" unless $host =~ /\A(?:linux|macos)\z/;
	return $host;
}

sub restriction
{
	my ($file) = @_;
	return '' if !-f $file;
	open(my $input, '<', $file) or die "Cannot read $file: $!\n";
	my (%allowed, @reasons);
	while (my $line = <$input>)
	{
		chomp($line);
		my ($names, $reason) = split(/#/, $line, 2);
		push @reasons, $reason if defined($reason) && $reason =~ /\S/;
		for my $name (split(/\s+/, $names))
		{
			next if $name eq '';
			die "$file: unknown platform '$name'\n" unless $name =~ /\A(?:linux|macos)\z/;
			$allowed{$name} = 1;
		}
	}
	close($input);
	die "$file: list supported platforms and a # reason\n" if !%allowed || !@reasons;
	return '' if $allowed{platform()} || ($ENV{CPPGM_TEST_INCLUDE_ALL} // '') eq '1';
	my $reason = join(' ', @reasons);
	$reason =~ s/^\s+|\s+$//g;
	return 'requires ' . join('/', sort keys %allowed) . ': ' . $reason;
}

sub skip
{
	my ($label, $reason) = @_;
	$reason =~ s/[\r\n\t]+/ /g;
	my $file = $ENV{CPPGM_TEST_SKIPS_FILE};
	if (defined($file) && $file ne '')
	{
		$label = getcwd() . '/' . $label unless $label =~ m{^/};
		open(my $output, '>>', $file) or die "Cannot record platform skip in $file: $!\n";
		flock($output, LOCK_EX) or die "Cannot lock $file: $!\n";
		print $output "$label\t$reason\n";
		close($output) or die "Cannot close $file: $!\n";
	}
	else
	{
		# Discovery commands return fixture paths on stdout. Diagnostics
		# must not become apparent paths when no report file is configured.
		print STDERR "SKIP $label: $reason\n";
	}
}

sub select_tests
{
	my @selected;
	for my $test (@_)
	{
		(my $base = $test) =~ s/\.(?:t(?:\.1)?|cpp|lowir)$//;
		my $reason = restriction("$base.platforms");
		if ($reason ne '') { skip($test, $reason); }
		else { push @selected, $test; }
	}
	return @selected;
}

sub inspection_supported
{
	my ($base) = @_;
	my $reason = restriction("$base.inspect.platforms");
	if ($reason eq '' && !-f "$base.inspect.platforms" &&
		!-f "$base.inspect.cmd" && !-f("$base.inspect.cmd." . platform()) &&
		!-f "$base.inspect.facts" && !-f "$base.inspect.expect" && !-f "$base.inspect.plan")
	{
		my @provided = grep { -f "$base.inspect.cmd.$_" } qw(linux macos);
		$reason = 'inspection command provided only for ' . join('/', @provided)
			if @provided && ($ENV{CPPGM_TEST_INCLUDE_ALL} // '') ne '1';
	}
	return 1 if $reason eq '';
	skip("$base inspection", $reason);
	return 0;
}

sub require_platform
{
	my ($required, $label, $reason) = @_;
	return 1 if platform() eq $required || ($ENV{CPPGM_TEST_INCLUDE_ALL} // '') eq '1';
	skip($label, "requires $required: $reason");
	return 0;
}

sub host_output_oracle
{
	my ($base) = @_;
	my $file = "$base.oracle." . platform();
	$file = "$base.oracle" if !-f $file;
	return 0 if !-f $file;
	open(my $input, '<', $file) or die "Cannot read $file: $!\n";
	local $/;
	my $text = <$input>;
	close($input);
	$text =~ s/#.*$//mg;
	die "$file: expected host-output\n" unless $text =~ /\A\s*host-output\s*\z/;
	return 1;
}

sub summary
{
	my ($file) = @_;
	return if !-f $file;
	open(my $input, '<', $file) or die "Cannot read $file: $!\n";
	my %skips = map { chomp; $_ => 1 } <$input>;
	close($input);
	return if !%skips;
	print '===== PLATFORM SKIPS: ' . scalar(keys %skips) . ' (' . platform() . ") =====\n";
	my $root = getcwd() . '/';
	for my $line (sort keys %skips)
	{
		$line =~ s/^\Q$root\E//;
		$line =~ s/\t/: /;
		print "SKIP $line\n";
	}
}

1;
