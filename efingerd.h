/* ===================================================================
 * 	efingerd.h
 *
 *
 * ================================================================ */

#ifndef EFINGER_H_
#define EFINGER_H_

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
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <utmp.h>
#include <unistd.h>


#include "define.h"

/* ==================================================================
 * GLOBALS
 * ==================================================================
 */
/* SERVICE BEHAVIOR     */
unsigned char   resolve_addr;		/* reverse lookup addresses	*/
unsigned char   ignore_user;		/* ignore users' .efingerd file	*/
unsigned short	client_timeout;		/* number of seconds till disconnect */
/* ==================================================================
 * PROTOTYPES:
 * ==================================================================
 */

/* #### EFINGER.C #### */
void    killsock (int);
int     get_ports (char[], u_short *, u_short *);
void    client_reply (int, char *);
int	get_request (int d, char buffer[], u_short len);


/* #### CHILD.C #### */
extern void killtic (int);
extern void inetd_child (int, int);
void inetd_service (int sd_in, int sd_out);


#endif /* WHOLE FILE */
