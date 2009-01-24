/*
 * $Id: dbksym.c,v 1.7 2004/08/20 14:23:53 awgn Exp $
 * dbksym --
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
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
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sysexits.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <alloca.h>
#include <err.h>

#ifndef __linux__
#error !dbksym is designed for linux only!
#endif


/*
 * MACRO
 */

#define VERSION		"1.1"
#define KERNBASESOURCE	"/usr/src/"

#define ADDR_MASK       0xff000000	/* address bitmask */
#define HDEPTH  	1<<16		/* depth for hash table */
#define MISMAX		256		/* max number of allowed mismatching
				 	* symbols */

#define __INTERNAL__    "%s:%d %s() internal error",\
			__FILE__,__LINE__,__FUNCTION__
#define __OK__          "\t(ok)\n"
#define __FAIL__	"\t(fail)\n"

#ifndef __unused
#define __unused	__attribute__((unused))
#endif
#ifndef __dead
#define __dead		__attribute__((noreturn))
#endif
#ifndef __elem
#define	__elem(lista,i, inf, next) \
			[i]={ inf, next: &lista[i+1] }
#endif
#ifndef __tail
#define __tail(lista,i, inf, next) \
      			[i]={ inf, next: NULL }
#endif

#define unless(x,y)   	do { if ( (x) || (y) ); } while (0)

#define msg(format,...) fprintf(stderr, format, ## __VA_ARGS__)
#define MSG_NULL	"    "
#define MSG_DIR		"[*] "
#define MSG_INFO	"[i] "
#define MSG_READ	"[r] "
#define MSG_WRITE	"[w] "

/* macro DEBUG() */
#ifdef EBUG
#define DEBUG(format, ...) do { \
fprintf (stderr, "    %s:", __PRETTY_FUNCTION__); \
fprintf (stderr, format, ## __VA_ARGS__); \
} while (0)
#else
#define DEBUG(f,arg...) do {} while (0)
#endif

/* strdupa */
#ifndef strdupa
#define strdupa(s)                                                            \
  (__extension__                                                              \
    ({                                                                        \
      __const char *__old = (s);                                              \
      size_t __len = strlen (__old) + 1;                                      \
      char *__new = (char *) __builtin_alloca (__len);                        \
      (char *) memcpy (__new, __old, __len);                                  \
    }))
#endif

/*
 * like assert() exits if the argument passed is false, unlike assert() don't care
 * about NDEBUG macro
 */
#define ASSERT(x)       x ? : fatal("%s:%d :%s(): Assertion `%s' failed",\
			__FILE__,__LINE__,__FUNCTION__,#x);

/*
 * cvsid
 */
__unused const char cvsid[] = "$Id: dbksym.c,v 1.7 2004/08/20 14:23:53 awgn Exp $";


/*
 * DATA and TYPES
 */
typedef unsigned long addr_t;
typedef unsigned long hash32_t;

/*
   Database design:

		   .-----.  .-----.  .-----.
            map -->|     |->|     |->|     |
                   `-----'  `-----'  `-----'
                       ^         ^
                     ^ :       ^ :      ^
     hash            | :       | :      |
       \             | :       | :      |
        [     ]--> {   }       | :     /
        [     ]------------->{   }-->{   }
        [     ]        :         :
                       :        '
                      '       '
     key             '      '
       \           '      '
        [addr ][ ptr ]  '
        [     ][ ptr ]'
        [     ][     ]

 */

TAILQ_HEAD(tailhead, symbol_s);

struct symbol_s {
	addr_t addr;
	char *symb;
	char *type;
	     TAILQ_ENTRY(symbol_s) entries;
};


struct key_s {
	addr_t addr;
	struct symbol_s *symb;
};


struct system {
	TAILQ_HEAD(, symbol_s) map;
	struct symbol_s **hash[HDEPTH];
	struct key_s *key;
	int hash_nmemb[HDEPTH];
	int key_nmemb;
}      System, proc_ksyms;


struct elem {
	char *tag;
	struct elem *next;
};

/*
 * GLOBAL structures
 */
const char usage_s[] = "\
Usage:%s [options] symbol symbol... 	 		         \n\
   -m FILE		System.map file                          \n\
   -f                   when -m opt is given, don't validate     \n\
   -v                   display the version and exit.            \n\
   -h                   print this help.                         \n";


