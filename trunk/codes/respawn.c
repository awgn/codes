/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

extern char *__progname; 

const int log_facility = LOG_LOCAL6;

static const char usage_str[]=
        "Usage: %s [OPTIONS] -- argv[0] argv[1]...\n\n"
        "   -s sec           sec. to sleep before respawning.          (def=1)\n"
		"   -r \"command\"     auxiliary command to run at each respawn. (def none)\n"
        "\n exit behaviour:\n"
        "   -t max           max. consecutive failures before quitting.(def=0, unlimited)\n"
        "   -T max           max. total failures before quitting.      (def=0, unlimited)\n"
        "   -e               quit when child exits successfully(0).    (def respawn)\n"
		"   -q               quit when child exits with any code (useful with -x/-k options)\n"
        "\n signals:\n"
        "   -k SIG -k SIG..  list of signals to redirect to the child. (def none)\n" 
        "   -x SIG -x SIG..  list of signals that kill the child, \n"
        "                    not triggering the respawn                (def none)\n" 
        "\n others:\n"
		"   -d               run as daemon.\n"
        "     -p             changes the working directory of child to the root (\"/\")\n"
        "     -c             redirect std input, output and error of child to /dev/null.\n"
        "   -h               print this help\n";

typedef struct { 
    const char *name;
    int val;
    int exit;
    int forward;
} sigtype;


#define _sig(x,e,f) [x]={ #x, x, e, f } 
static sigtype signal_list[]= {
    _sig(SIGHUP,    0, 0),
    _sig(SIGINT,    0, 0),
    _sig(SIGQUIT,   0, 0),
    _sig(SIGILL,    0, 0),
    _sig(SIGTRAP,   0, 0),
    _sig(SIGABRT,   0, 0),
    // _sig(SIGIOT,    0, 0),
    _sig(SIGBUS,    0, 0),
    _sig(SIGFPE,    0, 0),
    _sig(SIGKILL,   0, 0),
    _sig(SIGUSR1,   0, 0),
    _sig(SIGSEGV,   0, 0),
    _sig(SIGUSR2,   0, 0),
    _sig(SIGPIPE,   0, 0),
    _sig(SIGALRM,   0, 0),
    _sig(SIGTERM,   0, 0),
    _sig(SIGSTKFLT, 0, 0),
    _sig(SIGCLD,    0, 0),
    _sig(SIGCHLD,   0, 0),
    _sig(SIGCONT,   0, 0),
    _sig(SIGSTOP,   0, 0),
    _sig(SIGTSTP,   0, 0),
    _sig(SIGTTIN,   0, 0),
    _sig(SIGTTOU,   0, 0),
    _sig(SIGURG,    0, 0),
    _sig(SIGXCPU,   0, 0),
    _sig(SIGXFSZ,   0, 0),
    _sig(SIGVTALRM, 0, 0),
    _sig(SIGPROF,   0, 0),
    _sig(SIGWINCH,  0, 0),
    _sig(SIGPOLL,   0, 0),
    _sig(SIGIO,     0, 0),
    _sig(SIGPWR,    0, 0),
    _sig(SIGSYS,    0, 0),
    _sig(SIGUNUSED, 0, 0),
};

int   res_sec = 1;	/* sec before respawning */
int   res_cf;		/* consecutive failures  */
int   res_tf;		/* total failures */
int   res_ex; 		/* terminate when child exits succefully */
int   res_ex_any;   /* terminate when child exits with any code */
int   res_chdir;
int   res_redirect;
int   res_daemon;
int   res_ksignal = SIGTERM;
char *res_child;
char *res_helper;

int  pid;
char file_pid[256] = "/var/run/";

