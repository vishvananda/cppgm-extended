#!/usr/bin/perl

use strict;
use warnings;
use Digest::SHA;
use Fcntl qw(:flock);
use File::Copy qw(copy);
use File::Path qw(make_path remove_tree);
use File::Spec;
use File::Temp qw(tempdir);
use FindBin;
use Getopt::Long qw(GetOptions);
use JSON::PP;
use POSIX qw(uname);

my $prefix = 'obj/wasm-tools';
GetOptions('prefix=s' => \$prefix) && !@ARGV
    or die "usage: install_wasm_tools.pl [--prefix directory]\n";
$prefix = File::Spec->rel2abs($prefix);
make_path("$prefix/bin");
open(my $lock, '>', "$prefix/.install.lock") or die "$prefix: $!\n";
flock($lock, LOCK_EX) or die "Cannot lock $prefix: $!\n";

sub read_json {
    my ($path) = @_;
    open(my $file, '<', $path) or die "Cannot read $path: $!\n";
    local $/;
    return JSON::PP::decode_json(<$file>);
}

sub digest {
    my ($path) = @_;
    open(my $file, '<', $path) or die "Cannot read $path: $!\n";
    binmode($file);
    return Digest::SHA->new(256)->addfile($file)->hexdigest;
}

my (undef, undef, undef, undef, $architecture) = uname();
$architecture = 'aarch64' if $architecture eq 'arm64';
my $system = $^O eq 'darwin' ? 'macos' : $^O;
my $manifest = read_json("$FindBin::Bin/wasm_tools.json");
my $tools = $manifest->{hosts}{"$architecture-$system"}
    or die "No pinned Wasm tools for $architecture-$system\n";
my $installed = -f "$prefix/installed.json" ? read_json("$prefix/installed.json") : {};
for my $name (sort keys %$tools) {
    my $tool = $tools->{$name};
    my $sdk = ($tool->{kind} // '') eq 'sdk';
    my $binary = $sdk ? "$prefix/wasi-sdk/bin/clang++" : "$prefix/bin/$name";
    next if -x $binary && ($installed->{$name}{archive_sha256} // '') eq $tool->{sha256}
        && ($installed->{$name}{binary_sha256} // '') eq digest($binary);
    my $temporary = tempdir('wasm-tools-XXXXXX', DIR => $prefix, CLEANUP => 1);
    my $archive = "$temporary/download.tar";
    system('curl', '--fail', '--location', '--retry', '3', '--connect-timeout', '15',
        '--max-time', '300', '--output', $archive, $tool->{url}) == 0
        or die "Cannot download pinned $name $tool->{version}\n";
    digest($archive) eq $tool->{sha256}
        or die "Archive checksum mismatch for $name $tool->{version}\n";
    system('tar', '-xf', $archive, '-C', $temporary) == 0
        or die "Cannot extract $name\n";
    if ($sdk) {
        remove_tree("$prefix/wasi-sdk");
        rename("$temporary/$tool->{directory}", "$prefix/wasi-sdk")
            or die "Cannot install WASI SDK: $!\n";
        $installed->{$name} = {
            version => $tool->{version}, archive_sha256 => $tool->{sha256}, binary_sha256 => digest($binary)
        };
        next;
    }
    my $source = "$temporary/$tool->{directory}/$name";
    copy($source, "$binary.new") or die "Cannot install $name: $!\n";
    chmod(0755, "$binary.new") or die "Cannot make $name executable: $!\n";
    rename("$binary.new", $binary) or die "Cannot replace $binary: $!\n";
    $installed->{$name} = {
        version => $tool->{version}, archive_sha256 => $tool->{sha256}, binary_sha256 => digest($binary)
    };
}
for my $tool (['clang++', 'wasm-ref-clang++'], ['wasm-ld', 'wasm-ref-ld'], ['llvm-ar', 'wasm-ref-ar']) {
    my ($source, $name) = @$tool;
    my $link = "$prefix/bin/$name";
    unlink($link) if -l $link;
    symlink("../wasi-sdk/bin/$source", $link) or die "Cannot link $name: $!\n";
}
open(my $marker, '>', "$prefix/installed.json") or die "$prefix: $!\n";
print $marker JSON::PP->new->canonical->pretty->encode($installed);
close($marker) or die "Cannot finish install metadata: $!\n";
print "Pinned Wasm tools installed in $prefix/bin\n";
