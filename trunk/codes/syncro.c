/*
 * $Id: syncro.c,v 1.25 2005/10/10 11:00:41 awgn Exp $
 *
 * Copyright (c) 2005 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 * 
 * Compile under Linux 2.4/2.6 on the top of i386 processors.
 * 
 * Linux Kernel 2.4:
 * gcc -g -O2 -o syncro  -D_GNU_SOURCE -Wall -Wp,-Wunused-macros syncro.c
 *
 * Linux Kernel 2.6: 
 * gcc -g -O2 -o syncro  -D_GNU_SOURCE -DKERNEL_2_6 -Wall -Wp,-Wunused-macros syncro.c
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer. 2.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */

#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/time.h>
#include <asm/ptrace.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <sched.h>
#include <netdb.h>

#include <time.h>

#include <sysexits.h>
#include <errno.h>
#include <err.h>


/************************************************
 *               enum & typedefs                *
 ************************************************/


enum trace_mode_e {     

        trace_null=0,
        trace_syscall,
        trace_address 
};

enum trigg_mode_e {     

        trigg_null=0,
        trigg_time,
        trigg_net       
};

enum arg_type {

        tk_host=0,
        tk_port,
        tk_time,
        tk_delay,
        tk_now,
        tk_rt,
        tk_dr,
        tk_addr,
        tk_sys,
        tk_delim,
};

typedef unsigned long addr_t;

typedef void (*sighandler_t)(int);

/*
 * 32 bit FNV-0 hash type
 */
typedef unsigned long Fnv32_t;

#define FNV1_32_INIT ((Fnv32_t)0x811c9dc5)

/************************************************
 *                 structures                   *
 ************************************************/


struct __option {
        struct timeval	   trigger;
        char *		   script;
        char *             password;
        char *             symbol;
        long               address;
        int                syscall;
        int		   rt_lifetime;
        int		   dry_run;
        int		   family;
        int		   daemon;
        enum trace_mode_e  trace_mode;
        enum trigg_mode_e  trigg_mode;

        int		   msg_len;	/* received net_msg */
        int		   ntoken;	/* received net_msg */
        int		   fnv;		/* received net_msg */
}; 


/************************************************

    message header:

    +-----------+-----------+
    |  address  |  syscall  |  
    +-----------+-----------+
    | RT_lifeT  |  dry-run  |  
    +-----------+-----------+
    | trace_md  | trigg_md  |  
    +-----------+-----------+
    |  tv_sec   |  tv_usec  | <- trigger time 
    +-----------+-----------+
    |  msg_len  |    fnv    |  
    +-----------+-----------+---------+
 1  |   len     |    id[]             | ...
    +-----------+-----------+-----+---+
 2  |   len     |    id[]         | ...
    +-----------+-----------+-----+
 3  |   len     |    id[]   | ... 
    +-----------+-----------+


************************************************/

struct hdr {
        long               address;             /* breakpoint address */
        int                syscall;             /* syscall index */
        int                rt_lifetime;         /* RT lifetime sec. */
        int                dry_run;             /* dry-run sec. */
        enum trace_mode_e  trace_mode;          /* trace mode */
        enum trigg_mode_e  trigg_mode;          /* trigger mode */
        struct timeval     trigger;
        int                msg_len;             /* msg length */
        int                ntoken;              /* n. of tokens */
        int                fnv;                 /* [hdr][pass] -> HASH */
};


struct net_msg {
        struct hdr      header;
        char            msg[];
}  __attribute__((packed));


struct instance {
        struct net_msg   *packet;
        struct hostent   *h_remote;
        char             *s_remote;
        int              port;		/* host byte order */
        int		 msg_len;	/* host byte order */
        int		 ntoken;	/* host byte order */
};


/************************************************
 *                  macros                      *
 ************************************************/

#define __INTERNAL__    "%s:%d %s() internal error",\
        __FILE__,__LINE__,__FUNCTION__

#ifndef __unused
#define __unused        __attribute__((unused))
#endif
#ifndef __dead
#define __dead          __attribute__((noreturn))
#endif

#define unless(x,y)     ( (x) ? : (y) )

#define msg(format,...)  fprintf(stdout, format, ## __VA_ARGS__)
#define MSG_NULL        "    "
#define MSG_DIR         "[*] "
#define MSG_INFO        "[i] "
#define MSG_DEBUG	" -  "
#define MSG_WORNGPASS	MSG_INFO "\e[1mwrong password!\e[0m\n"

#define SEP             " ,\t\n"
#define ACK             0x06   		/* ack message */ 
#define CD_RATE         2               /* count down rate (sec) */
#define ATIMEO          5               /* timeout (sec) */
#define INT3            0xcc            /* int3 opcode (IA32) */

#ifdef KERNEL_2_6
#warning : vgettimeofday support enabled. ;-)

#define __kernel_vsyscall       0xffffe400      /* linux kernel 2.6 */
#define vgettimeofday(x) \
        asm volatile ( "call *%3 \n" : :\
                        "a" (0x4e),\
                        "b" ((x)),\
                        "c" (0),\
                        "r" (__kernel_vsyscall): "memory" );

#define vptrace(request,pid,addr,data) \
        asm volatile ( "call *%5 \n" : :\
                        "a" (26),\
                        "b" (request),\
                        "c" (pid),\
                        "d" (addr),\
                        "S" (data), \
                        "r" (__kernel_vsyscall) : "memory" );

#endif

/* macro DEBUG() */
#ifdef EBUG
#define DEBUG(format, ...) do { \
        fprintf (stderr, MSG_DEBUG "%s:", __PRETTY_FUNCTION__); \
        fprintf (stderr, format, ## __VA_ARGS__); \
} while (0)
#else
#define DEBUG(f,arg...) do {} while (0)
#endif


/************************************************
 *                  globals                     *
 ************************************************/


/*
 * cvsid
 */
__unused const char cvsid[] = "$Id: syncro.c,v 1.25 2005/10/10 11:00:41 awgn Exp $";


/* syscall names for i386 from <asm/unistd.h> */

