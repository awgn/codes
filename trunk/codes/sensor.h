/*
 * sensor.h
 *
 * Copyright (c) 2002 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * Linux   2.4.x
 * OpenBSD 2.8|2.9|3.0|3.1|3.2
 * FreeBSD 4.4|4.5|4.6|4.7|5.0
 * NetBSD  1.5.2|1.6 (no siginfo support)
 *
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
 */


#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#define __USE_GNU
#include <ucontext.h>
#undef  __USE_GNU
#endif

#define CVS		"$Id: sensor.h,v 1.13 2003/05/09 00:52:22 awgn Exp $"
#define COPYRIGHT       "Copyright (c) 2002 Bonelli Nicola <bonelli@antifork.org>"
#define VERSION         "0.51"

#define LOG_INFO	1
#define LOG_REG		1<<1
#define LOG_ALL		0xffffffffL

#define CAT(a,b)       a ## b
#define INFO_PID()     printf("   pid= %d\n",		getpid())
#define INFO_HEX(x)    printf("   %s= %#x\n",		#x,(long)x)
#define INFO_INT(x)    printf("   si_%s= %d\n",		#x,(int)sip->si_##x)
#define INFO_ADR(x)    printf("   si_%s= %p\n",		#x,(void *)sip->si_##x)
#define INFO_CODE(x,c) printf("   si_code(%d)= %s\n",	c,x[c])

/* arch dependences */

#if ! #cpu (i386)
#error "The current sensor.h supports i386 hosts only"
#endif

#if   defined(__linux__)

#define SI_CODE_POSIX
#define HAVE_SIGINFO

#define ILL_CODE  	 ill_code
#define FPE_CODE  	 fpe_code
#define SEGV_CODE 	segv_code
#define BUS_CODE  	 bus_code
#define TRAP_CODE	trap_code
#define CLD_CODE 	 cld_code
#define POLL_CODE	poll_code

#define DUMP_REG(x)     printf("   %s\t%#x\t%d\n",(#x)+4,((ucontext_t *)scp)->uc_mcontext.gregs[x],\
                        ((ucontext_t *)scp)->uc_mcontext.gregs[x])

#define REG_gs         REG_GS
#define REG_fs         REG_FS
#define REG_es         REG_ES
#define REG_ds         REG_DS
#define REG_edi        REG_EDI
#define REG_esi        REG_ESI
#define REG_ebp        REG_EBP
#define REG_esp        REG_ESP
#define REG_ebx        REG_EBX
#define REG_edx        REG_EDX
#define REG_ecx        REG_ECX
#define REG_eax        REG_EAX
#define REG_eip        REG_EIP
#define REG_cs         REG_CS
#define REG_eflags     REG_EFL
#define REG_ss 	REG_SS


#elif defined(__OpenBSD__)

#define SI_CODE_POSIX
#define HAVE_SIGINFO

#define ILL_CODE         ill_code
#define FPE_CODE         fpe_code
#define SEGV_CODE       segv_code
#define BUS_CODE         bus_code
#define TRAP_CODE       trap_code
#define CLD_CODE         cld_code
#define POLL_CODE       poll_code

#define DUMP_REG(x)  	printf("   %s\t%#x\t%d\n", (#x)+4, \
			((struct sigcontext *)scp)->CAT(sc_,x), \
			((struct sigcontext *)scp)->CAT(sc_,x))

#define REG_gs        	gs
#define REG_fs        	fs
#define REG_es         es
#define REG_ds         ds
#define REG_edi        edi
#define REG_esi        esi
#define REG_ebp        ebp
#define REG_esp        esp
#define REG_ebx        ebx
#define REG_edx        edx
#define REG_ecx        ecx
#define REG_eax        eax
#define REG_eip        eip
#define REG_cs         cs
#define REG_eflags     eflags
#define REG_ss         ss

#elif defined(__FreeBSD__)

#define SI_CODE_TRAP
#define HAVE_SIGINFO

#define ILL_CODE        global_code
#define FPE_CODE           fpe_code
#define SEGV_CODE       global_code
#define BUS_CODE        global_code
#define TRAP_CODE       global_code
#define CLD_CODE        global_code
#define POLL_CODE       global_code

#define DUMP_REG(x)     printf("   %s\t%#x\t%d\n", (#x)+4, \
                        ((struct sigcontext *)scp)->CAT(sc_,x), \
                        ((struct sigcontext *)scp)->CAT(sc_,x))

