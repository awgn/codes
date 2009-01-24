/*
 * qhash: a quasi perfect hash generator.
 *
 * Copyright (c) 2004 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * Linux  : gcc qhash.c -o qhash -lm -ldl
 * OpenBSD: gcc qhash.c -o qhash -lm
 * FreeBSD: gcc qhash.c -o qhash -lm
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <sysexits.h>
#include <dlfcn.h>
#include <math.h>

static const char cvsid[] = "$Id: qhash.c,v 1.18 2004/01/06 17:04:28 awgn Exp $";
static const char copyright[] = "Copyright (c) 2003 Bonelli Nicola <bonelli@antifork.org>";

typedef struct {
	char 			*key;
	unsigned long long 	hash;
}
tab;

#define CC			"/usr/bin/cc"
#define VERSION			"1.3"
#define TABIN_SIZE		(1<<16) 
#define TABOUT_SIZE		(1<<16)*10	/* heuristically 10 times bigger than
					 	* the input table */

/* Thanks to ettercap's authors */
#ifdef __OpenBSD__
/* The below define is a lie since we are really doing RTLD_LAZY since the
 * system doesn 't support RTLD_NOW.
 */
#define RTLD_NOW DL_LAZY
#endif

#if defined(__OpenBSD__)
#define SYM(x)  "_" ## x
#else
#define SYM(x)  x
#endif

#define SONAME(x) ({\
	char *r;\
	r=(char *)malloc(strlen(x)+6);\
	r[0]='.';\
	r[1]='/';\
	strcat(r,x);\
	strcat(r,".so");\
	r;\
})

#define MNAME(x) do {\
char *r;\
	r=strrchr(x,'.');\
	if (r!= NULL)\
		*r='\0';\
} while (0)
		
#define SET(reg,bit) 	((reg) |=(bit))
#define CLR(reg,bit) 	((reg) &=~(bit))
#define ISON(reg,bit) 	((reg) & (bit))

/* 
 * options
 */
#define  FILE_OPT        1
#define  HA32_OPT        1<<1
#define  HA64_OPT	 1<<2
#define  TABT_OPT        1<<3

#ifndef __unused 
#define __unused        __attribute__((unused))
#endif
#ifndef __dead
#define __dead          __attribute__((noreturn))
#endif


static char usage_str[]=
"Usage: %s OPTIONS\n"
" input:                                    \n"
"    -f                     set keyword file\n"
"    -s                     set hash.c source\n"
" general:                                   \n"
"    -t                     set (tab_t *) as default return value\n"
"    -6                     use 64 bit hash (default 32)\n"
"    -p                     use table prime size (fast)\n"
"    -i                     use table incremental size (slow)\n"
"    -h                     print this help\n"
"    -v                     print version\n";


tab table[TABIN_SIZE];
tab tabout[TABOUT_SIZE];

static char esc_kl[] = {0x1b, 0x5b, '2', 'K'};  /* vt100: clear the current line */
#define clear_line()	write(2,esc_kl,sizeof(esc_kl))

unsigned int n_key;
unsigned int r_key;
unsigned int r_test;
unsigned int null_entry;
unsigned int options;
unsigned int exit_status;

unsigned long (*qhash32) (char *, int);
unsigned long long (*qhash64) (char *, int);

extern char *__progname;

char **environ;			/* thanks to Megat0n for obsd porting */

char filename[80];
char hashname[80];
char buf[1024];
char tok[1024];

/* functions */

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


int
next_prime(int base)
{
        int p = base + 1;       /* candiate prime */
        int ret;                /* return value */
        int max;
        int i;

        for (;;p++) {
                max= (int)sqrt((double)p);
                ret= p;

                for (i = 2; i <= max; i++) {

                        if ((p%i) == 0) {
                                ret= -1;
                                break;
                        }
                }
                if (ret != -1)
                        break;
        }

        return ret;
}


int
next_int(int i)
{
	return i+1;
}


int (*next_key)(int) = next_prime;


void
clear_table(tab * t, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		t[i].key = NULL;
		t[i].hash = 0;
	}
}


int
cc_call(const char *source)
{
	int pid, status;
	char *s, *o;

	s = (char *) malloc(strlen(source) + 3);
	o = (char *) malloc(strlen(source) + 4);

	strcpy(s, source);
	strcpy(o, source);
	strcat(s, ".c");
	strcat(o, ".so");

	if (source == 0)
		return -1;
	pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) {
		char *argv[8];
		argv[0] = CC;
		argv[1] = (char *) s;
		argv[2] = "-o";
		argv[3] = (char *) o;
		argv[4] = "-fPIC";
		argv[5] = "-shared";
		argv[6] = 0;

		execve(CC, argv, environ);
		exit(127);
	}
	do {
		if (waitpid(pid, &status, 0) == -1) {
			if (errno != EINTR)
				return -1;
		} else
			return status;
	}
	while (1);
}

