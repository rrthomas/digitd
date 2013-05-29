#define MAX_SOCK_LENGTH		100

#define EFINGER_LIST		"/etc/efingerd/list"
#define EFINGER_LUSER		"/etc/efingerd/luser"
#define EFINGER_NOUSER		"/etc/efingerd/nouser"

#define EFINGER_USER_FILE	".efingerd"


/* maximum time we should wait for ident reply */
#define IDENT_TIME	20

/* how much time we give to user processes */
#define TIME_UNTIL_INT		10

/* how much time we give to users processes if they do not react to INT 
 * signal */
#define TIME_UNTIL_KILL		3

