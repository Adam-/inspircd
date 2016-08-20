#!/usr/bin/perl -w
# A simple IRC test client, thanks http://archive.oreilly.com/pub/h/1964

use strict;

# We will use a raw socket to connect to the IRC server.
use IO::Socket;

# The server to connect to and our details.
my $server = "127.0.0.1";
my $port = 6667;
my $nick = "testbot";
my $login = "testbot";

my $channel = "#testchannel";

print "[*] Connecting to $server:$port\n";
# Connect to the IRC server.
my $sock = new IO::Socket::INET(PeerAddr => $server,
                                PeerPort => $port,
                                Proto => 'tcp') or
                                die "[+] Can't connect to '$server:$port': $!\n";

# Log on to the server.
print $sock "NICK $nick\r\n";
print $sock "USER $login 8 * :Perl IRC Hacks Robot\r\n";

# Read lines from the server until it tells us we have connected.
while (my $input = <$sock>) {
    # Check the numerical responses from the server.
    if ($input =~ /004/) {
        # We are now logged in.
		print "[*] Connected to $server\n";
        last;
    }
    elsif ($input =~ /433/) {
        die "[+] Nickname is already in use.";
    }
}

# Join the channel.
print "[*] Joining $channel\n";
print $sock "JOIN $channel\r\n";

# Keep reading lines from the server.
while (my $input = <$sock>) {
    chop $input;
    if ($input =~ /^PING(.*)$/i) {
        # We must respond to PINGs to avoid being disconnected.
        print $sock "PONG $1\r\n";
    }
	else {
        # Print the raw line received by the bot.
        print "raw input: $input\n";
    }

	if ($input =~ /JOIN/) {
		print "[*] Sucessfully joined random channel $channel\n";
		last;
	}

}

print "[*] Test ended with success\n";
