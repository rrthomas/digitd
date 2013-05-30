/*
 * ================================================================
 * Please view:
 *
 *	README for program information.
 *	COPYING for distribution information.
 *
 *	based on ident2 by Michael Bacarella (defile@nyct.net)
 *
 * ================================================================
 */

#define ID_VERSION		"1.6.2"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <pwd.h>
#include <syslog.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <utmp.h>
#include <unistd.h>


#define MAX_SOCK_LENGTH		100

#define EFINGER_LIST		"/etc/efingerd/list"
#define EFINGER_LUSER		"/etc/efingerd/luser"
#define EFINGER_NOUSER		"/etc/efingerd/nouser"

#define EFINGER_USER_FILE	".finger"

/* ==================================================================
 * GLOBALS
 * ==================================================================
 */
/* SERVICE BEHAVIOR     */
const unsigned short client_timeout = 60; /* number of seconds till disconnect */


/* ------------------------------------------------------------------
 * killsock:
 *	violently kills a socket
 * ------------------------------------------------------------------
 */
static void killsock(int s)
{
    shutdown(s, SHUT_RDWR);
    close(s);
}


/* ------------------------------------------------------------------
 * get_request	:	a fgets for file descriptors
 * ------------------------------------------------------------------
 */
static int get_request(int d, char buffer[], u_short len)
{
    u_short i;
    char ch;

    memset(buffer, 0, len);
    for (i = 0; i < len; i++) {
	if (read(d, &ch, 1) != 1)
	    return -1;
	else if (ch == '\n') /* some buggy clients send only \n */
	    break;
	else if (ch == '\r') {
	    read(d, &ch, 1); /* should read the following \n */
	    break;
	}
	else
	    buffer[i] = ch;
    }
    buffer[i] = '\0';
    return i;
}


/* ------------------------------------------------------------------
 * client_reply:
 *	send a reply back to the client..
 * ------------------------------------------------------------------
 */
static void client_reply(int sd, char *outcome)
{
    write(sd, outcome, strlen(outcome));
}


/* ------------------------------------------------------------------
 * usage:
 * ------------------------------------------------------------------
 */
static void usage(char *progname)
{
    fprintf(stderr,
	    "usage: %s [options]\n"
	    "   --help     This information.\n"
	    "   --version  Print version information and exit.\n"
	    "   -t X       Time to keep connection.\n"
	    "              ex: -t 25  maintain connections for up to 25 seconds.\n",
	    progname);
    exit(0);
}


/* ------------------------------------------------------------------
 * print_version:
 *	wouldn't want to disappoint anyone.
 * ------------------------------------------------------------------
 */
static void print_version(void)
{
    fprintf(stderr, "efingerd %s\n", ID_VERSION);
    exit(0);
}


static int s_in = -1, s_out = -1;

/* ----------------------------------------------------------------
 * killtic:
 *	kill this process after a pre-determined
 *	timeout period. (SIGALRM handler)
 * ----------------------------------------------------------------
 */
static void killtic(int s)
{
    killsock(s_in);
    killsock(s_out);
    exit(0);
}


static void safe_exec(const char *cmd, char *arg1, char *arg2)
{
    int pid;
    if ((pid = fork()) == 0) {	/* Program inherits the socket */
	execl(cmd, cmd, arg1, arg2, NULL);
	_exit(0);		/* Should never happen */
    }
    wait(NULL);
}

static void do_finger(char *user, char *remote_address)
{
    const char *prog = EFINGER_NOUSER;
    if (strlen(user) == 0)
	prog = EFINGER_LIST;
    else {
	struct passwd *pwd = getpwnam(user);
	if (pwd != NULL) {
	    char *path;
	    struct stat st;
	    asprintf(&path, "%s/%s", pwd->pw_dir, EFINGER_USER_FILE);
	    if (stat(path, &st) == 0)
		prog = EFINGER_LUSER;
	    free(path);
	}
    }
    safe_exec(prog, remote_address, user);
}

/* ------------------------------------------------------------------
 * inetd_service:
 *	this function does the actual pipe handling
 *	user lookups, replies, etcetera.
 * ------------------------------------------------------------------
 */
static void inetd_service(void)
{
    struct sockaddr_in sin;
    char buffer[MAX_SOCK_LENGTH];
    socklen_t sinsize = sizeof(struct sockaddr_in);
    char remote_host[NI_MAXHOST];

    if (getpeername(s_in, (struct sockaddr *)&sin, &sinsize) == -1) {
	syslog(LOG_NOTICE, "error: getpeername: %s", strerror(errno));
	client_reply(s_out, "getpeername failed\r\n");
	return;			/* the error implies the net is down, but try */
    }
    getnameinfo((struct sockaddr *)&sin, sinsize,
		remote_host, sizeof(remote_host),
		NULL, 0, 0);

    if (getsockname(s_in, (struct sockaddr *)&sin, &sinsize) == -1) {
	syslog(LOG_ERR, "error: getsockname: %s", strerror(errno));
	client_reply(s_out, "getsockname failed\r\n");
	return;
    }
    get_request(s_in, buffer, MAX_SOCK_LENGTH);
    do_finger(buffer, remote_host);
}


int main(int argc, char *argv[])
{
    u_short i;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'v':
		print_version();
		break;

	    case 'h':
		usage(argv[0]);
		break;

	    case '-':
		if (strncmp("version", argv[i]+2, 7) == 0)
		    print_version();
		else if (strncmp("help", argv[i]+2, 4) == 0)
		    usage(argv[0]);
		break;
	    }
	}
    }

    openlog("efingerd", LOG_PID, LOG_DAEMON);
    alarm(client_timeout);
    signal(SIGALRM, killtic);

    s_in = 0;
    s_out = 1;
    inetd_service();

    killsock(0);
    killsock(1);
    closelog();
    exit(-1);
}
