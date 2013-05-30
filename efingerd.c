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
unsigned char resolve_addr;		/* reverse lookup addresses	*/
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
	    "              ex: -t 25  maintain connections for up to 25 seconds.\n"
	    "   -n         Do not lookup addresses, use IP numbers instead.\n",
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

/* ------------------------------------------------------------------
 * lookup_addr:
 *      if resolve_addr, try to reverse resolve the address.
 *              else return the numerical ip.
 * ------------------------------------------------------------------
 */
static char *lookup_addr(struct in_addr in)
{
    static char addr[100];
    struct hostent *he = NULL;

    if (resolve_addr) {
	he = gethostbyaddr((char *) &in, sizeof(struct in_addr), AF_INET);
	if (he != NULL)
	    strncpy(addr, he->h_name, sizeof(addr));
    }
    if (he == NULL)
	strncpy(addr, inet_ntoa(in), sizeof(addr));

    addr[sizeof(addr)-1] = '\0';
    return addr;
}


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


static void safe_exec(char *cmd, char *arg1, char *arg2)
{
    int pid;
    if ((pid = fork()) == 0) {	/* Program inherits the socket */
	execl(cmd, cmd, arg1, arg2, NULL);
	_exit(0);		/* Should never happen */
    }
    wait(NULL);
}

static void do_finger(char *user, char *remote_address, int sd_out)
{
    struct passwd *passs;
    struct stat st;
    char path[200];

    if (strlen(user) == 0)
	safe_exec(EFINGER_LIST, remote_address, NULL);
    else {
	passs = getpwnam(user);
	if (passs == NULL)
	    safe_exec(EFINGER_NOUSER, remote_address, user);
	else {
	    if (sizeof(path) >= strlen(passs->pw_dir) + sizeof(EFINGER_USER_FILE) + 1) {
		strncpy(path, passs->pw_dir, sizeof(path));
		strcat(path, "/");
		strcat(path, EFINGER_USER_FILE);
		if (stat(path, &st))
		    safe_exec(EFINGER_NOUSER, remote_address, user);
		else
		    safe_exec(EFINGER_LUSER, remote_address, user);
	    }
	}
    }
}

/* ------------------------------------------------------------------
 * inetd_service:
 *	this function does the actual pipe handling
 *	user lookups, replies, etcetera.
 * ------------------------------------------------------------------
 */
static void inetd_service(int sd_in, int sd_out)
{
    struct in_addr raddr;
    struct sockaddr_in sin;
    char buffer[MAX_SOCK_LENGTH];
    socklen_t sinsize = sizeof(struct sockaddr_in);
    char *remote_address;

    s_in = sd_in;
    s_out = sd_out;

    if (getpeername(sd_in, (struct sockaddr *) &sin, &sinsize) == -1) {
	syslog(LOG_NOTICE, "error: getpeername: %s", strerror(errno));
	client_reply(sd_out, "401 getpeername failed\r\n");
	return;			/* the error implies the net is down, but try */
    }
    raddr = sin.sin_addr;

    if (getsockname(sd_in, (struct sockaddr *) &sin, &sinsize) == -1) {
	syslog(LOG_ERR, "error: getsockname: %s", strerror(errno));
	client_reply(sd_out, "402 getsockname failed\r\n");
	return;
    }

    get_request(sd_in, buffer, MAX_SOCK_LENGTH);
    remote_address = lookup_addr(raddr);
    do_finger(buffer, remote_address, sd_out);
}


int main(int argc, char *argv[])
{
    u_short i;

    resolve_addr = 1;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'v':
		print_version();
		break;

	    case 'n':
		resolve_addr = 0;
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

    inetd_service(0, 1);

    killsock(0);
    killsock(1);
    closelog();
    exit(-1);
}
