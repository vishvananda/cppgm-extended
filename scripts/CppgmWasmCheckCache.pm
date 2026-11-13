package CppgmWasmCheckCache;

use strict;
use warnings;
use Digest::SHA qw(sha256_hex);
use JSON::PP;
use CppgmBatchWorker qw(run_command_capture);

# One checker owns this cache. Compiler invocations always run; only successful
# observations of immutable inputs by the trusted validator/runtime are reused.
sub new {
    my ($class, %tools) = @_;
    return bless {tools => \%tools, successful => {}}, $class;
}

sub read_bytes {
    my ($path) = @_;
    open(my $file, '<:raw', $path) or die "Cannot read $path: $!\n";
    local $/;
    return scalar(<$file>) // '';
}

sub key {
    my ($self, $options) = @_;
    my @args = @{$options->{cmd}};
    my $runtime = $args[0] eq $self->{tools}{runtime};
    return unless ($args[0] eq $self->{tools}{validator} && $args[1] eq 'validate')
        || ($runtime && $args[1] eq 'run' && grep {$_ eq '--invoke'} @args);
    return unless $options->{limits};
    return if $options->{stdin} || ($options->{env} && keys %{$options->{env}})
        || grep {$_ eq '--dir' || $_ eq '--env'} @args;

    my @identity;
    for my $arg (@args) {
        my ($prefix, $path) = ('', $arg);
        ($prefix, $path) = ($1, $2) if $arg =~ /\A([^=]+=)(.+)\z/;
        if (-f $path && $path =~ /(?:\.wasm(?:\.[12])?|\.o)\z/) {
            my $bytes = read_bytes($path);
            # WASI can observe argv[0], files and other host state. This also
            # checks preloads, including the host used for proc_exit assertions.
            return if $runtime && ($bytes =~ /wasi_snapshot_preview1|wasi_unstable/);
            push @identity, $prefix . 'sha256:' . sha256_hex($bytes);
        } else {
            push @identity, $arg;
        }
    }
    return JSON::PP->new->canonical->encode({args => \@identity,
        timeout => $options->{timeout}, limits => $options->{limits},
        internal_error => $options->{internal_error}});
}

sub capture {
    my ($self, %options) = @_;
    my $key = $self->key(\%options);
    if (defined($key) && exists $self->{successful}{$key}) {
        my $streams = $self->{successful}{$key};
        for my $index (0, 1) {
            my $path = $options{$index ? 'stderr' : 'stdout'};
            open(my $file, '>:raw', $path) or die "Cannot write $path: $!\n";
            print {$file} $streams->[$index];
            close($file) or die "Cannot close $path: $!\n";
        }
        %{$options{outcome}} = (kind => 'exit', status => 0) if ref($options{outcome}) eq 'HASH';
        return 0;
    }
    my $status = run_command_capture(%options);
    if (defined($key) && $status == 0) {
        my $streams = [read_bytes($options{stdout}), read_bytes($options{stderr})];
        $self->{successful}{$key} = $streams if length($streams->[0]) + length($streams->[1]) <= 65536;
    }
    return $status;
}

1;
