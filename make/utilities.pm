#
# InspIRCd -- Internet Relay Chat Daemon
#
#   Copyright (C) 2010 Daniel De Graaf <danieldg@inspircd.org>
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

package make::utilities;

use strict;
use warnings FATAL => qw(all);

use Exporter 'import';
use Fcntl;
use File::Path;
use File::Temp;
use Getopt::Long;
use POSIX;

our @EXPORT = qw(promptstring);

my %already_added = ();

sub promptstring($$$$$)
{
	my ($prompt, $configitem, $default, $package, $commandlineswitch) = @_;
	my $var;
	if (!$main::interactive)
	{
		my $opt_commandlineswitch;
		GetOptions ("$commandlineswitch=s" => \$opt_commandlineswitch);
		if (defined $opt_commandlineswitch)
		{
			print "\e[1;32m$opt_commandlineswitch\e[0m\n";
			$var = $opt_commandlineswitch;
		}
		else
		{
			die "Could not detect $package! Please specify the $prompt via the command line option \e[1;32m--$commandlineswitch=\"/path/to/file\"\e[0m";
		}
	}
	else
	{
		print "\nPlease enter the $prompt?\n";
		print "[\e[1;32m$default\e[0m] -> ";
		chomp($var = <STDIN>);
	}
	if ($var eq "")
	{
		$var = $default;
	}
	$main::config{$configitem} = $var;
}

1;