enum { 	proc_ksyms_linux_24,
	proc_ksyms_linux_26 };

const char *proc_ksyms_s[] = { [proc_ksyms_linux_24] = "/proc/ksyms",
			       [proc_ksyms_linux_26] = "/proc/kallsyms" };

struct elem system_path[] = {
	__elem(system_path, 0, tag:"/", next),
	__elem(system_path, 1, tag:"/boot", next),
	__tail(system_path, 2, tag:"/usr/src/linux", next)
};
struct elem *system_path_head = system_path;

struct elem system_file[] = {
	__tail(system_file, 0, tag:"/boot/System.map", next)
};
struct elem *system_file_head = system_file;

struct {
	char *system_map;
	int force;
}      options;

/*
 * FUNCTION
 */

/* insert element to a-list */
void
alist_insert(struct elem * head, const char *t)
{
	struct elem *p, *q;

	for (q = head; q != NULL; q = q->next)
		p = q;
	q = malloc(sizeof(struct elem));
	q->tag = strdup(t);
	q->next = NULL;
	p->next = q;
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


/* <----- FNV -----> */
/*
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 *
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *      chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!     :-)
 */

#define MASK_16 	(((u_int32_t)1<<16)-1)
#define xor_fold_16(x)  ((x) = ((x)>>16) ^ ((x)& MASK_16))

#define FNV_prime       16777619U
#define offset_basis    2166136261U

__inline
hash32_t
fnv32(const char *p, int s)
{
	unsigned long h = offset_basis;	/* FNV-1 hash */
	int i = 0;

	for (; i < s; i++)
		h = ((h * FNV_prime) ^ (p[i]));

	return h;
}

/* <----- FNV -----> */


/*
 * remove crc from versioned symbol
 */
char *
discard_crc(char *p)
{
	int len;

	if (p == NULL)
		goto ret;

	if ((len = strlen(p)) < 11)
		goto ret;

	if (p[len - 10] == '_' &&
	    p[len - 9] == 'R' &&
	    isxdigit(p[len - 8]) &&
	    isxdigit(p[len - 7]) &&
	    isxdigit(p[len - 6]) &&
	    isxdigit(p[len - 5]) &&
	    isxdigit(p[len - 4]) &&
	    isxdigit(p[len - 3]) &&
	    isxdigit(p[len - 2]) &&
	    isxdigit(p[len - 1]))
		p[len - 10] = '\0';
ret:
	return p;
}


/*
 * add hash reference
 */
void
add_hash_reference(struct system * sys, struct symbol_s * s, u_int16_t h)
{
	sys->hash[h] = realloc(sys->hash[h], (sys->hash_nmemb[h] + 1) * sizeof(struct symbol_s *));
	*(sys->hash[h] + sys->hash_nmemb[h]) = s;
	sys->hash_nmemb[h]++;
#if 0
	int i = 0;
	printf("[h]=%d ->", h);
	for (; i < sys->hash_nmemb[h]; i++) {
		printf("%p ", *(sys->hash[h] + i));
	}
	printf("\n");
#endif

}


/*
 * add key reference
 */
void
add_key_reference(struct system * sys, struct symbol_s * s, addr_t addr)
{
	sys->key = realloc(sys->key, (sys->key_nmemb + 1) * sizeof(struct key_s));
	sys->key[sys->key_nmemb].addr = addr;
	sys->key[sys->key_nmemb].symb = s;
	sys->key_nmemb++;

#if 0
	int i = 0;
	printf("===========\n");
	for (i; i < sys->key_nmemb; i++)
		printf("%x %p\n", sys->key[i].addr, sys->key[i].symb);
#endif

}


/*
 * parse the proc/ksyms entry, and load symbols in the head_ksyms list
 */

int proc_sscanf_24(char *line, u_int32_t *addr, char *sym, char *mod)
{
	//printf("proc_sscanf_24:");
	return sscanf(line,"%x %s [%[^]]", addr, sym, mod);
}

int proc_sscanf_26(char *line, u_int32_t *addr, char *sym, char *mod)
{
	//printf("proc_sscanf_26:");
	return sscanf(line,"%x %s %s", addr, mod, sym);
}

int (*proc_sscanf)(char *,u_int32_t *, char *, char *);

int
proc_ksyms_load(FILE * k, int kernel)
{
	char sym[128], mod[128];
	struct symbol_s *new;
	addr_t addr;
	hash32_t hash;
	char *line;
	size_t len;
	int ret;

	line = NULL;
	len = 0;
	ret = -1;

	
	if ( kernel == proc_ksyms_linux_24 )
		proc_sscanf=proc_sscanf_24;
	else
	if ( kernel == proc_ksyms_linux_26 )
		proc_sscanf=proc_sscanf_26;
	else
		fatal(__INTERNAL__);

	while (getline(&line, &len, k) != -1) {

		ret = proc_sscanf(line, (u_int32_t *) &addr, sym, mod);
		ASSERT(ret >= 2);

		/*
		 * alloc a new symbol_s
		 */
		new = malloc(sizeof(struct symbol_s));
		new->addr = addr;
		new->symb = discard_crc(strdup(sym));
		new->type = (ret == 3 ? strdup(mod) : NULL);

		TAILQ_INSERT_TAIL(&proc_ksyms.map, new, entries);

		/*
		 * ADD hash reference
		 */

		/* calc fnv32 hash */
		hash = fnv32(new->symb, strlen(new->symb));

		/* produce a 16 bit FNV-1 hash by xor-folding */
		xor_fold_16(hash);

		/* add the symbol-hash reference to proc_ksyms system */
		add_hash_reference(&proc_ksyms, new, hash);

		/*
		 * ADD key reference
		 */
		add_key_reference(&proc_ksyms, new, addr);

	}
	return ret;
}


/*
 * for dubug purposes...
 */
int
sys_dump_map(struct tailhead * hp)
{
	struct symbol_s *n;

	for (n = hp->tqh_first; n != NULL; n = n->entries.tqe_next)
		msg(MSG_INFO "%x %s\n", (u_int32_t) n->addr, n->symb);

	return 0;
}


/*
 * given an address, search for the nearest symbol
 */
struct symbol_s *
query_address(struct system * sys, addr_t a)
{
	struct symbol_s *ret;
	int i;

	ret = NULL;
	for (i = 0; i < sys->key_nmemb; i++) {
		if (a < sys->key[i].addr)
			return NULL;
		if (a >= sys->key[i].addr && a < sys->key[i + 1].addr) {
			ret = sys->key[i].symb;
			break;
		}
	}
	return ret;
}

/*
 * search a symbol in the given database, return its address when found, 
 * NULL otherwise 
 */
struct symbol_s *
query_symbol(struct system * sys, char *symb)
{
	struct symbol_s *n, *ret;
	hash32_t hash;
	int i;

	ret = NULL;
	if (symb == NULL)
		goto ret;

	DEBUG("\t%s\n", symb);
	/* calc fnv32 hash */
	hash = fnv32(symb, strlen(symb));

	/* produce a 16 bit FNV-1 hash by xor-folding */
	xor_fold_16(hash);
	DEBUG("\t[h]=%d\n", hash);
	DEBUG("\tsys->hash[h]=%p\n", sys->hash[hash]);

	if (sys->hash[hash] == NULL)
		goto ret;

	/* scan the collision avoidance list */
	for (i = 0, n = *(sys->hash[hash]); i < sys->hash_nmemb[hash]; n = *(++i + sys->hash[hash])) {
		ASSERT(n != NULL);
		ASSERT(n->symb != NULL);
		if (strcmp(n->symb, symb) == 0) {
			ret = n;
			break;
		}
	}
ret:
	return ret;
}


/*
 * to validate the System.map, the proc/ksyms kernel symbols must occur in
 * the System.map and match in values.
 */
int
sys_validate(struct system * sys, struct system * pks, int verbose)
{
	struct symbol_s *n, *ret;
	int n_test, n_ok;

	n_test = n_ok = 0;
	for (n = pks->map.tqh_first; n != NULL; n = n->entries.tqe_next) {

		if ((n->addr & ADDR_MASK) != 0xc0000000)
			continue;	/* symbol exported by a kernel
					 * module */

		if ((ret = query_symbol(sys, n->symb)) == NULL) {
			(verbose == 0 ? : warnx("`%s' symbol not found.", n->symb));
			goto restart;
		}
		if (verbose) {
			(n->addr != ret->addr ?
			 warnx("`%s@0x%x' symbol mismatch (0x%x).",
			       n->symb, (u_int32_t) ret->addr, (u_int32_t) n->addr) : n_ok++);

		} else
			(n->addr != ret->addr ? : n_ok++);
restart:
		n_test++;
	}

	fprintf(stderr, " %d/%d ",n_ok,n_test);
	return (n_test - n_ok) < MISMAX ? 0 : -1;
}


/*
 * database descrutor
 */
void
sys_free(struct system * s)
{
	struct symbol_s *n, *m;
	int i;

	/* free the map */
	for (n = s->map.tqh_first; n != NULL; n = m) {
		m = n->entries.tqe_next;
		free(n->symb);
		free(n);
	}
	s->map.tqh_first = 0;
	s->map.tqh_last = &s->map.tqh_first;

	/* free hash reference */
	for (i = 0; i < HDEPTH; i++) {
		free(s->hash[i]);
		s->hash[i] = NULL;
		s->hash_nmemb[i] = 0;
	}
	/* free key reference */
	free(s->key);
	s->key = NULL;
	s->key_nmemb = 0;
}

/*
 * parse the System.map file, and load symbols in the database.
 */
int
sys_load(FILE * k)
{
	char sym[128], type[128];
	struct symbol_s *new;
	int ret;
	addr_t addr;
	hash32_t hash;

	while ((ret = fscanf(k, "%x %s %s\n", (u_int32_t *) & addr, type, sym)) == 3) {

		new = malloc(sizeof(struct symbol_s));
		new->addr = addr;
		new->symb = strdup(sym);
		new->type = strdup(type);


		TAILQ_INSERT_TAIL(&System.map, new, entries);

		/*
		 * ADD hash reference
		 */

		/* calc fnv32 hash */
		hash = fnv32(new->symb, strlen(new->symb));

		/* produce a 16 bit FNV-1 hash by xor-folding */
		xor_fold_16(hash);

		/* add the symbol-hash reference to proc_ksyms system */
		add_hash_reference(&System, new, hash);

		/*
                 * ADD key reference
                 */
		add_key_reference(&System, new, addr);
	}

	return ret == -1 ? 0 : -1;
}


/*
 * concatenate two strings, using a static buffer
 */
char *
strmrg(char *a, char *b)
{
	static char *__strmrg;
	char *pa, *pb, *p;
	int len_a, len_b;
	len_a = 0;
	len_b = 0;
	pa = a;
	pb = b;

	if (a != NULL) {
		len_a = strlen(a);
		if (pa == __strmrg)
			pa = strdupa(a);

	}
	if (b != NULL) {
		len_b = strlen(b);
		if (pb == __strmrg)
			pb = strdupa(b);
	}
	p = __strmrg = realloc(__strmrg, len_a + len_b + 1);

	if (pa) {
		while ((*p++ = *pa++));
		p--;
	}
	if (pb) {
		while ((*p++ = *pb++));
	}
	return __strmrg;
}


/*
 * add paths for System.map candidates
 */
void
update_path()
{
	struct dirent **namelist;
	int n;

	n = scandir(KERNBASESOURCE, &namelist, 0, alphasort);
	if (n != -1) {
		for (; n--; free(namelist[n])) {
			if (namelist[n]->d_type != DT_DIR)
				continue;
			if (namelist[n]->d_name[0] == '.')
				continue;
			alist_insert(system_path, strmrg(KERNBASESOURCE, namelist[n]->d_name));
		}
		free(namelist);
	}
}


/*
 * update the list of candidates
 */
void
update_candiate(char *dir)
{
	struct dirent **namelist;
	int n;

	n = scandir(dir, &namelist, 0, alphasort);
	if (n != -1) {
		for (; n--; free(namelist[n])) {
			if (namelist[n]->d_type != DT_REG)
				continue;
			if (memcmp(namelist[n]->d_name, "System.map", sizeof("System.map") - 1) != 0)
				continue;
			alist_insert(system_file, strmrg(strmrg(dir, "/"), namelist[n]->d_name));
		}
		free(namelist);
	}
}


/*
 * open System.map file
 */
char *
sys_open(char *f)
{
	struct elem *p;
	FILE *sm;

	if (f != NULL) {
		sm = fopen(f, "r");
		if (sm == NULL)
			fatal("%s: file not found", f);

		msg(MSG_DIR "loading %s...   ", f);
		if (sys_load(sm) == -1)
			fatal(__INTERNAL__);
		msg(__OK__);

		if (options.force)
			return f;

		msg(MSG_NULL "validating %s...", f);
		if (sys_validate(&System, &proc_ksyms, 1) == -1)
			fatal("%s: invalid System.map!\t(failed)", f);
		msg(__OK__);

		return f;
	}
	/* extend paths */
	update_path();

	/* add possibile System.map candidates */
	for (p = system_path_head; p != NULL; p = p->next)
		update_candiate(p->tag);

	/* dump the candidate list */
	for (p = system_file_head; p != NULL; p = p->next) {
		msg(MSG_INFO "%s\n", p->tag);
		sm = fopen(p->tag, "r");
		if (sm == NULL)
			continue;

		if (sys_load(sm) == -1)
			fatal(__INTERNAL__);

		msg(MSG_NULL "validating %s...", p->tag);
		if (sys_validate(&System, &proc_ksyms, 0) == 0) {
			msg(__OK__);
			fclose(sm);
			break;
		}
		msg(__FAIL__);
		sys_free(&System);
		fclose(sm);
	}

	if (p == NULL)
		fatal("no valid System.map found");

	return p->tag;
}


/*
 * perform an automatic query
 */
struct symbol_s *
query_auto(char *token)
{
	struct symbol_s *ret;
	addr_t addr;
	char *endptr;

	addr = strtoul(token, &endptr, 16);

	if (*endptr == '\0')
		unless(ret = query_address(&System, addr),
		       ret = query_address(&proc_ksyms, addr));
	else
		unless(ret = query_symbol(&System, token),
		       ret = query_symbol(&proc_ksyms, token));

	if (ret == NULL)
		printf(*endptr == '\0' ? "%s [out of scope]\n" : "0x0 %s [not found]\n", token);

	return ret;
}

/*
 * query interface
 */
void
query_interface(int argc, char **argv)
{
	char buff[128];
	struct symbol_s *ret;
	int c;

	/* read from argv */
	if (argc != 0) {
		while (argc--) {
			ret = query_auto(*argv);
			if (ret != NULL)
				printf("0x%x %s [%s]\n", (u_int32_t) ret->addr, ret->symb, ret->type);
			argv++;
		}
		return;
	}
	/* read from stdin */
	while ((c = read(fileno(stdin), buff, 127)) != -1) {
		if (c == 0)
			break;
		buff[c - 1] = '\0';
		ret = query_auto(buff);
		if (ret != NULL)
			printf("0x%x %s [%s]\n", (u_int32_t) ret->addr, ret->symb, ret->type);
	}
}


/*
 * findksym: a tool for kernel symbols querying.
 */
int
main(int argc, char **argv)
{
	FILE *pks;
	char *sysmap;

	int i;

	while ((i = getopt(argc, argv, "fhm:v")) != EOF)
		switch (i) {
		case 'f':
			options.force = 1;
			break;
		case 'h':
			usage();
			/* NOTREACHED */
		case 'm':
			options.system_map = (optarg);
			break;
		case 'v':
			printf("%s\n", cvsid);
			exit(0);
		case '?':
			fatal(__INTERNAL__);
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	msg(MSG_DIR "dbksym %s\n", VERSION);

	/* initialize TAIL */
	TAILQ_INIT(&proc_ksyms.map);
	TAILQ_INIT(&System.map);

	/* load /proc/ksyms database */

	if ( (pks=fopen(proc_ksyms_s[proc_ksyms_linux_24], "r")) ) {
		/* /proc/ksyms */
		if (proc_ksyms_load(pks,proc_ksyms_linux_24) == -1)
			fatal(__INTERNAL__);

		goto pks_done;
	}

        if ( (pks=fopen(proc_ksyms_s[proc_ksyms_linux_26], "r")) ) {
                /* /proc/ksyms */
                if (proc_ksyms_load(pks,proc_ksyms_linux_26) == -1)
                        fatal(__INTERNAL__);

                goto pks_done;
        }
		 
	ASSERT(pks != NULL);

pks_done:

	fclose(pks);
#if 0
	DEBUG("\n%d=sys_dump_map()\n", sys_dump_map((struct tailhead *) & proc_ksyms.map));
#endif
	/* open and load a valid system_map database */
	sysmap = sys_open(options.system_map);
	ASSERT(sysmap != NULL);

	query_interface(argc, argv);

	return 0;
}