char *(syscall_names[256]) = {
NULL, "exit", "fork", "read", "write", "open", "close", "waitpid", "creat",
"link", "unlink", "execve", "chdir", "time", "mknod", "chmod",
"lchown", "break", "oldstat", "lseek", "getpid", "mount", "umount",
"setuid", "getuid", "stime", "ptrace", "alarm", "oldfstat", "pause",
"utime", "stty", "gtty", "access", "nice", "ftime", "sync", "kill",
"rename", "mkdir", "rmdir", "dup", "pipe", "times", "prof", "brk",
"setgid", "getgid", "signal", "geteuid", "getegid", "acct", "umount2",
"lock", "ioctl", "fcntl", "mpx", "setpgid", "ulimit", "oldolduname",
"umask", "chroot", "ustat", "dup2", "getppid", "getpgrp", "setsid",
"sigaction", "sgetmask", "ssetmask", "setreuid", "setregid",
"sigsuspend", "sigpending", "sethostname", "setrlimit", "getrlimit",
"getrusage", "gettimeofday", "settimeofday", "getgroups", "setgroups",
"select", "symlink", "oldlstat", "readlink", "uselib", "swapon",
"reboot", "readdir", "mmap", "munmap", "truncate", "ftruncate",
"fchmod", "fchown", "getpriority", "setpriority", "profil", "statfs",
"fstatfs", "ioperm", "socketcall", "syslog", "setitimer", "getitimer",
"stat", "lstat", "fstat", "olduname", "iopl", "vhangup", "idle",
"vm86old", "wait4", "swapoff", "sysinfo", "ipc", "fsync", "sigreturn",
"clone", "setdomainname", "uname", "modify_ldt", "adjtimex",
"mprotect", "sigprocmask", "create_module", "init_module",
"delete_module", "get_kernel_syms", "quotactl", "getpgid", "fchdir",
"bdflush", "sysfs", "personality", "afs_syscall", "setfsuid",
"setfsgid", "_llseek", "getdents", "_newselect", "flock", "msync",
"readv", "writev", "getsid", "fdatasync", "_sysctl", "mlock",
"munlock", "mlockall", "munlockall", "sched_setparam",
"sched_getparam", "sched_setscheduler", "sched_getscheduler",
"sched_yield", "sched_get_priority_max", "sched_get_priority_min",
"sched_rr_get_interval", "nanosleep", "mremap", "setresuid",
"getresuid", "vm86", "query_module", "poll", "nfsservctl",
"setresgid", "getresgid", "prctl","rt_sigreturn","rt_sigaction",
"rt_sigprocmask", "rt_sigpending", "rt_sigtimedwait",
"rt_sigqueueinfo", "rt_sigsuspend", "pread", "pwrite", "chown",
"getcwd", "capget", "capset", "sigaltstack", "sendfile", "getpmsg",
"putpmsg", "vfork", "ugetrlimit", "mmap2", "truncate64",
"ftruncate64", "stat64", "lstat64", "fstat64", "lchown32", "getuid32",
"getgid32", "geteuid32", "getegid32", "setreuid32", "setregid32",
"getgroups32", "setgroups32", "fchown32", "setresuid32",
"getresuid32", "setresgid32", "getresgid32", "chown32", "setuid32",
"setgid32", "setfsuid32", "setfsgid32", "pivot_root", "mincore",
"madvise", "getdents64", "fcntl64", 0, "security", "gettid",
"readahead", "setxattr", "lsetxattr", "fsetxattr", "getxattr",
"lgetxattr", "fgetxattr", "listxattr", "llistxattr", "flistxattr",
"removexattr", "lremovexattr", "fremovexattr", "tkill", "sendfile64",
"futex", "sched_setaffinity", "sched_getaffinity",
};


const char usage_s[] = 
#ifdef KERNEL_2_6
"Usage:%s [options] -- command args... \e[5m:-)	   	    \e[0m\n"
#else
"Usage:%s [options] -- command args... 				 \n"
#endif
"   -s syscall           set the breakpoint at system_call        \n\
   -a address           set the breakpoint at address            \n\
   -t %%h:%%m:%%s          trigger command at hh:mm:ss           \n\
   -r sec.              run as realtime (high precision)         \n\
   -d sec.              dry-run. estimate the mean trigger-delay \n\n\
   -w pass              password                                 \n\
   -f file              run script (remote-control)              \n\
   -p port              port to listen on (def. 31337)           \n\
   -x                   run as daemon                            \n\n\
   -h                   print this help.                         \n\
   -l                   print syscall table                      \n";


struct __option options = {
        .family =	AF_INET
};

/* script commands */

struct command {
        char            *id;
        int             arg;
} script_command[]={
        [tk_host]     = { .id="HOST",  .arg= 1 },
        [tk_port]     = { .id="PORT",  .arg= 1 },
        [tk_time]     = { .id="TIME",  .arg= 1 },
        [tk_delay]    = { .id="DELAY", .arg= 1 },
        [tk_now]      = { .id="NOW",   .arg= 0 },
        [tk_rt]       = { .id="RT",    .arg= 1 },
        [tk_dr]       = { .id="DR",    .arg= 1 },
        [tk_addr]     = { .id="ADDR",  .arg= 1 },
        [tk_sys]      = { .id="SYS",   .arg= 1 },
        [tk_delim]    = { .id="--",    .arg= 0 },
};


struct  instance statements[1024];	/* script file staments */
struct  timeval  tv_start;

int     listen_port = 31337;		/* default server port */
int  	pid;				/* process id */
int     sk_fd;				/* socket fd */

char 	brk_prev_opcode;
char    buffer  [1024];
char *  net_argv[256];
int     net_argc;


/************************************************
 *                 functions                    *
 ************************************************/


        Fnv32_t
