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


sub finish {
  close(STDIN);
  close(STDOUT);
  closelog();
  exit(0);
}

sub finger {
  my $user = shift;
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
}

die("$PROGRAM version $VERSION\nNot for interactive use.\n") if -t STDIN;

openlog($PROGRAM, "pid", LOG_DAEMON);
$SIG{XCPU} = &finish;
my $user = <STDIN>;
$user =~ s/\r?\n//; # Some buggy clients omit the \r
finger($user);
