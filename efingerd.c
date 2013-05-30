/* 
 * ================================================================
 * Please view:
 *
 * 	README for program information.
 * 	COPYING for distribution information.
 *
 * 	based on ident2 by Michael Bacarella (defile@nyct.net)
 *
 * ================================================================ 
 */
#include "efingerd.h"

const unsigned short client_timeout = 60;


/* ------------------------------------------------------------------
 * killsock:
 *	violently kills a socket
 * ------------------------------------------------------------------
 */
void killsock (int s)
{
	shutdown (s, 2);
	close (s);
}



/* ------------------------------------------------------------------
 * get_request	:	a fgets for file descriptors 
 * ------------------------------------------------------------------
 */
int get_request (int d, char buffer[], u_short len)
{
	u_short i;
	char ch;
 
	memset (buffer, 0, len);
	for (i = 0; i < len; i++) {
		if (read (d, &ch, 1) != 1)
			return -1; 
		else if (ch == '\n') /* some buggy clients send only \n */
			break;
		else if (ch == '\r') {
			read (d, &ch, 1); /* should read the following \n */
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
void client_reply (int sd, char *outcome)
{
	write (sd, outcome, strlen (outcome));
}



/* ============================================================================
	LOCAL FUNCTIONS
============================================================================ */


/* usage:
 * ------------------------------------------------------------------
 */	
static void	usage (char *progname)
{
	 fprintf (stderr, "usage: %s [options]\n"
			"   --help     This information.\n"
			"   --version  Print version information and exit.\n"
			"   -t X       Time to keep connection.\n"
			"              ex: -t 25  maintain connections for up to 25 seconds.\n"
			"   -n         Do not lookup addresses, use IP numbers instead.\n",
			progname);
	exit (0);
}



/* ------------------------------------------------------------------
 * print_version:
 *	wouldn't want to disappoint anyone.
 * ------------------------------------------------------------------
 */
static void print_version (void)
{
	fprintf (stderr, "efingerd %s\n", ID_VERSION );
	exit (0);
}



int main (int argc, char *argv[])
{	
	u_short i;
	
	resolve_addr = 1;
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'v':
					print_version ();
					
				case 'n':
					resolve_addr = 0;
					break;
                
				case 'h':
					usage (argv[0]);
                	
				case '-':

					if (strncmp ("version", ( argv[i]+2), 7) == 0)
						print_version ();

					else if (strcmp ("help", ( argv[i]+2)) == 0)
						usage (argv[0]);
						
					break;
			}
		}
	}
	
	openlog ("efingerd", LOG_PID, LOG_DAEMON);
	alarm (client_timeout);
	signal (SIGALRM, killtic);
    	
	inetd_service (0, 1);
    
	killsock (0);
	killsock (1);
	closelog ();
	exit (-1);
}
