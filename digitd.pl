#!/usr/bin/perl
# digitd: a simple, flexible and safe finger daemon
# (c) 2013 Reuben Thomas <rrt@sc3d.org>
# Released under the GPL version 3, or (at your option) any later version.

use strict;
use warnings;

my $VERSION = "1.0";
my $PROGRAM = "digitd";

my $SCRIPT_LIST = "/etc/$PROGRAM/list";
my $SCRIPT_USER = "/etc/$PROGRAM/user";
my $SCRIPT_NOUSER = "/etc/$PROGRAM/nouser";
my $USER_FILE = ".finger";

die("$PROGRAM version $VERSION\nNot for interactive use.\n") if -t STDIN;

my $user = <STDIN>;
$user =~ s/\r?\n//; # Some buggy clients omit the \r
my $prog = $SCRIPT_NOUSER;
if (length($user) == 0) {
  $prog = $SCRIPT_LIST;
} else {
  my ($dir) = (getpwnam($user))[7];
  if ($dir && stat("$dir/$USER_FILE")) {
    $prog = $SCRIPT_USER;
  }
}
$SIG{ALRM} = sub { exit(1); };
alarm 60;
system $prog, $user;
