#
# InspIRCd -- Internet Relay Chat Daemon
#
#   Copyright (C) 2012-2014 Peter Powell <petpow@saberuk.com>
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
	require 5.10.0;
}

package make::configure;

use feature ':5.10';
use strict;
use warnings FATAL => qw(all);

use Cwd                   qw(getcwd);
use Exporter              qw(import);
use File::Basename        qw(basename dirname);
use File::Spec::Functions qw(catfile);

use make::common;
use make::console;
use make::utilities;

use constant CONFIGURE_DIRECTORY     => '.configure';
use constant CONFIGURE_CACHE_FILE    => catfile(CONFIGURE_DIRECTORY, 'cache.cfg');
use constant CONFIGURE_CACHE_VERSION => '1';

our @EXPORT = qw(CONFIGURE_CACHE_FILE
                 CONFIGURE_CACHE_VERSION
                 cmd_clean
                 cmd_help
                 read_configure_cache
                 write_configure_cache
                 get_property );

# TODO: when buildtool is done this can be mostly removed with
#       the remainder being merged into parse_templates.
sub __get_template_settings($$$) {

	# These are actually hash references
	my ($config, $compiler, $version) = @_;

	# Start off by populating with the config
	my %settings = %$config;

	# Compiler information
	while (my ($key, $value) = each %{$compiler}) {
		$settings{'COMPILER_' . $key} = $value;
	}

	# Version information
	while (my ($key, $value) = each %{$version}) {
		$settings{'VERSION_' . $key} = $value;
	}

	# Miscellaneous information
	$settings{CONFIGURE_DIRECTORY} = CONFIGURE_DIRECTORY;
	$settings{CONFIGURE_CACHE_FILE} = CONFIGURE_CACHE_FILE;
	$settings{SYSTEM_NAME} = lc $^O;
	chomp($settings{SYSTEM_NAME_VERSION} = `uname -sr 2>/dev/null`);

	return %settings;
}

sub cmd_clean {
	unlink CONFIGURE_CACHE_FILE;
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
  --manual-dir=[dir]            The location where the manual files are stored.
                                [$PWD/run/manuals]
  --module-dir=[dir]            The location where the loadable modules are
                                stored.
                                [$PWD/run/modules]

EXTRA MODULE OPTIONS

  --enable-extras=[extras]      Enables a comma separated list of extra modules.
  --disable-extras=[extras]     Disables a comma separated list of extra modules.
  --list-extras                 Shows the availability status of all extra
                                modules.

MISC OPTIONS

  --clean                       Remove the configuration cache file and start
                                the interactive configuration wizard.
  --disable-interactive         Disables the interactive configuration wizard.
  --distribution-label=[text]   Sets a distribution specific version label in
                                the build configuration.
  --gid=[id|name]               Sets the group to run InspIRCd as.
  --help                        Show this message and exit.
  --uid=[id|name]               Sets the user to run InspIRCd as.
  --update                      Updates the build environment.


FLAGS

  CXX=[name]                    Sets the C++ compiler to use when building the
                                server. If not specified then the build system
                                will search for c++, g++, clang++ or icpc.

If you have any problems with configuring InspIRCd then visit our IRC channel
at irc.inspircd.org #InspIRCd for support.

EOH
	exit 0;
}

sub read_configure_cache {
	my %config;
	open(CACHE, CONFIGURE_CACHE_FILE) or return %config;
	while (my $line = <CACHE>) {
		next if $line =~ /^\s*($|\#)/;
		my ($key, $value) = ($line =~ /^(\S+)(?:\s(.*))?$/);
		$config{$key} = $value;
	}
	close(CACHE);
	return %config;
}

sub write_configure_cache(%) {
	unless (-e CONFIGURE_DIRECTORY) {
		print_format "Creating <|GREEN ${\CONFIGURE_DIRECTORY}|> ...\n";
		create_directory CONFIGURE_DIRECTORY, 0750 or print_error "unable to create ${\CONFIGURE_DIRECTORY}: $!";
	}

	print_format "Writing <|GREEN ${\CONFIGURE_CACHE_FILE}|> ...\n";
	my %config = @_;
	open(CACHE, '>', CONFIGURE_CACHE_FILE) or print_error "unable to write ${\CONFIGURE_CACHE_FILE}: $!";
	while (my ($key, $value) = each %config) {
		$value //= '';
		say CACHE "$key $value";
	}
	close(CACHE);
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
	return $default // '';
}