#define REG_gs         gs
#define REG_fs         fs
#define REG_es         es
#define REG_ds         ds
#define REG_edi        edi
#define REG_esi        esi
#define REG_ebp        ebp
#define REG_esp        esp
#define REG_ebx        ebx
#define REG_edx        edx
#define REG_ecx        ecx
#define REG_eax        eax
#define REG_eip        eip
#define REG_cs         cs
#define REG_eflags     eflags
#define REG_ss         ss

#elif defined(__NetBSD__)

/* Struct siginfo not available yet!
   Future versions of NetBSD will replace the sigcontext interface with the siginfo interface.
*/

#define DUMP_REG(x)     printf("   %s\t%#x\t%d\n", (#x)+4, \
                        ((struct sigcontext *)scp)->CAT(sc_,x), \
                        ((struct sigcontext *)scp)->CAT(sc_,x))

#define REG_gs         gs
#define REG_fs         fs
#define REG_es         es
#define REG_ds         ds
#define REG_edi        edi
#define REG_esi        esi
#define REG_ebp        ebp
#define REG_esp        esp
#define REG_ebx        ebx
#define REG_edx        edx
#define REG_ecx        ecx
#define REG_eax        eax
#define REG_eip        eip
#define REG_cs         cs
#define REG_eflags     eflags
#define REG_ss         ss

#elif
#error "The current sensor.h doesn't support your unix flavor"
#endif

#define SIG(x)       [  SIG##x]  "SIG"  #x
#define ILL_(x,y)    [ ILL_##x]  "ILL_" #x "(" y ")"
#define FPE_(x,y)    [ FPE_##x]  "FPE_" #x "(" y ")"
#define BUS_(x,y)    [ BUS_##x]  "BUS_" #x "(" y ")"
#define CLD_(x,y)    [ CLD_##x]  "CLD_" #x "(" y ")"
#define SEGV_(x,y)   [SEGV_##x] "SEGV_" #x "(" y ")"
#define TRAP_(x,y)   [TRAP_##x] "TRAP_" #x "(" y ")"
#define POLL_(x,y)   [POLL_##x] "POLL_" #x "(" y ")"
#define T_(x,y)	     [T_##x]       "T_" #x "(" y ")"

/* prototype */

static void sensor();

/* sigaction */

struct sigaction old[32];
struct sigaction hdr;
int options;

/* signal table */

char *sig_table[33] = {
#ifdef SIGHUP
SIG(HUP),
#endif
#ifdef SIGINT
SIG(INT),
#endif
#ifdef SIGQUIT
SIG(QUIT),
#endif
#ifdef SIGILL
SIG(ILL),
#endif
#ifdef SIGTRAP
SIG(TRAP),
#endif
#ifdef SIGABRT
SIG(ABRT),
#endif
#ifdef SIGEMT
SIG(EMT),
#endif
#ifdef SIGFPE
SIG(FPE),
#endif
#ifdef SIGKILL
SIG(KILL),
#endif
#ifdef SIGBUS
SIG(BUS),
#endif
#ifdef SIGSEGV
SIG(SEGV),
#endif
#ifdef SIGSYS
SIG(SYS),
#endif
#ifdef SIGPIPE
SIG(PIPE),
#endif
#ifdef SIGALRM
SIG(ALRM),
#endif
#ifdef SIGTERM
SIG(TERM),
#endif
#ifdef SIGURG
SIG(URG),
#endif
#ifdef SIGSTOP
SIG(STOP),
#endif
#ifdef SIGTSTP
SIG(TSTP),
#endif
#ifdef SIGCONT
SIG(CONT),
#endif
#ifdef SIGCHLD
SIG(CHLD),
#endif
#ifdef SIGTTIN
SIG(TTIN),
#endif
#ifdef SIGTTOU
SIG(TTOU),
#endif
#ifdef SIGIO
SIG(IO),
#endif
#ifdef SIGXCPU
SIG(XCPU),
#endif
#ifdef SIGXFSZ
SIG(XFSZ),
#endif
#ifdef SIGVTALRM
SIG(VTALRM),
#endif
#ifdef SIGPROF
SIG(PROF),
#endif
#ifdef SIGWINCH
SIG(WINCH),
#endif
#ifdef SIGLOST
SIG(LOST),
#endif
#ifdef SIGUSR1
SIG(USR1),
#endif
#ifdef SIGUSR2
SIG(USR2),
#endif
#ifdef SIGPWR
SIG(PWR),
#endif
#ifdef SIGPOLL
SIG(POLL),
#endif
#ifdef SIGWIND
SIG(WIND),
#endif
#ifdef SIGPHONE
SIG(PHONE),
#endif
#ifdef SIGWAITING
SIG(WAITING),
#endif
#ifdef SIGLWP
SIG(LWP),
#endif
#ifdef SIGDANGER
SIG(DANGER),
#endif
#ifdef SIGGRANT
SIG(GRANT),
#endif
#ifdef SIGRETRACT
SIG(RETRACT),
#endif
#ifdef SIGMSG
SIG(MSG),
#endif
#ifdef SIGSOUND
SIG(SOUND),
#endif
#ifdef SIGSAK
SIG(SAK),
#endif
#ifdef SIGPRIO
SIG(PRIO),
#endif
};


