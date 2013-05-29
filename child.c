/* ==================================================================
 *	CHILD.C
 *	-------
 *
 *	This module is responsible for running 
 *	efingerd as a child service of inetd.
 *	It cares for this child like a good parent should,
 *	providing vital nutrients, love, support, child
 *	reapers, and built in functionality to have the
 *	child kill itself if it stays alive too long.
 *
 * ================================================================== 
 */


#include "efingerd.h"
#ifdef DONT_HAVE_LIBIDENT
# define ident_id(a,b) NULL
#else
# include "ident.h"
#endif

#include <pwd.h>
#include <sys/stat.h>

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
    struct hostent *he;

    if (resolve_addr) {
	he = gethostbyaddr((char *) &in, sizeof(struct in_addr), AF_INET);
	if (he == NULL)
	    strncpy(addr, inet_ntoa(in), sizeof(addr));
	else
	    strncpy(addr, he->h_name, sizeof(addr));
    } else
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
void killtic(int s)
{
    killsock(s_in);
    killsock(s_out);
    exit(0);
}


void alm(int i)
{
    i++;
    signal(SIGINT, SIG_IGN);	/* Don't want to kill ourselves */
    kill(-getpgrp(), SIGINT);
    sleep(TIME_UNTIL_KILL);	/* 3 second for the process to terminate itself */
    kill(-getpgrp(), SIGKILL);
    exit(0);
}


void safe_exec(char *cmd, char *arg1, char *arg2, char *arg3)
{
    int pid;
    if ((pid = fork()) == 0) {	/* Program inherits the socket */
	execl(cmd, cmd, arg1, arg2, arg3, NULL);
	_exit(0);		/* Should never happen */
    }
    signal(SIGALRM, alm);
    alarm(TIME_UNTIL_INT);
    wait(NULL);
}

void do_finger(char *user, char *identity, char *remote_address,
	       int sd_out)
{

    struct passwd *passs;
    struct stat st;

    char buff[200], path[200];
    char *poi;

    if (strlen(user) == 0) {
	safe_exec(EFINGER_LIST, identity, remote_address, NULL);

    } else {
	passs = getpwnam(user);
	if (passs == NULL) {
	    safe_exec(EFINGER_NOUSER, identity, remote_address, user);

	} else {
	    if (display_full_name) {
		strncpy(buff, passs->pw_gecos, sizeof(buff) - 1);
		buff[sizeof(buff) - 1] = '\0';
		for (poi = buff; (*poi) != '\0'; poi++)
		    if ((*poi) == ',')
			(*poi) = ' ';

		client_reply(sd_out, buff);
		client_reply(sd_out, "\r\n");
	    };
	    if (sizeof(path) >=
		strlen(passs->pw_dir) + sizeof(EFINGER_USER_FILE) + 1) {
		strncpy(path, passs->pw_dir, sizeof(path));
                strcat(path, "/");
		strcat(path, EFINGER_USER_FILE);
		if (ignore_user || stat(path, &st)) {
		    safe_exec(EFINGER_LUSER, identity, remote_address,
			      user);

		} else {
		    safe_exec(path, identity, remote_address, user);
		}
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
void inetd_service(int sd_in, int sd_out)
{
    struct in_addr laddr, raddr;
    struct sockaddr_in sin;
    char buffer[MAX_SOCK_LENGTH];
    int sinsize = sizeof(struct sockaddr_in);
    int reqstat;
    char *identity, *remote_address;
    char *nullident = "(null)";

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
    laddr = sin.sin_addr;

    reqstat = get_request(sd_in, buffer, MAX_SOCK_LENGTH);
    identity = ident_id(sd_in, IDENT_TIME);
    if (identity == NULL)
	identity = nullident;
    remote_address = lookup_addr(raddr);
    do_finger(buffer, identity, remote_address, sd_out);

}
