# digitd

© Reuben Thomas <rrt@sc3d.org>  
https://github.com/rrthomas/digitd  

digitd is a simple and safe finger daemon, ready-to-run out of the
box, yet easy to customize. It allows users to opt in to being
fingerable, rather than the usual opt-out.

digitd is distributed under the GNU General Public License version 3,
or, at your option, any later version.

It is based on Radovan Garabik’s efingerd, simplified and completely
rewritten.

digitd implements the Finger User Information Protocol as described in
RFC 1288: http://tools.ietf.org/html/rfc1288

See the man page digitd(8) for more information.


## Installation

Requirements: Perl on a system supporting syslog (GNU/Linux, macOS,…)

1. As root, type: `make install`
2. Add this line to `/etc/inetd.conf`

```
finger	stream	tcp	nowait	nobody	/usr/sbin/tcpd	/usr/local/sbin/digitd
```

If there is already a finger line, comment it out. Note that the
entries are separated by tabs, not spaces.

You can create a user under which the daemon should run (e.g. `digitd`),
in which case its name should replace `nobody` in the line above.

3. Restart inetd with: `killall -HUP inetd`
