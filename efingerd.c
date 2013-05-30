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
#include <syslog.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define EFINGER_LIST		"/etc/efingerd/list"
#define EFINGER_LUSER		"/etc/efingerd/luser"
#define EFINGER_NOUSER		"/etc/efingerd/nouser"
#define EFINGER_USER_FILE	".finger"


/* Shut down the process (SIGALRM handler) */
static void die(int s)
{
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    closelog();
    exit(0);
}

static void run(const char *cmd, char *arg1)
{
    if (fork() == 0) {	/* Program inherits the socket */
	execl(cmd, cmd, arg1, NULL);
	abort(); /* Should never happen */
    }
    wait(NULL);
}

static void finger(char *user)
{
    const char *prog = EFINGER_NOUSER;
    if (strlen(user) == 0)
	prog = EFINGER_LIST;
    else {
	struct passwd *pwd = getpwnam(user);
	if (pwd != NULL) {
	    char *path;
	    asprintf(&path, "%s/%s", pwd->pw_dir, EFINGER_USER_FILE);
	    struct stat st;
	    if (stat(path, &st) == 0)
		prog = EFINGER_LUSER;
	    free(path);
	}
    }
    run(prog, user);
}

int main(int argc, char *argv[])
{
    if (isatty(STDIN_FILENO)) {
	fprintf(stderr, "efingerd version %s\nNot for interactive use.\n", ID_VERSION);
	exit(0);
    }

    openlog("efingerd", LOG_PID, LOG_DAEMON);
    signal(SIGXCPU, die);
    char user[100];
    fgets(user, sizeof(user), stdin);
    size_t len = strlen(user);
    if (user[len - 1] == '\r') /* Can't assume this, some buggy clients send only \n */
	user[len - 1] = '\0';
    finger(user);
    die(0);
}