/* si_code */

#if defined (SI_CODE_POSIX)

char *ill_code[12] = {
	ILL_(ILLOPC, "illegal opcode"),
	ILL_(ILLOPN, "illegal operand"),
	ILL_(ILLADR, "illegal addressing mode"),
	ILL_(ILLTRP, "illegal trap"),
	ILL_(PRVOPC, "privileged opcode"),
	ILL_(PRVREG, "privileged register"),
	ILL_(COPROC, "co-processor"),
	ILL_(BADSTK, "bad stack"),
};

char *fpe_code[12] = {
	FPE_(INTDIV, "integer divide by zero"),
	FPE_(INTOVF, "integer overflow"),
	FPE_(FLTDIV, "floating point divide by zero"),
	FPE_(FLTOVF, "floating point overflow"),
	FPE_(FLTUND, "floating point underflow"),
	FPE_(FLTRES, "floating point inexact result"),
	FPE_(FLTINV, "invalid floating point operation"),
	FPE_(FLTSUB, "subscript out of range"),
};

char *segv_code[4] = {
	SEGV_(MAPERR, "address not mapped to object"),
	SEGV_(ACCERR, "invalid permissions"),
};

char *bus_code[4] = {
	BUS_(ADRALN, "invalid address alignment"),
	BUS_(ADRERR, "non-existent physical address"),
	BUS_(OBJERR, "object specific hardware error"),
};

char *trap_code[4] = {
	TRAP_(BRKPT, "breakpoint trap"),
	TRAP_(TRACE, "trace trap"),
};

char *cld_code[8] = {
	CLD_(EXITED, "child has exited"),
	CLD_(KILLED, "child was killed"),
	CLD_(DUMPED, "child has coredumped"),
	CLD_(TRAPPED, "traced child has stopped"),
	CLD_(STOPPED, "child has stopped on signal"),
	CLD_(CONTINUED, "stopped child has continued"),
};

#if defined (__linux__)
char *poll_code[8] = {
	POLL_(IN, "input available"),
	POLL_(OUT, "output possible"),
	POLL_(MSG, "message available"),
	POLL_(ERR, "I/O error"),
	POLL_(PRI, "high priority input available"),
	POLL_(HUP, "device disconnected"),
};
#endif
#endif

#if defined(SI_CODE_TRAP)

/* Freebsd flavor */

char *global_code[32] = {
	T_(PRIVINFLT, "privileged instruction"),
	T_(BPTFLT, "breakpoint instruction"),
	T_(ARITHTRAP, "arithmetic trap"),
#if defined(T_ASTFLT)
	T_(ASTFLT, "system forced exception"),
#endif
	T_(PROTFLT, "protection fault"),
	T_(TRCTRAP, "debug exception (sic)"),
	T_(PAGEFLT, "page fault"),
	T_(ALIGNFLT, "alignment fault"),

	T_(DIVIDE, "integer divide fault"),
	T_(NMI, "non-maskable trap"),
	T_(OFLOW, "overflow trap"),
	T_(BOUND, "bound instruction fault"),
	T_(DNA, "device not available fault"),
	T_(DOUBLEFLT, "double fault"),
	T_(FPOPFLT, "fp coprocessor operand fetch fault"),
	T_(TSSFLT, "invalid tss fault"),
	T_(SEGNPFLT, "segment not present fault"),
	T_(STKFLT, "stack fault"),
	T_(MCHK, "machine check trap"),
	T_(XMMFLT, "SIMD floating-point exception"),
	T_(RESERVED, "reserved (unknown)"),

	ILL_(PRIVIN_FAULT, "privileged instruction"),
	ILL_(ALIGN_FAULT, "alignment fault"),
	ILL_(FPOP_FAULT, "coprocessor operand fault"),

	BUS_(PAGE_FAULT, "page fault protection base"),
	BUS_(SEGNP_FAULT, "segment not present"),
	BUS_(STK_FAULT, "stack segment"),
	BUS_(SEGM_FAULT, "segment protection base"),

};