int
main(int argc, char *argv[], char **env)
{
	FILE *fd;
	unsigned long i,j,es;
	char *sobj;
	char *hname, *tt;
	void *handle;

	environ = (char **) env;

	while ((es = getopt(argc, argv, "f:s:pti6vh")) != EOF)
		switch (es) {
		case 'f':
			strncpy(filename, optarg, 79);
			SET(options, FILE_OPT);
			break;
		case 's':
			strncpy(hashname, optarg, 79);
			SET(options, HA32_OPT);
			break;
		case '6':
			SET(options, HA64_OPT);
			break;
		case 't':
			SET(options, TABT_OPT);
			break;
		case 'i':
			next_key = next_int;
			break;
		case 'p':
			next_key = next_prime;
			break;
		case 'v':
			printf("%s\n", VERSION);
			exit(1);
			break;
		case 'h':
			fprintf(stderr,usage_str,__progname);
			exit(0);
		}

        argc -= optind;
        argv += optind;

	if (!ISON(options, FILE_OPT))
		fatal("! qhash: -f opt non given");

	if (!ISON(options, HA32_OPT))
		fatal("! qhash: -s opt non given");

	fprintf(stderr,"> qhash %s: %s\n",VERSION,copyright);

	fprintf(stderr, next_key== next_prime ? 
			"  using table prime size (fast)\n" :
			"  using table incremental size (slow)\n");

        fprintf(stderr, ISON(options, HA64_OPT) ?
                        "  using 64-bit hash algorithm\n" :
                        "  using 32-bit hash algorithm (default)\n" );

	hname = strdup(hashname);

	if (strchr(hashname,'.') == NULL)               
        	fatal("! qhash: not extension given");  

	MNAME(hname);

	fprintf(stderr,"> compiling %s.c ...\n",hname);
	exit_status = cc_call(hname);

	switch (WEXITSTATUS(exit_status)) {
	case 0:
		/* ok */
		break;
	case 1:
		fatal("! cc: fatal error");
		break;
	case 127:
		fatal("! qhash: #define CC %s ?!? compiler not found", CC);
		break;
	}

	sobj = SONAME(hname);

	fprintf(stderr, "> loading %s ...\n", sobj);
	handle = dlopen(sobj, RTLD_NOW);

	if (!handle) {
		fputs(dlerror(), stderr);
		fputs("\n", stderr);
		exit(1);
	}

	qhash32 = dlsym(handle, SYM("hash"));
	qhash64 = dlsym(handle, SYM("hash"));

	if (qhash32 == NULL)
		fatal("! qhash: prototype error");

	if ((fd = fopen(filename, "r")) == NULL) {
		perror("open");
		exit(1);
	}

	fprintf(stderr,"> loading tokens ...\n");
	while (fgets(buf, 1023, fd) != NULL && n_key < TABIN_SIZE) {
		if ( sscanf(buf,"%1023s",tok) != 1 )
			continue;
				
		tt = strtok(tok," ");

		if ( tt == NULL || tt[0]=='#' )
			continue;
		
		fprintf(stderr,"+ token: \"%s\"\r",tok);
		table[n_key].key = strdup(tok);

		table[n_key].hash =  ISON(options, HA64_OPT) ? 
				     (unsigned long long)qhash64(tok, strlen(tok)) : 
				     (unsigned long long)qhash32(tok, strlen(tok)) ;
		n_key++;
	}

	fclose(fd);

	clear_line();
	fprintf(stderr,"> done.\n");

	if (n_key == TABIN_SIZE)
		fatal("! qhash: number of words exceed TAB_SIZE[%d]", TABIN_SIZE);

	/* check for hash collision */
	fprintf(stderr,"> checking for free hash collision:\n");
	for (i = 0; i < n_key - 1; i++) {
		fprintf(stderr,"  %lu%%\r",100*(i+1)/n_key);
		for (j = i + 1; j < n_key; j++) {
#if 0
			fprintf(stderr,"%llx\n", table[i].hash);
#endif
			if (table[i].hash != table[j].hash)
				continue;
			
			if (strcmp(table[i].key,table[j].key)==0)
				fatal("! qhash: found clones of the key <%s>", table[i].key);
			else			
				fatal("! qhash: collision! <%s> and <%s> have the same hash. change hash.c algorithm",
					 table[i].key, table[j].key);

		}
	}
	fprintf(stderr,"  free collision not found (hash ok).\n");
	r_key = (n_key + 1);

	/* try to get the smallest lazy mod mapping table with no collision */ 
	for (;;r_key=next_key(r_key)) {
		fprintf(stderr,"+(%d) trying size=%d ...\r",++r_test,r_key);

		if (r_key > TABOUT_SIZE)
			fatal("! qhash: table required too large. (max %d)", TABOUT_SIZE);

		clear_table(tabout, r_key);

		for (i = 0; i < n_key; i++) {
			j = (table[i].hash % r_key);

			if (tabout[j].key != NULL)	/* collision */
				break;

			tabout[j].key = table[i].key;
			tabout[j].hash = i;
		}

		/* search for a null entry: required */
		for (j = 0; j < r_key; j++)
			if (tabout[i].key == NULL) {
				null_entry = j;
				break;
			}

		/* exit test*/
		if (i == n_key && null_entry != -1)
			break;
	}

	clear_line();
	fprintf(stderr,"! lazy-mod-table size=%d!\n",r_key);

	fprintf(stderr, "> printing source code to stdout...\n");
	
	/* print tab_t */
	printf("#include <stdlib.h>						\n"
	       "#include <string.h>						\n"
	       "	typedef struct {					\n"
	       "		char *key;					\n"
	       "		int id;						\n"
	       "	}							\n"
	       "	tab_t;							\n"
		);


	printf( ISON(options, TABT_OPT) ?
		"tab_t * qsearch(const char *);\n\n" :
		"int qsearch(const char *);\n\n");

	printf("#if 0\n");
	for (i = 0; i < n_key; i++)
		printf("#define KEY_%s %ld\n", table[i].key, i);

	printf(ISON(options, TABT_OPT) ?
		"\n\ntab_t *p;                                                \n"
		"                    int id;                                  \n"
		"     p = qsearch(\"----->STR<-----\"); switch (p->id) {      \n" :

		"\nint id;                                                    \n"
		"     switch ((id = qsearch (\"----->STR<-----\")) ) {        \n");

	for (i = 0; i < n_key; i++) 
		printf("\tcase KEY_%s:\n\t\tbreak;\n", table[i].key);

	printf ("\tcase -1: /*unknown */\n\tbreak;\n\t}\n");
	
	printf (ISON(options, TABT_OPT) ?
		"#define TABT_NULL %d\n#endif\n\n" :
		"#endif\n\n" , null_entry );

        printf ("#define TABLE_SIZE (sizeof(tab)/sizeof(tab[0]))  /* %d */\n\n", r_key);

	printf ("static tab_t tab[]={\n");
	for (i = 0; i < r_key; i++) {
		if (tabout[i].key)
			printf("{\"%s\",%llu },", tabout[i].key, tabout[i].hash);
		else
			printf("{\"\",-1 },");

		if ( i && (i%10)==0 )
			printf("\n");
	}
	printf("};\n\n");

	if ((fd = fopen(hashname, "r")) == NULL) {
		perror("open");
		exit(1);
	}

	while (fgets(buf, 1024, fd) != NULL) {
		fputs(buf, stdout);
	}

	fclose(fd);

	if (ISON(options, TABT_OPT))
		printf(
		       "\n#ifdef __GNUC__							\n"
		       "__inline								\n"
		       "#endif									\n"
		       "tab_t *									\n"
		       "qsearch(p)								\n"
		       "	const char *p;							\n"
		       "{									\n"
		       "	unsigned int i;							\n"
		       "	if (p == NULL)							\n"
		       "		return &tab[TABT_NULL];				        \n"
		       " 									\n"
		       "	i = hash((char *) p, strlen(p)) %% TABLE_SIZE;			\n"
		       "	if (*tab[i].key == *p && strcmp(tab[i].	key, p) == 0)   	\n"
		       "		return	&tab[i];					\n"
		       "	else								\n"
		       "		return	&tab[TABT_NULL];				\n"
		       "}\n\n 									\n"
			);
	else
		printf(
		       "\n#ifdef __GNUC__							\n"
		       "__inline								\n"
		       "#endif									\n"
		       "int									\n"
		       "qsearch(p)								\n"
		       "	const char *p;							\n"
		       "{									\n"
		       "	unsigned int i;							\n"
		       "	if (p == NULL)							\n"
		       "		return -1;						\n"
		       "	i = (unsigned int) hash((char *) p, strlen(p)) %% TABLE_SIZE;	\n"
		       "	if (*tab[i].key == *p && strcmp(tab[i].  key, p) == 0)		\n"
		       "		return tab[i].id;					\n"
		       "	else								\n"
		       "		return -1;						\n"
		       "}\n\n									\n"
			);

	fprintf(stderr,"  n_key=%d r_key=%d #test=%d -> efficiency=%d%%\n",
			n_key,r_key,r_test,n_key*100/r_key);

        fprintf(stderr,"> done.\n");

	exit(0);
}