fnv_32a_buf(void *buf, size_t len, Fnv32_t hval)
{
        unsigned char *bp = (unsigned char *) buf;      /* start of buffer */
        unsigned char *be = bp + len;   /* beyond end of buffer */

        /*
         * FNV-1a hash each octet in the buffer
         */
        while (bp < be) {

                /* xor the bottom with the current octet */
                hval ^= (Fnv32_t) * bp++;

                /* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
                hval *= FNV_32_PRIME;
#else
                hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
        }

        /* return our new hash value */
        return hval;
}


        Fnv32_t
fnv_32a_iovec(const struct iovec * vector, size_t count, Fnv32_t hval)
{
        unsigned char *bp;      /* start of block */
        unsigned char *be;      /* beyond end of buffer */
        int lcount;

        for (lcount = 0; lcount < count; lcount++) {

                if ( vector[lcount].iov_base == NULL || vector[lcount].iov_len == 0 )
                        continue;

                bp = vector[lcount].iov_base;
                be = vector[lcount].iov_base + vector[lcount].iov_len;

                /*
                 * FNV-1a hash each octet in the buffer
                 */
                while (bp < be) {
                        /* xor the bottom with the current octet */
                        hval ^= (Fnv32_t) * bp++;

                        /* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
                        hval *= FNV_32_PRIME;
#else
                        hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
                }

        }

        /* return our new hash value */
        return hval;
}


        Fnv32_t
compute_hash(struct net_msg * buf, char *passwd) 
{
        struct iovec vector[2];
        Fnv32_t ret;

        vector[0].iov_base = buf;
        vector[0].iov_len  = ntohl(buf->header.msg_len);

        vector[1].iov_base = passwd;
        vector[1].iov_len  = ( passwd ? strlen(passwd) : 0 );

        ret = fnv_32a_iovec (vector, 2, FNV1_32_INIT);
        return ret;
}


/* syscall management */
        int
get_syscall_index(char *symbol)
{
        int i;

        if (symbol == NULL)
                return -1;

        for (i=0;i<256;i++) {
                if (syscall_names[i] == NULL)
                        continue;

                if ( strcmp(symbol,syscall_names[i]) == 0 )
                        return i;
        }
        return -1;
}


/* vwarn()/vwarnx() wrapper */
        void
warning(const char *pattern,...)
{
        va_list ap;
        va_start(ap, pattern);

        if (errno)
                vwarn(pattern, ap);
        else
                vwarnx( pattern, ap);

}


/* verr()/verrx() wrapper */
        __dead void
fatal(const char *pattern,...)
{
        va_list ap;

        va_start(ap, pattern);

        if (errno)
                verr(EX_SOFTWARE, pattern, ap);
        else
                verrx(EX_SOFTWARE, pattern, ap);
}

/* usage */
        __dead void
usage()
{
        extern char *__progname;
        printf(usage_s, __progname);
        exit(0);
}


        __dead void
show_systable()
{
        int i;

        msg(MSG_DIR "Linux syscall table:\n");
        for (i=0;i<256;i++) {
                if (syscall_names[i] == NULL)
                        continue;
                msg(MSG_NULL "[%d] %s\n",i,syscall_names[i]);	
        }
        exit(0);
}


/*
 * soft-rt scheduling
 */
        void
set_realtime(int pid, int policy)
{
        struct sched_param sp;

        if (policy != SCHED_RR && policy != SCHED_FIFO )
                fatal(__INTERNAL__);

        memset(&sp, 0, sizeof(sp));
        sp.sched_priority = sched_get_priority_max(policy);
        if (sp.sched_priority == -1)
                fatal(__INTERNAL__);

        if ( sched_setscheduler(pid, policy, &sp) == -1 )
                fatal(__INTERNAL__);

        msg (MSG_NULL "pid:%d rt_sched_priority=%d\n",pid,sp.sched_priority);
}	


/* 
 * timing
 */
        void
wait_until(struct timeval *tv)
{
        struct timeval now;

#if defined(__GXX_ABI_VERSION) && __GXX_ABI_VERSION >= 100  
        __builtin_prefetch(&tv->tv_sec, 0,3);
        __builtin_prefetch(&tv->tv_usec,0,3);
#endif
        do {
#ifdef KERNEL_2_6
                vgettimeofday(&now);
#else
                gettimeofday(&now,NULL);
#endif
        }
        while (  __builtin_expect(now.tv_sec<tv->tv_sec,1)   ||
                        (  now.tv_sec == tv->tv_sec   &&
                           now.tv_usec < tv->tv_usec  ));

}


        void
count_down(struct timeval *tv)
{
        struct timespec ps={0,0};
        struct timeval now;
        int res;

        gettimeofday (&now,NULL);
        res = (int)tv->tv_sec-now.tv_sec;

        while ( res > CD_RATE ) {
                fprintf(stdout,"\e[2K -> %d sec. to run...\r",res); 
                fflush (stdout);

                ps.tv_sec = ( res >= CD_RATE*2 ?  CD_RATE : res % CD_RATE );

                nanosleep(&ps,NULL);
                gettimeofday (&now,NULL);
                res = (int)tv->tv_sec-now.tv_sec;
        }

        fprintf(stdout,"\e[2K*** less than %d sec. to run...\r", CD_RATE);
        fflush (stdout);
}

#if 0
        static int
timeval_diff(struct timeval * new, struct timeval * old) 
{
        int usec;
        usec = (new->tv_sec-old->tv_sec) * 1000000 + (new->tv_usec-old->tv_usec);
        return usec;
}
#endif


/* 
 * signal handlers 
 */
void 
sig_interrupt(int null) {
        msg (MSG_DIR "cltr+c catched.\n");
        if ( ptrace(PTRACE_KILL, pid, 0, 0) == -1 && errno)
                kill (pid, SIGKILL);

        exit(1);
}


void
sig_alarm(int null) {
        msg (MSG_DIR "rt_lifetime expired.\n");
        if ( ptrace(PTRACE_KILL, pid, 0, 0) == -1 && errno)
                kill (pid, SIGKILL);

}


void sig_alarm_null(int dummy) __attribute__((used));
        void
sig_alarm_null(int dummy)
{

}

/********* socket *********/

/* 
 * dup_hostent - create hostent in one memory block (KAME/NETBSD) 
 */

struct hostent *dup_hostent(hp)
        struct hostent *hp;
{
        struct hostent_block {
                struct hostent host;
                char   *addr_list[1];
        };
        struct hostent_block *hb;
        int     count;
        char   *data;
        char   *addr;

        for (count = 0; hp->h_addr_list[count] != 0; count++)
                /* void */ ;

        if ((hb = (struct hostent_block *) malloc(sizeof(struct hostent_block)
                                        + (hp->h_length + sizeof(char *)) * count)) == 0) {
                fatal("dup_hostent()");
        }
        memset((char *) &hb->host, 0, sizeof(hb->host));

        hb->host.h_addrtype = hp->h_addrtype;
        hb->host.h_length = hp->h_length;
        hb->host.h_addr_list = hb->addr_list;
        hb->host.h_addr_list[count] = 0;
        data = (char *) (hb->host.h_addr_list + count + 1);

        for (count = 0; (addr = hp->h_addr_list[count]) != 0; count++) {
                hb->host.h_addr_list[count] = data + hp->h_length * count;
                memcpy(hb->host.h_addr_list[count], addr, hp->h_length);
        }
        return &hb->host;
}


/********* parse atoms *********/

        struct hostent *
p_gethostbyname(const char *s)
{
        struct hostent *h;

        printf(MSG_INFO "\e[2Kgethostbyname(\"%s\")...\r",s);

        h = unless ( gethostbyname2(s,PF_INET), gethostbyname2(s,PF_INET6) );

        if (h == NULL)
                fatal("\"%s\": h_errno(%d): %s!?",s,h_errno,hstrerror(h_errno));

        return dup_hostent(h);
}


        int
p_integer(char *a, int base)
{
        char *endptr;
        int ret;

        if ( base == 16 ) {
                ret = strtol (a, &endptr, 10);
                if (*endptr == '\0')
                        fatal("strtol(): 0x...");
        }

        ret= strtol (a, &endptr, base);
        if (*endptr != '\0')
                fatal("strtol()");

        return ret;
}


        int
p_timeval (struct timeval *tv, const char *str)
{
        struct tm tm_now,tm_fut;
        time_t now, fut;

        now = time(NULL);

        localtime_r(&now,&tm_now);
        localtime_r(&now,&tm_fut);

        if ( strptime(str,"%T",&tm_fut) == NULL ) {
                fatal("strptime()");
        }

        fut = mktime (&tm_fut);

        tv->tv_sec  = fut;
        tv->tv_usec = 0;

        if ( fut-now <= 0) {
                fatal("[%s] is not a future event!",str);
        }

        return 0;
}


        int
get_token_type(char *c)
{
        int i;

        if ( c == NULL )
                return -1;

        for (i=0; i < sizeof(script_command)/sizeof(script_command[0]) ; i++ ) {
                if ( strcmp(c,script_command[i].id) == 0 )
                        return i;
        }

        return -1;
}


        void
check_statement(char *file,struct instance *s, int i)
{
        /* hostent */

        if ( s->h_remote == NULL )
                fatal("%s:%d HOST missing!",file,i);

        if ( s->port == 0 )
                fatal("%s:%d PORT missing!",file,i);

        if ( s->packet->header.address == 0 && s->packet->header.syscall == 0)
                fatal("%s:%d ADDR or SYS required!",file,i);
}


        int
core_parser(char *script)
{
        FILE *s;
        char *lp, *tk;
        int i, j;
        size_t ln;

        if (script == NULL)
                return -1;

        msg (MSG_DIR "running \"%s\"\n",script);

        s = fopen(script,"r");
        if (s == NULL)
                fatal("can't open %s",script);

        i  = 1, j = 0, ln = 0, lp = NULL;

        for (;getline (&lp,&ln,s) != -1;i++) {

                tk = strtok(lp, SEP);
                /* skip blank and comment lines */
                if (tk == NULL || tk[0] == '\0' || tk[0] == '#' || tk[0] == '\n' )
                        continue;

                statements[j].packet = (struct net_msg *)malloc(sizeof(struct hdr)+1);
                memset(statements[j].packet,0, sizeof(struct hdr)+1);

                /* process options */
                do {
                        int c;
                        char *arg = NULL;

                        c   = get_token_type(tk);
                        if ( c == -1 )
                                fatal("\"%s\": unknown statement!?",tk);

                        if ( script_command[c].arg != 0  )
                                arg = strtok(NULL,SEP);

                        switch (c) {
                                case tk_host:
                                        statements[j].s_remote=(char *)strdup(arg);
                                        statements[j].h_remote=p_gethostbyname(arg);                      
                                        break;
                                case tk_port: {
                                                      int port;
                                                      port = p_integer(arg,10);
                                                      if ( port < 0 || port > 65535 )
                                                              fatal("<%s:%s> wrong port?!",tk,arg);
                                                      statements[j].port =  port;
                                              }       break;
                                case tk_time: {
                                                      struct timeval tv;
                                                      p_timeval(&tv, arg);
                                                      statements[j].packet->header.trigger.tv_sec = 
                                                              htonl(tv.tv_sec);
                                                      statements[j].packet->header.trigger.tv_usec = 
                                                              htonl(tv.tv_usec);
                                              }       break;
                                case tk_delay: 
                                              statements[j].packet->header.trigger.tv_sec  = 
                                                      htonl( tv_start.tv_sec+p_integer(arg,10) );
                                              statements[j].packet->header.trigger.tv_usec = 
                                                      htonl( tv_start.tv_usec );
                                              break;
                                case tk_now:
                                              statements[j].packet->header.trigger.tv_sec  = htonl(0);
                                              statements[j].packet->header.trigger.tv_usec = htonl(0);
                                              break;
                                case tk_rt:
                                              statements[j].packet->header.rt_lifetime = 
                                                      htonl(p_integer(arg,10));
                                              break;
                                case tk_dr:
                                              statements[j].packet->header.dry_run = 
                                                      htonl(p_integer(arg,10));
                                              break;
                                case tk_addr:
                                              statements[j].packet->header.address = 
                                                      htonl(p_integer(arg,16));
                                              statements[j].packet->header.trace_mode = 
                                                      htonl(trace_address);
                                              break;
                                case tk_sys:
                                              statements[j].packet->header.syscall = 
                                                      htonl(get_syscall_index(arg));
                                              statements[j].packet->header.trace_mode = 
                                                      htonl(trace_syscall);
                                              break;
                                case tk_delim:
                                              goto parse_tokens;
                        }

                        DEBUG("<%s:%s>\n", tk,arg);
                }
                while ( (tk = strtok(NULL,SEP)) != NULL );

                fatal ("\n%d: separator -- is missing",i);

parse_tokens:
                statements[j].ntoken = 0;
                statements[j].msg_len= sizeof(struct hdr);

                check_statement(script,&statements[j],i);

                /* process tokens */
                while ( (tk = strtok(NULL, SEP)) != NULL ) {

                        DEBUG("[%s]\n",tk);

                        statements[j].ntoken++;
                        statements[j].msg_len+= strlen(tk)+2;	// "token \0"

                        statements[j].packet = (struct net_msg *)
                                realloc(statements[j].packet, statements[j].msg_len);
                        strcat(statements[j].packet->msg, tk);
                        strcat(statements[j].packet->msg, " ");

                }
                statements[j].packet->header.ntoken  = htonl(statements[j].ntoken);
                statements[j].packet->header.msg_len = htonl(statements[j].msg_len);		

                j++;
                DEBUG("---\n");
        }

        fclose(s);
        return 0;
}


/*
 * ensure all of data on socket comes through.  
 */

        ssize_t
atomicio(f, fd, _s, n)
        ssize_t(*f) ();
        int fd;
        void *_s;
        size_t n;
{
        char *s = _s;
        ssize_t res, pos = 0;

        while (n > pos) {
                res = (f) (fd, s + pos, n - pos);
                switch (res) {
                        case -1:
                                if (errno == EINTR || errno == EAGAIN)
                                        continue;
                                else
                                        fatal("atomicio()");
                        case 0:
                                return res;
                        default:
                                pos += res;
                }
        }
        return pos;
}


        int
send_ack(int fd)
{
        char ack = ACK;
        int r;

        if ( (r=atomicio(write,fd,&ack,sizeof(char))) < sizeof(char) )
                warning("send_ack()");

        return r;
}


        int
recv_ack(int fd)
{
        char buff='\0';

        alarm(ATIMEO);
        if ( read(fd,&buff,sizeof(char)) < sizeof(char) )
                warning("read_ack()");
        alarm(0);

        if ( buff == ACK )
                return 0;

        return -1;
}


        int
send_statement ()
{
        struct sockaddr_storage addr;
        struct sockaddr_in *in;
        struct sockaddr_in6 *in6;
        int i,s,c;

        for ( i = 0 ; statements[i].packet != NULL; i++ ) {

                printf(MSG_NULL "[%s]: connecting...\r", statements[i].s_remote );
                fflush(stdout);

                s = socket ( statements[i].h_remote->h_addrtype , SOCK_STREAM, 0);
                if ( s == -1)
                        fatal("socket()");

                switch(  statements[i].h_remote->h_addrtype ) {
                        case PF_INET: {
                                              in = (struct sockaddr_in *)&addr;
                                              in->sin_family = PF_INET;
                                              in->sin_port   = htons (statements[i].port);
                                              memcpy( &in->sin_addr.s_addr,
                                                              statements[i].h_remote->h_addr,
                                                              statements[i].h_remote->h_length);
                                      } break;

                        case PF_INET6: {
                                               in6 = (struct sockaddr_in6 *)&addr;
                                               in6->sin6_family = PF_INET6;
                                               in6->sin6_port   = htons(statements[i].port);
                                               memcpy( &in6->sin6_addr.s6_addr,
                                                               statements[i].h_remote->h_addr,
                                                               statements[i].h_remote->h_length);
                                       } break;
                }

                alarm(ATIMEO);
                c = connect(s, (struct sockaddr *)&addr,
                                statements[i].h_remote->h_addrtype == PF_INET ?
                                sizeof(struct sockaddr_in) :
                                sizeof(struct sockaddr_in6));
                alarm(0);

                if ( c == -1 ) {
                        printf(MSG_NULL "\e[2K<%s|%d>: %s\r",statements[i].s_remote,
                                        statements[i].port, strerror(errno));
                        goto next;
                }

                printf(MSG_NULL "\e[2K<%s|%d>: sending msg...\r",statements[i].s_remote,
                                statements[i].port);
                fflush(stdout);

                statements[i].packet->header.fnv = 0;
                statements[i].packet->header.fnv = 
                        htonl(compute_hash(statements[i].packet, options.password)); 

                atomicio(write,s,&statements[i].packet->header, statements[i].msg_len);

                printf(MSG_NULL "\e[2K<%s|%d>: waiting for ack...\r",statements[i].s_remote,
                                statements[i].port);
                fflush(stdout);

                if ( recv_ack(s) == -1 ) {
                        printf(MSG_NULL "\e[2K<%s|%d>: recv() Ack timeout\r",statements[i].s_remote,
                                        statements[i].port);
                        goto next;
                }

                printf(MSG_NULL "\e[2K<%s|%d>: \e[1m%s\e[0m\n",statements[i].s_remote, 
                                statements[i].port,
                                statements[i].packet->msg);
                while ( (c=read(s,&buffer,1024)) > 0 )
                        write(fileno(stdout), buffer, c);
next:
                putchar('\n');
                close (s);
        }

        return 0;
}


        sighandler_t
_signal(int signum, sighandler_t handler)
{
        struct sigaction act;

        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        if (sigaction(signum, &act, NULL) < 0)
                return SIG_ERR;
        return act.sa_handler;
}


        struct net_msg *
recv_message(int fd)
{
        struct net_msg *msg;
        int msg_len;

        msg = (struct net_msg *)malloc(sizeof(struct net_msg));
        if (msg == NULL)
                fatal ("malloc()");

        if ( atomicio(read,fd,&msg->header,sizeof(struct net_msg)) < sizeof(struct net_msg) )
                goto error;

        msg_len = ntohl(msg->header.msg_len);
        if ( msg_len == sizeof(struct net_msg) )
                goto fine;

        msg = (struct net_msg *)realloc(msg, msg_len);
        if (msg == NULL)
                fatal ("malloc()");

        msg(MSG_INFO "msg_len: %d bytes\n",msg_len);

        msg_len -= sizeof(struct net_msg);
        if ( atomicio(read,fd, &msg->msg, msg_len) < msg_len ) {
                goto error;
        }

fine:
        return msg;
error:
        free(msg);
        return NULL;

}

#define store_option(x)	(options.x=ntohl(m->header.x))
        int
parse_message(struct net_msg *m)
{
        Fnv32_t h; 
        char *r, *msg=(char *)m->msg;
        int i;

        store_option(address);
        store_option(syscall);
        store_option(rt_lifetime);
        store_option(dry_run);

        store_option(trace_mode);
        store_option(trigg_mode);

        store_option(trigger.tv_sec);
        store_option(trigger.tv_usec);

        store_option(msg_len);
        store_option(ntoken);
        store_option(fnv);

        m->header.fnv = 0;
        h = compute_hash(m, options.password);

        msg(MSG_NULL "address: 0x%x\n",	  (unsigned int)options.address);
        msg(MSG_NULL "syscall: %s[%d]\n", syscall_names[options.syscall],options.syscall);
        msg(MSG_NULL "rt-life: %d\n",     options.rt_lifetime);
        msg(MSG_NULL "dry-run: %d\n",     options.dry_run);

        msg(MSG_NULL "trace_m: %d\n",  	  options.trace_mode);
        msg(MSG_NULL "trigg_m: %d\n",     options.trigg_mode);
        msg(MSG_NULL "trigger:  {.tv_sec=%d, .tv_usec=%d}\n",(int)options.trigger.tv_sec,
                        (int)options.trigger.tv_usec);
        msg(MSG_NULL "msg_len: %d\n",  options.msg_len);
        msg(MSG_NULL "ntoken:  %d\n",  options.ntoken);
        msg(MSG_NULL "hash:    0x%x\n",options.fnv);

        msg(MSG_NULL "<%s>\n",msg);

        if ( h != options.fnv ) {
                msg(MSG_DIR "wrong password!\n");
                return -1;
        }

        for (i=0; (r=strtok(msg, SEP)) != NULL && i < options.ntoken && net_argc < 255; i++) {
                net_argv[net_argc++] = strdup(r);
                msg=NULL;
        }

        net_argv[net_argc] = NULL;
        return 0;
}


        void
free_argv(char **argv, int max)
{
        int i;
        for ( i =0; argv[i] != NULL && i < max && (free(argv[i]),1) ; i++ )
                argv[i] = NULL;
}


        int
recv_statement(int domain)
{
        struct net_msg *msg;
        int s, true=1;

        s = socket ( domain , SOCK_STREAM, 0);
        if ( s == -1) {
                warning("socket()");
                goto err0;
        }

        msg (MSG_INFO "listening on port (%d)\n",listen_port);
        msg (MSG_NULL "waiting for incoming connections...\n");

        /* socket options: reuseaddr */
        if ( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)) == -1 ) {
                warning("setsockopt(SOL_SOCKET..)");
                goto err1;
        }

        switch (domain) {
                case PF_INET: {
                                      struct sockaddr_in in, peer;
                                      socklen_t cs = sizeof(struct sockaddr_in);

                                      /* bind */
                                      in.sin_family = AF_INET;
                                      in.sin_port = htons((unsigned short) listen_port);
                                      in.sin_addr.s_addr = INADDR_ANY;
                                      if (bind(s, (struct sockaddr *) &in, sizeof(in)) == -1) {
                                              warning("bind()");
                                              goto err1;
                                      }

                                      /* listen */
                                      if (listen(s, 0) == -1) {
                                              warning("listen()");
                                              goto err1;
                                      }

                                      /* accept */
                                      if ((sk_fd = accept(s, (struct sockaddr *) &peer, &cs)) == -1) {
                                              warning("accept()");
                                              goto err1;
                                      }

                              } break;
                case PF_INET6: {
                                       struct sockaddr_in6 in6, peer6;
                                       socklen_t cs6 = sizeof(struct sockaddr_in6);

                                       /* bind */
                                       in6.sin6_family = AF_INET6;
                                       in6.sin6_port = htons((unsigned short) listen_port);
                                       memset(in6.sin6_addr.s6_addr,0,sizeof(struct in6_addr)); //  IN6ADDR_ANY_INIT

                                       if (bind(s, (struct sockaddr *) &in6, sizeof(in6)) == -1) {
                                               warning("bind()");
                                               goto err1;
                                       }

                                       /* listen */
                                       if (listen(s, 0) == -1) {
                                               warning("listen()");
                                               goto err1;
                                       }

                                       /* accept */
                                       if ((sk_fd = accept(s, (struct sockaddr *) &peer6, &cs6)) == -1) {
                                               warning("accept()");
                                               goto err1;
                                       }

                               } break;
                default:
                               fatal(__INTERNAL__);
        }

        msg = recv_message(sk_fd);
        if ( msg == NULL ) {
                warning("recv_message()");
                goto err2;
        }

        send_ack(sk_fd);
        if ( parse_message(msg) == -1) {
                atomicio (write, sk_fd , MSG_WORNGPASS , sizeof(MSG_WORNGPASS)-1);
                goto err3;
        }

        /* ok: */
        close (s);
        return 0;
err3:
        free(msg);
err2:
        close (sk_fd);
err1:
        close (s);
err0:
        return -1;

}


/*
 * generic wrappers
 */
inline void
my_kill(pid_t pid, int sig) {

        if ( kill (pid,sig) == -1 )
                fatal("kill()");
}


inline void
my_ptrace(int request, pid_t pid, void *addr, void *data) {

        if ( ptrace(request, pid, addr, data) == -1 && errno)
                fatal("ptrace");
}


#define EXPECT_EXITED   1
#define EXPECT_SIGNALED 2
#define EXPECT_STOPPED  4

inline void
my_wait(pid_t p, int report, int stopsig) {
        int status;
        pid_t pw = waitpid(p,&status,0);

        if (pw == (pid_t) -1) {
                warning("wait");
                return;
        }	

        if (WIFEXITED(status) && !(report & EXPECT_EXITED))
                msg(MSG_INFO "child exited%s with status %d\n",
                                WCOREDUMP(status) ? " and dumped core" : "", WEXITSTATUS(status));

        if (WIFSTOPPED(status) && !(report & EXPECT_STOPPED))
                msg(MSG_INFO "child stopped by signal %d\n", WSTOPSIG(status));

        if (WIFSIGNALED(status) && !(report & EXPECT_SIGNALED))
                msg(MSG_INFO "child signalled by signal %d\n", WTERMSIG(status));

        if (WIFSTOPPED(status) && WSTOPSIG(status) != stopsig) {
                /* a different signal - send it on and wait */
                msg(MSG_INFO "Waited for signal %d, got %d\n", stopsig, WSTOPSIG(status));

                if ((WSTOPSIG(status) & 0x7f) == (stopsig & 0x7f))
                        return;

                my_ptrace(PTRACE_SYSCALL, p, 0, (void*) WSTOPSIG(status));
                return my_wait(p, report, stopsig);
        }

        if ((report & EXPECT_STOPPED) && !WIFSTOPPED(status)) {
                warning("Not stopped?");
        }
}


/*
 * return the address of a given libc symbol
 */
#if 0
        addr_t 
get_libc_address(char *symbol)
{
        void *addr;
        void *handle;

        handle = dlopen(NULL, RTLD_NOW);
        if (handle == NULL)
                fatal ("%s", dlerror());		

        addr = dlsym(handle,symbol);
        if (addr == NULL) 
                fatal ("%s", dlerror());

        return (addr_t)addr;
}
#endif


        inline long
get_eip(pid_t p)
{
        long eip;

        eip = ptrace(PTRACE_PEEKUSER, p, 4*EIP, NULL);
        if (eip == -1 && errno)
                fatal("ptrace()");	

        return eip;
}


        inline long
set_eip(pid_t p,addr_t addr)
{
        long eip;

        eip = ptrace(PTRACE_POKEUSER, p, 4*EIP, (void *)addr);
        if (eip == -1 && errno)
                fatal("ptrace()");      

        return eip;
}


        inline long
get_orig_eax(pid_t p)
{
        long orig_eax;

        orig_eax = ptrace(PTRACE_PEEKUSER, p, 4*ORIG_EAX, NULL);
        if (orig_eax == -1 && errno ) {
                fatal("ptrace()");
        }

        return orig_eax;
}


/*
 * set a breakpoint at the given address
 */
        void
set_breakpoint(pid_t p, addr_t addr)
{
        unsigned long word;
        unsigned char *brkpoint = (unsigned char *)&word;

        errno=0;

        word=ptrace(PTRACE_PEEKTEXT,p , (void *)addr, NULL);
        if ( word == -1 && errno )
                fatal("ptrace(PEEKTEXT)");

        brk_prev_opcode = *brkpoint;
        *brkpoint = INT3;

        if ( ptrace(PTRACE_POKETEXT,p , (void *)addr, (void *)word)==-1 && errno )
                fatal("ptrace(POKETEXT)");

        msg(MSG_INFO "breakpoint set @0x%x\n",(unsigned int)addr);	
}	


/*
 * unset the breakpoint at the given address
 */
        void
unset_breakpoint(pid_t p, addr_t addr)
{
        unsigned long word;
        unsigned char *brkpoint = (unsigned char *)&word;

        errno=0;

        word=ptrace(PTRACE_PEEKTEXT,p , (void *)addr, NULL);
        if ( word == -1 && errno )
                fatal("ptrace(PEEKTEXT)");

        if (*brkpoint != INT3)
                fatal("not a valid breakpoint!");

        *brkpoint = brk_prev_opcode;

        if ( ptrace(PTRACE_POKETEXT,p , (void *)addr, (void *)word)==-1 && errno )
                fatal("ptrace(POKETEXT)");
}


/*
 * continue the program execution until...
 */
        void
continue_until_breakpoint(addr_t addr)
{
        set_breakpoint(pid, addr);

        my_ptrace( PTRACE_CONT, pid, NULL, NULL);	
        my_wait(pid, EXPECT_STOPPED, SIGTRAP);

        unset_breakpoint(pid, addr);

        set_eip(pid,addr);		/* restart from addr */
}


        void
continue_until_syscall(int i)
{
        int eax;

        eax = get_orig_eax(pid);
        do {
                my_ptrace( PTRACE_SYSCALL, pid, NULL, NULL);
                my_wait(pid, EXPECT_STOPPED, SIGTRAP);

        } while ( (eax=get_orig_eax(pid)) != i );

}


        inline
void child_process(int argc,char **argv)
{
        struct timeval now;

        my_ptrace (PTRACE_TRACEME,0, NULL, NULL);

        if ( !options.dry_run && execve(argv[0],argv,NULL) == -1 )
                fatal (__INTERNAL__);   

        /* dry test */
        asm("int3");

        gettimeofday(&now,NULL);
        msg (MSG_NULL "dry-run: {.tv_sec=%d , .tv_usec=%d}\n",(int)now.tv_sec,(int)now.tv_usec);	

        exit(0);
}


/*
 * tracer function
 */
        int
tracer(argc,argv,std)
        int argc;
        char **argv;
        FILE *std;
{
        struct timeval now;

        pid = fork();
        if (pid == -1)
                fatal(__INTERNAL__);

        if ( pid == 0 ) {
                /*** child ***/
                if ( std != stdout ) {
                        fclose (std);
                        close(sk_fd);
                }

                child_process(argc,argv);
        }			

        /*** parent ***/
        my_wait(pid, EXPECT_STOPPED, SIGTRAP);

        _signal(SIGINT,  sig_interrupt);
        _signal(SIGALRM, sig_alarm); 
        _signal(SIGPIPE, SIG_IGN);

        /* run the child until the system call/breakpoint is encountered */
        switch (options.trace_mode) {
                case trace_null:
                        fprintf(std,MSG_NULL "pid:%d eip:0x%x...\n", pid,
                                        (unsigned int)get_eip(pid));
                        break;

                case trace_syscall:
                        fprintf(std,MSG_INFO "tracing %s. break@%s[0x%x]\n", argv[0],
                                        syscall_names[options.syscall],
                                        options.syscall);

                        continue_until_syscall(options.syscall);
                        fprintf(std,MSG_NULL "pid:%d eip:0x%x...\n",pid,
                                        (unsigned int)get_eip(pid));
                        break;

                case trace_address:
                        fprintf(std,MSG_DIR "tracing %s. break@0x%x\n", argv[0],
                                        (unsigned int)options.address);

                        continue_until_breakpoint(options.address);
                        fprintf(std,MSG_NULL "pid:%d eip:0x%x...\n",pid,
                                        (unsigned int)get_eip(pid));
                        break;
        }

        /* wait for a trigger */
        switch (options.trigg_mode) {
                case trigg_null:
                        fprintf (std,MSG_INFO "triggered NOW!\n");
                        if ( fileno(std) != fileno(stdout) )
                                fclose(std);
                        break;

                case trigg_time:
                        gettimeofday(&now,NULL);
                        fprintf (std,MSG_INFO "sheduled at {.tv_sec=%d, .tv_usec=%d}\n", 
                                        (int)options.trigger.tv_sec, 
                                        (int)options.trigger.tv_usec);

                        count_down(&options.trigger);

                        /* become realtime, if required */
                        if (options.rt_lifetime) {
                                set_realtime(0  , SCHED_RR);
                                set_realtime(pid, SCHED_RR);
                        }

                        /* fine waiting */
                        wait_until(&options.trigger);
                        break;

                case trigg_net:
                        gettimeofday(&now,NULL);
                        fprintf (std,MSG_INFO "sheduled at {.tv_sec=%d, .tv_usec=%d}\n",
                                        (int)options.trigger.tv_sec, 
                                        (int)options.trigger.tv_usec);

                        /* release the client*/
                        fclose (std);	
                        close(sk_fd);

                        count_down(&options.trigger);

                        /* become realtime, if required */
                        if (options.rt_lifetime) {
                                set_realtime(0  , SCHED_RR);
                                set_realtime(pid, SCHED_RR);
                        }

                        /* fine waiting */
                        wait_until(&options.trigger);
                        break;
        }

        /* ulock the child, detaching the ptrace() */
#ifdef KERNEL_2_6
        vptrace   (PTRACE_DETACH, pid, 0, 0);
#else
        my_ptrace (PTRACE_DETACH, pid, NULL, NULL);
#endif
        /* relinquish the processor */
        sched_yield();

        /* set the safe-alarm, in case of RT privileges */
        if (options.rt_lifetime)	
                alarm(options.rt_lifetime);

        /* wait for the child termination */
        my_wait(pid,EXPECT_EXITED,0);
        alarm(0);

        return 0;
}


/*
 * main 
 */
        int 
main(argc,argv)
        int argc;
        char **argv;
{
        int i;
        char *endptr;
        time_t now_t;

        gettimeofday(&tv_start,NULL);

        time(&now_t);
        endptr= ctime(&now_t);
        msg (MSG_DIR "%.15s: %s", cvsid+5, endptr);

        while ((i = getopt(argc, argv, "f:t:a:s:r:d:w:p:6hlx")) != EOF)
                switch (i) {	
                        case '6':
                                options.family  = PF_INET6;
                                break;
                        case 'a':
                                options.address = p_integer (optarg,16);
                                options.trace_mode = trace_address;
                                break;
                        case 's':
                                options.symbol  = optarg;
                                options.syscall = get_syscall_index(optarg);
                                if (options.syscall == -1)
                                        fatal("get_syscall_index(\"%s\")= -1: symbol not found",optarg);
                                options.trace_mode = trace_syscall;
                                break;
                        case 't':
                                options.trigg_mode = trigg_time;
                                if ( p_timeval(&options.trigger, optarg) == -1 )
                                        fatal("bad trigger time!");
                                break;
                        case 'r':
                                options.rt_lifetime = strtol(optarg, &endptr, 10);
                                if (*endptr != '\0')
                                        fatal("strtol()");
                                break;
                        case 'd':
                                options.dry_run = p_integer(optarg, 10);
                                break;
                        case 'f':
                                options.script = optarg;
                                break;
                        case 'w':
                                options.password = optarg;
                                break;
                        case 'p':
                                listen_port = p_integer(optarg,10);
                                break; 
                        case 'x':
                                options.daemon = 1;
                                break;
                        case 'h':
                                usage();
                                /* NOTREACHED */
                        case 'l':
                                show_systable();
                                /* NOTREACHED */
                        case '?':
                                fatal(__INTERNAL__);
                                /* NOTREACHED */
                }

        argc -= optind;
        argv += optind;

        if (options.script && options.daemon)
                fatal("-x and -f are mutually exclusive options");

        /* client */
        if (options.script) {
                _signal(SIGALRM,sig_alarm_null);	
                core_parser(options.script);
                send_statement();
        }

        /* server */
        if (options.daemon) {
                FILE *std;
                for (;;) {

                        free_argv(net_argv,256);
                        net_argc=0;

                        if ( recv_statement(options.family) == -1 ) {
                                warning("recv_statement() I/O error");	
                                continue;
                        }

                        std = fdopen(sk_fd,"w+");
                        if ( std == NULL ) {
                                warning("fdopen()");
                                continue;
                        }

                        options.trigg_mode = trigg_net;

                        if (options.dry_run) {
                                struct timeval now;
                                gettimeofday(&now,NULL);
                                options.trace_mode     = trace_syscall;
                                options.syscall        = 0x4e; /* gettimeofday */
                                options.trigger.tv_sec = now.tv_sec + options.dry_run;
                                options.trigger.tv_usec= 0;
                        }
                        tracer(net_argc,net_argv,std);
                }

        }

        /* dry-run*/
        if (options.dry_run) {
                struct timeval now;
                gettimeofday(&now,NULL);
                options.trace_mode     = trace_syscall;
                options.trigg_mode     = trigg_time;
                options.syscall        = 0x4e; /* gettimeofday */
                options.trigger.tv_sec = now.tv_sec + options.dry_run;
                options.trigger.tv_usec= 0;
                argv[0]                = "\"dry-run\"";
        }

        /* local */
        if (argc || options.dry_run) 
                tracer(argc,argv,stdout);		

        return 0;
}
