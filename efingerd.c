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
 * get_request:
 *	fgets for file descriptors
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


/* ----------------------------------------------------------------
 * killtic:
 *	kill this process after a pre-determined
 *	timeout period. (SIGALRM handler)
 * ----------------------------------------------------------------
 */
static void killtic(int s)
{
    killsock(STDIN_FILENO);
    killsock(STDOUT_FILENO);
    closelog();
    exit(0);
}


static void safe_exec(const char *cmd, char *arg1)
{
    int pid;
    if ((pid = fork()) == 0) {	/* Program inherits the socket */
	execl(cmd, cmd, arg1, NULL);
	_exit(0);		/* Should never happen */
    }
    wait(NULL);
}

static void do_finger(char *user)
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
    safe_exec(prog, user);
}

int main(int argc, char *argv[])
{
    char buffer[MAX_SOCK_LENGTH];

    if (isatty(STDIN_FILENO)) {
	fprintf(stderr, "efingerd version %s\nNot for interactive use.\n", ID_VERSION);
	exit(0);
    }

    openlog("efingerd", LOG_PID, LOG_DAEMON);
    alarm(client_timeout);
    signal(SIGALRM, killtic);
    get_request(STDIN_FILENO, buffer, MAX_SOCK_LENGTH);
    do_finger(buffer);
    killtic(0);
}
