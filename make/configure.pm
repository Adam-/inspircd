#
# InspIRCd -- Internet Relay Chat Daemon
#
#   Copyright (C) 2012 Peter Powell <petpow@saberuk.com>
#   Copyright (C) 2008 Robin Burchell <robin+git@viroteck.net>
#   Copyright (C) 2007-2008 Craig Edwards <craigedwards@brainbox.cc>
#   Copyright (C) 2008 Thomas Stagner <aquanight@inspircd.org>
#   Copyright (C) 2007 Dennis Friis <peavey@inspircd.org>
#
# This file is part of InspIRCd.  InspIRCd is free software: you can
# redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


BEGIN {
	require 5.8.0;
}

package make::configure;

use strict;
use warnings FATAL => qw(all);

use Cwd;
use Exporter 'import';

use make::utilities;

our @EXPORT = qw(cmd_clean cmd_help cmd_update
                 read_configure_cache write_configure_cache
                 get_property get_revision
                 dump_hash);

my $revision;

sub cmd_clean {
	unlink '.config.cache';
}

sub cmd_help {
	my $PWD = getcwd();
	print <<EOH;
Usage: $0 [options]

When no options are specified, configure runs in interactive mode and you must
specify any required values manually. If one or more options are specified,
non-interactive configuration is started and any omitted values are defaulted.

PATH OPTIONS

  --system                      Automatically set up the installation paths
                                for system-wide installation.
  --prefix=[dir]                The root install directory. If this is set then
                                all subdirectories will be adjusted accordingly.
                                [$PWD/run]
  --binary-dir=[dir]            The location where the main server binary is
                                stored.
                                [$PWD/run/bin]
  --config-dir=[dir]            The location where the configuration files and
                                SSL certificates are stored.
                                [$PWD/run/conf]
  --data-dir=[dir]              The location where the data files, such as the
                                pid file, are stored.
                                [$PWD/run/data]
  --log-dir=[dir]               The location where the log files are stored.
                                [$PWD/run/logs]
  --module-dir=[dir]            The location where the loadable modules are
                                stored.
                                [$PWD/run/modules]
  --build-dir=[dir]             The location to store files in while building.


EXTRA MODULE OPTIONS

  --enable-extras=[extras]      Enables a comma separated list of extra modules.
  --disable-extras=[extras]     Disables a comma separated list of extra modules.
  --list-extras                 Shows the availability status of all extra
                                modules.

MISC OPTIONS

  --clean                       Remove the configuration cache file and start
                                the interactive configuration wizard.
  --disable-interactive         Disables the interactive configuration wizard.
  --help                        Show this message and exit.
  --uid=[name]                  Sets the user to run InspIRCd as.


FLAGS

  CXX=[name]                    Sets the C++ compiler to use when building the
                                server. If not specified then the build system
                                will search for c++, g++, clang++ or icpc.

If you have any problems with configuring InspIRCd then visit our IRC channel
at irc.ChatSpike.net #InspIRCd.

EOH
	exit 0;
}

sub read_configure_cache {
	my %cfg = ();
	open(CACHE, '.config.cache') or return %cfg;
	while (my $line = <CACHE>) {
		next if $line =~ /^\s*($|\#)/;
		my ($key, $value) = ($line =~ /^(\S+)="(.*)"$/);
		$cfg{$key} = $value;
	}
	close(CACHE);
	return %cfg;
}

sub write_configure_cache(%) {
	my %cfg = @_;
	open(CACHE, ">.config.cache") or return 0;
	while (my ($key, $value) = each %cfg) {
		$value = "" unless defined $value;
		print CACHE "$key=\"$value\"\n";
	}
	close(CACHE);
	return 1;
}

sub get_property($$;$)
{
	my ($file, $property, $default) = @_;
	open(MODULE, $file) or return $default;
	while (<MODULE>) {
		if ($_ =~ /^\/\* \$(\S+): (.+) \*\/$/) {
			next unless $1 eq $property;
			close(MODULE);
			return translate_functions($2, $file);
		}
	}
	close(MODULE);
	return defined $default ? $default : '';
}

sub get_revision
{
	return $revision if defined $revision;
	chomp(my $tags = `git describe --tags 2>/dev/null`);
	$revision = $tags || 'release';
	return $revision;
}

sub dump_hash()
{
	print "\n\e[1;32mPre-build configuration is complete!\e[0m\n\n";
	print "\e[0mBase install path:\e[1;32m\t\t$main::config{BASE_DIR}\e[0m\n";
	print "\e[0mConfig path:\e[1;32m\t\t\t$main::config{CONFIG_DIR}\e[0m\n";
	print "\e[0mData path:\e[1;32m\t\t\t$main::config{DATA_DIR}\e[0m\n";
	print "\e[0mLog path:\e[1;32m\t\t\t$main::config{LOG_DIR}\e[0m\n";
	print "\e[0mModule path:\e[1;32m\t\t\t$main::config{MODULE_DIR}\e[0m\n";
	#print "\e[0mCompiler:\e[1;32m\t\t\t$main::cxx{NAME} $main::cxx{VERSION}\e[0m\n";
	#print "\e[0mSocket engine:\e[1;32m\t\t\t$main::config{SOCKETENGINE}\e[0m\n";
	#print "\e[0mGnuTLS support:\e[1;32m\t\t\t$main::config{USE_GNUTLS}\e[0m\n";
	#print "\e[0mOpenSSL support:\e[1;32m\t\t$main::config{USE_OPENSSL}\e[0m\n";
}

1;