char *fpe_code[12] = {
	FPE_(INTOVF, "integer overflow"),
	FPE_(INTDIV, "integer divide by zero"),
	FPE_(FLTDIV, "floating point divide by zero"),
	FPE_(FLTOVF, "floating point overflow"),
	FPE_(FLTUND, "floating point underflow"),
	FPE_(FLTRES, "floating point inexact result"),
	FPE_(FLTINV, "invalid floating point operation"),
	FPE_(FLTSUB, "subscript out of range"),
};

#endif

static void
#ifdef HAVE_SIGINFO
act_handler(int sig, siginfo_t * sip, void *scp)
#else
act_handler(int sig, int code, struct sigcontext * scp)
#endif
{

	int c;

	printf("catched signal %s!\n", sig_table[sig]);
	if (options & LOG_INFO) {
		INFO_PID();
#ifdef HAVE_SIGINFO
		INFO_INT(signo);
		INFO_INT(errno);

		switch (sig) {
		case SIGSEGV:
		case SIGBUS:
		case SIGILL:
		case SIGFPE:
			if (sip->si_addr)
				INFO_ADR(addr);
			break;

		case SIGCHLD:
			INFO_INT(pid);
			INFO_INT(uid);
			INFO_INT(status);

#if !defined(__FreeBSD__)
			INFO_INT(utime);
			INFO_INT(stime);
#endif

#if defined (SIGPOLL)
		case SIGPOLL:
#endif

#if defined (SIGXFSZ)
		case SIGXFSZ:
#endif

#if defined (SIGPOLL) || (SIGXFSZ)

#if defined (__linux__)
			INFO_INT(fd);
			INFO_INT(band);
			break;
#endif
#endif
		}

		/* code */
		c = sip->si_code;

		switch (sig) {
		case SIGILL:
			INFO_CODE(ILL_CODE, c);
			break;
		case SIGFPE:
			INFO_CODE(FPE_CODE, c);
			break;
		case SIGSEGV:
			INFO_CODE(SEGV_CODE, c);
			break;
		case SIGBUS:
			INFO_CODE(BUS_CODE, c);
			break;
		case SIGTRAP:
			INFO_CODE(TRAP_CODE, c);
			break;
		case SIGCHLD:
			INFO_CODE(CLD_CODE, c);
			break;

#if defined (SIGPOLL)
		case SIGPOLL:
			INFO_CODE(POLL_CODE, c);
			break;
#endif

		}
#else
		INFO_HEX(code);
#endif				/* HAVE_SIGINFO */

	}
	if (options & LOG_REG) {
		puts("\ncontext:");
		DUMP_REG(REG_eax);
		DUMP_REG(REG_ecx);
		DUMP_REG(REG_edx);
		DUMP_REG(REG_ebx);
		DUMP_REG(REG_esp);
		DUMP_REG(REG_ebp);
		DUMP_REG(REG_esi);
		DUMP_REG(REG_edi);
		DUMP_REG(REG_eip);
		DUMP_REG(REG_eflags);
		DUMP_REG(REG_cs);
		DUMP_REG(REG_ss);
		DUMP_REG(REG_ds);
		DUMP_REG(REG_es);
		DUMP_REG(REG_fs);
		DUMP_REG(REG_gs);
		puts("");
	}
	switch ((int) old[sig].sa_handler) {

	case ((int) SIG_IGN):
		printf("   signal ignored.\n");
		break;

	case ((int) SIG_DFL):
		printf("   signal with default action.\n");
		signal(sig, SIG_DFL);
		raise(sig);
		break;

	default:
		printf("   signal handler at 0x%x\n", (int) old[sig].sa_handler);
		old[sig].sa_handler(sig);
		break;

	}

}

static void
sensor(int opt)
{
	int i;

#ifdef HAVE_SIGINFO
	hdr.sa_flags = SA_SIGINFO;
	hdr.sa_sigaction = act_handler;
#else
	hdr.sa_handler = (void (*) (int)) act_handler;
#endif
#ifdef  SA_RESTART
        hdr.sa_flags |= SA_RESTART;     /* SVR4, 44BSD */
#endif

	printf("sensor %s %s\n", VERSION, COPYRIGHT);
	options = opt;
	for (i = 0; i < 32; i++)
		sigaction(i, &hdr, &old[i]);

	return;
}
