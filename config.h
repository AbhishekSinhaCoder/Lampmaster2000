/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if lex declares yytext as a char * by default, not a char[].  */
#define YYTEXT_POINTER 1

/* Does your system have the snprintf() call? */
#define HAVE_SNPRINTF 1

/* Does your system have the vsnprintf() call? */
#define HAVE_VSNPRINTF 1

/*
 * The maximum number of simultaneous status, info, and monitor connections
 * the daemon will allow.
 */
#define DAEMON_MONITOR_MAX 16

/*
 * Debugging level.  0 is none.  Valid levels currently are 0-1.
 */
#define DEBUGLVL 1

/*
 * Pid file that the daemon maintains to tell us it is running.
 */
#define DAEMON_PID_FILE "ppowerd.pid"

/*
 * Command socket name.  The user program connects to this to send commands
 * to the daemon.
 */
#define DAEMON_SOCKET_FILE "ppowerd.socket"

/*
 * Monitor socket name.  The user program connects to this to get status,
 * info, and monitor the events coming in from the cm11a.
 */
#define DAEMON_MONITOR_SOCKET_FILE "ppowerd-monitor.socket"

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Name of package */
#define PACKAGE "ppower"

/* Version number of package */
#define VERSION "0.1.5"

