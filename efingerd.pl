#!/usr/bin/perl

use strict;
use warnings;

use Sys::Syslog qw(:standard :macros);

my $VERSION = "2.0";
my $PROGRAM = "efingerd";

my $EFINGER_LIST = "/etc/$PROGRAM/list";
my $EFINGER_LUSER = "/etc/$PROGRAM/luser";
my $EFINGER_NOUSER = "/etc/$PROGRAM/nouser";
my $EFINGER_USER_FILE = ".finger";

die("$PROGRAM version $VERSION\nNot for interactive use.\n") if -t STDIN;

openlog($PROGRAM, "pid", LOG_DAEMON);
$SIG{XCPU} = sub {
  close(STDIN);
  close(STDOUT);
  closelog();
  exit(1);
};

my $user = <STDIN>;
$user =~ s/\r?\n//; # Some buggy clients omit the \r
my $prog = $EFINGER_NOUSER;
if (length($user) == 0) {
  $prog = $EFINGER_LIST;
} else {
  my ($dir) = (getpwnam($user))[7];
  if ($dir && stat("$dir/$EFINGER_USER_FILE")) {
    $prog = $EFINGER_LUSER;
  }
}
system $prog, $user;