#define log(arg...)	do { 				\
	if (res_daemon)					\
		syslog(log_facility|LOG_INFO,## arg);	\
	else						\
		fprintf(stderr,## arg); fputc('\n',stderr);	\
} while (0)


void usage()
{
	printf(usage_str,__progname);	
	exit(0);
}

void exit_rt(int i)
{
	log("*** killing %s(%d) (request by user)", res_child , pid);

        if ( kill(pid,res_ksignal) == -1 && errno == ESRCH)
                log("the pid does not exist (zombie?!?)");

	log("%s exits (goodbye)", __progname); 
	exit (0);
}

void forward_sig(int i)
{
        log("*** catched signal %d(%s)!",i,signal_list[i].name);

        if (!signal_list[i].forward)
                return;
        if (kill(pid,i) == -1 && errno == ESRCH)
                log("the pid does not exist (zombie?!?)");
        else
                log("%s forwarded to child...", signal_list[i].name);
}

int getsignum(const char *sig) {
    int ret = -1, i;

    ret = atoi(sig);
    if (ret!=0)
        return ret;
    ret=-1;

    for (i=0; i < sizeof(signal_list)/sizeof(signal_list[0]); i++) {
        if (signal_list[i].name == NULL)
            continue;
        if (!strcmp(sig,signal_list[i].name)) {
            ret = signal_list[i].val;
            break;
        }
    }

    return ret;
}


int save_pid(const char *filename) {
    FILE *f;

    /* save the pid */
    f = fopen(filename, "wx");
    if (f == NULL) {
        log("%s exist (daemon is running, or stale pidfile to be removed)", filename);
        return -1;
    }

    fprintf(f,"%d",getpid());    
    fclose (f);
   return 0;
}


void respawn(int argc, char *argv[], char *envp[])
{
    int status;
    int t=0, cf=0, tf=0;

    for(;; sleep(res_sec),t++) {

        pid = fork();
        if (pid==-1)
            err(1,"fork");

        if (pid==0) {
            if (execve (argv[0],argv,envp) == -1) {
                warn("execve(%s,...",argv[0]);
                exit(-1);	
            }
        }

        log("process %d created.", pid);
        waitpid(pid,&status,0);	

        if (WIFEXITED(status)) {
            log("*** Child exited with: %d", WEXITSTATUS(status));
			if (res_ex_any) {
				log("child-exit-quit; bye.");
				exit(0);
			}
        }

        if (WIFEXITED(status) && WEXITSTATUS(status)==0) {
            cf = 0;
            if (res_ex) {
                log("child-exit-status=0; bye.");
                exit(0);
            }
            goto restart;
        }
        else {
            cf++; tf++;
        }	

        if (WIFSIGNALED(status)) {

            log("Child killed by signal %d", WTERMSIG(status));

            if (WTERMSIG(status) == SIGRTMIN || signal_list[WTERMSIG(status)].exit) {
                log("%s exits (goodbye)", __progname); 
                exit(0);
            }

        }

        if (res_tf && (res_tf < tf)) {
            log("Max. total failures reached; exit forced.");
            exit(1);
        }

        if (res_cf && (res_cf < cf)) {
            log("Max. consec. failures reached; exit forced.");		
            exit(2);
        }

restart:

        if (res_helper) 
            system(res_helper);

        log("#%d/%d/%d: Running %s in %d sec...",cf,tf,t,argv[0],res_sec);

    }

}

void thefunctionafter() __attribute__((destructor));
void
thefunctionafter()
{
	unlink(file_pid);	
}

int 
main(int argc, char *argv[], char *envp[])
{
    int i;

    while( (i=getopt(argc, argv, "s:t:T:r:k:x:eqdpch"))!= EOF)	
        switch(i) {
            case 's': res_sec = atoi(optarg); break;
            case 't': res_cf  = atoi(optarg); break;
            case 'T': res_tf  = atoi(optarg); break;
            case 'e': res_ex  = 1; break;
		    case 'q': res_ex_any = 1; res_ex = 1; break;
            case 'd': res_daemon = 1; break;
            case 'r': res_helper = optarg; break;
            case 'p': res_chdir = 1; break;
            case 'c': res_redirect = 1; break;
            case 'k': {
                int sig = getsignum(optarg);              
                if (sig == -1)
                        errx(1,"unknown signal %s", optarg);
                signal_list[sig].forward = 1; 
                break;
            }
            case 'x': {
                res_ksignal = getsignum(optarg);
                if (res_ksignal == -1)
                         errx(1,"unknown signal %s", optarg);
                signal_list[res_ksignal].exit = 1;
                signal_list[res_ksignal].forward = 1; 
                break;               
            }
            case 'h': usage();
        }

    argc -= optind;
    argv += optind;

    if ( argc < 1 )
        usage();

    if (!res_daemon && res_chdir) {
        errx(2, "-p is valid with -d option");
    }
    if (!res_daemon && res_redirect) {
        errx(3, "-c is valid with -d option");
    }

    res_child = argv[0];

    signal(SIGHUP,    forward_sig);
    signal(SIGINT,    exit_rt);             // convenience ctrl+c
    signal(SIGQUIT,   forward_sig);
    signal(SIGILL,    forward_sig);
    signal(SIGABRT,   forward_sig);
    signal(SIGFPE,    forward_sig);
    // signal(SIGKILL,   forward_sig);   cannot be caught
    signal(SIGSEGV,   forward_sig);
    signal(SIGPIPE,   forward_sig);
    signal(SIGALRM,   forward_sig);
    signal(SIGTERM,   forward_sig);
    signal(SIGUSR1,   forward_sig);
    signal(SIGUSR2,   forward_sig);
    // signal(SIGCHLD,   forward_sig);	do not catch this
    signal(SIGCONT,   forward_sig);
    // signal(SIGSTOP,   forward_sig);   cannot be caught
    signal(SIGTSTP,   forward_sig);
    signal(SIGTTIN,   forward_sig);
    signal(SIGTTOU,   forward_sig);
    signal(SIGBUS,    forward_sig);
    signal(SIGPOLL,   forward_sig);
    signal(SIGPROF,   forward_sig);
    signal(SIGSYS,    forward_sig);
    signal(SIGTRAP,   forward_sig);
    signal(SIGURG,    forward_sig);
    signal(SIGVTALRM, forward_sig);
    signal(SIGXCPU,   forward_sig);
    signal(SIGXFSZ,   forward_sig);
    signal(SIGIOT,    forward_sig);
    signal(SIGSTKFLT, forward_sig);
    signal(SIGIO,     forward_sig);
    // signal(SIGCLD,    forward_sig);	do not catch this (SIGCHLD)
    signal(SIGPWR,    forward_sig);
    signal(SIGWINCH,  forward_sig);

    signal(SIGRTMIN,  exit_rt);

    openlog(__progname, LOG_CONS|LOG_NDELAY, log_facility);

    fprintf(stderr,"respawn (%s):\n", argv[0]);
    fprintf(stderr,"    resp_sec   : %d\n", res_sec);
    fprintf(stderr,"    consc_fails: %d\n", res_cf);
    fprintf(stderr,"    total_fails: %d\n", res_tf);
    fprintf(stderr,"    resp_helper: %s\n", res_helper);
    fprintf(stderr,"    file_pid   : %s\n", file_pid);
    fprintf(stderr,"    sigkill    : %d\n", res_ksignal);
    if (res_chdir)
        fprintf(stderr,"    chdir to \"/\" on \n"); 
    if (res_redirect)
        fprintf(stderr,"    redirect std* to /dev/null\n"); 

    if (res_daemon) {
        char *r;
        daemon(!res_chdir,!res_redirect);

        r = strrchr(argv[0],'/');
        strcat(file_pid, r ? r+1 : argv[0]);
        strcat(file_pid,".rpid");
        if ( save_pid(file_pid) != 0 )
            abort();
    }

    respawn(argc, argv, envp);

    return 0;
}	
