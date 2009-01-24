/*
 *  dante.c: write arbitrary plaintext string in i386 executable opcodes.
 *
 *
 *  Copyright (c) 2002 Nicola Bonelli <bonelli@antifork.org>
 *
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <sysexits.h>
#include <errno.h>
#include <err.h>

static const char cvsid[] = "$Id: dante.c,v 1.15 2004/08/18 22:34:25 awgn Exp $";
static const char copyright[] = "Copyright (c) 2002 Bonelli Nicola <bonelli@antifork.org>";
static const char usage_str[]=
        "Usage: %s [OPTIONS]\n"
        "   -a                        Set EAX offset\n"
        "   -b                        Set EBX offset\n"
        "   -c                        Set ECX offset\n"
        "   -d                        Set EDX offset\n"
        "   -S                        Set ESI offset\n"
        "   -D                        Set EDI offset\n"
        "                             default values are set to 0\n"
        "   -v                        Display version number.\n"
        "   -h                        Print this help\n";


/* macro */
#define VERSION         "0.91"
#define MAX_SIZE        1024		/* stack */

/* registers' identifiers */

#define EAX     1
#define ECX     2
#define EDX     3
#define EBX     4
#define ESP     5
#define EBP     6
#define ESI     7
#define EDI     8
#define AL      EAX
#define CL      ECX
#define DL      EDX
#define BL      EBX
#define SI      ESI
#define DI      EDI
#define BP      EBP
#define SP      ESP

/* 
 * macro 
 */

#define CARRY4(x,y) do {                        \
  x+=y>>2;                                      \
  y =y&0x03;                                    \
} while (0)


#define SETUP_REG(x,y) do {                     \
  x.id=y;                                       \
  x.delta=0;                                    \
  x.offset=0;                                   \
} while (0)


#define MSG(f,arg...) do {                      \
  fprintf(stderr,f,## arg);                     \
} while (0)


#define NCUT(x) do {                            \
  register int i=0;                             \
  while(x[i]!=0)                                \
  { if (x[i]=='\n') x[i]=0;                     \
  i++; }                                        \
} while (0)


#define SPACE(x) do {                           \
  register int i=0;                             \
  while(x[i]!=0)                                \
  { if (x[i]==' ') x[i]='.';                    \
  i++; }                                        \
} while (0)


#define VALIDATE_CHARSET(c) do {		\
  if ( charset[(int)c] == 0 )                   \
  { MSG("\n\"%c\" is out of charset!            \
   [A-Z@\\[\\]\\^\\_/\\.]\n"                    \
  ,c); return 0; }                              \
} while (0)


#define HEX(h) do {                             \
  char t1[8];                                   \
  char t2[8];                                   \
  sprintf(t1,"\\x%.2x",h);                      \
  strcat(std_out,t1);                           \
  sprintf(t2,"%c",h);                           \
  strcat(std_code,t2);                          \
} while (0)


#define TEXT(s) do {                            \
  strcat(std_out,s);                            \
  strcat(std_code,s);                           \
} while (0)


#define INTEL_(x)       intel_tab[*(x)-'@']
#define INTEL_CODE(x)   intel_tab[*(x)-'@'].opcode
#define INTEL_BIT(x)    intel_tab[*(x)-'@'].regbit
#define INTEL_SEFF(x)   intel_tab[*(x)-'@'].seffect

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define SIGN(x)  ( (x) > 0 ? 1  : ( (x) == 0 ?   0  :  -1  ) )

#define SET(r,b)      ( r |=  b )
#define ISSET(r,b)    ( r &   b )
#define CLR(r,b)      ( r &= ~b )
#define BIT(x)        (	1<<(x-1))

#define INC(REG) HEX('@'-1+REG)
#define DEC(REG) HEX('H'-1+REG)

#define ADD(VAL,REG) switch(REG) {\
case AL: HEX(0x04); HEX(VAL); break;\
case BL: HEX(0x80); HEX(0xc3); HEX(VAL); break;\
case CL: HEX(0x80); HEX(0xc1); HEX(VAL); break;\
case DL: HEX(0x80); HEX(0xc2); HEX(VAL); break;\
case SI: HEX(0x66); HEX(0x83); HEX(0xc6); HEX(VAL); break;\
case DI: HEX(0x66); HEX(0x83); HEX(0xc7); HEX(VAL); break;\
case BP: HEX(0x66); HEX(0x83); HEX(0xc5); HEX(VAL); break;\
case SP: HEX(0x66); HEX(0x83); HEX(0xc4); HEX(VAL); break;\
default: fatal("unknown register"); break;}

#define SUB(VAL,REG) switch(REG) {\
case AL: HEX(0x2c); HEX(VAL); break;\
case BL: HEX(0x80); HEX(0xeb); HEX(VAL); break;\
case CL: HEX(0x80); HEX(0xe9); HEX(VAL); break;\
case DL: HEX(0x80); HEX(0xea); HEX(VAL); break;\
case SI: HEX(0x66); HEX(0x83); HEX(0xee); HEX(VAL); break;\
case DI: HEX(0x66); HEX(0x83); HEX(0xef); HEX(VAL); break;\
case BP: HEX(0x66); HEX(0x83); HEX(0xed); HEX(VAL); break;\
case SP: HEX(0x66); HEX(0x83); HEX(0xec); HEX(VAL); break;\
default: fatal("unknown register"); break;}


/* 
 * typedefs 
 */

typedef struct {
        int id;				/* register identifer */
        int delta;			/* effective offset */
	int offset;			/* expected offset, default is 0 */
} dnt_reg_t;


typedef struct {
	dnt_reg_t eax;
	dnt_reg_t ebx;
	dnt_reg_t ecx;
	dnt_reg_t edx;
	dnt_reg_t esp;
	dnt_reg_t ebp;
	dnt_reg_t esi;
	dnt_reg_t edi;
	int	poisoned;		/* bitfield */
	int	dirty;
} dnt_regset_t;


typedef struct {
	int (*opcode) (dnt_reg_t *);	/* opcode function */
	dnt_reg_t *p_reg;		/* involved register */
        int seffect;			/* stack's side effect */	
        int regbit;
} dnt_table_t;


typedef struct {
	dnt_reg_t rgstr[MAX_SIZE];
	int top;
	int top4;
	int offset;
	int depth;
} dnt_stack_t;


/* 
 * prototypes 
 */

int inc (dnt_reg_t *);
int dec (dnt_reg_t *);
int push(dnt_reg_t *);
int pop (dnt_reg_t *);


/* 
 * globals
 */

int charset[256] = {['.'] = 1,['/'] = 1,[64 ... 91] = 1,[93 ... 95] = 1};      /* good set */


/* stack */
dnt_stack_t stack;

/* registers */
dnt_regset_t reg;


extern char *__progname;

/* buffers*/
char std_in[MAX_SIZE];
char std_out[MAX_SIZE];
char std_code[MAX_SIZE];


/* instructions tables */

dnt_table_t intel_tab[] = {
	{inc,  &reg.eax, 0,  EAX}, {inc,  &reg.ecx, 0,  ECX}, 
        {inc,  &reg.edx, 0,  EDX}, {inc,  &reg.ebx, 0,  EBX},
	{inc,  &reg.esp, -1, ESP}, {inc,  &reg.ebp, 0,  EBP}, 
        {inc,  &reg.esi, 0,  ESI}, {inc,  &reg.edi, 0,  EDI},
	{dec,  &reg.eax, 0,  EAX}, {dec,  &reg.ecx, 0,  ECX}, 
        {dec,  &reg.edx, 0,  EDX}, {dec,  &reg.ebx, 0,  EBX},
	{dec,  &reg.esp, 1,  ESP}, {dec,  &reg.ebp, 0,  EBP}, 
        {dec,  &reg.esi, 0,  ESI}, {dec,  &reg.edi, 0,  EDI},
	{push, &reg.eax, 4,  EAX}, {push, &reg.ecx, 4,  ECX}, 
        {push, &reg.edx, 4,  EDX}, {push, &reg.ebx, 4,  EBX},
	{push, &reg.esp, 4,  ESP}, {push, &reg.ebp, 4,  EBP}, 
        {push, &reg.esi, 4,  ESI}, {push, &reg.edi, 4,  EDI},
	{pop,  &reg.eax, -4, EAX}, {pop,  &reg.ecx, -4, ECX}, 
        {pop,  &reg.edx, -4, EDX}, {pop,  &reg.ebx, -4, EBX},
	{pop,  &reg.esp, -4, ESP}, {pop,  &reg.ebp, -4, EBP}, 
        {pop,  &reg.esi, -4, ESI}, {pop,  &reg.edi, -4, EDI}
};


int i386_len[256] = {[0x04] = 1,[0x2c] = 1,[0x80] = 2,[0x83] = 2,[0x66] = 3};

char *i386_table_0[256] = {
	['@'] "inc     %eax",['A'] "inc      %ecx",['B'] "inc      %edx",['C'] "inc      %ebx",
	['D'] "inc     %esp",['E'] "inc      %ebp",['F'] "inc      %esi",['G'] "inc      %edi",
	['H'] "dec     %eax",['I'] "dec      %ecx",['J'] "dec      %edx",['K'] "dec      %ebx",
	['L'] "dec     %esp",['M'] "dec      %ebp",['N'] "dec      %esi",['O'] "dec      %edi",
	['P'] "push    %eax",['Q'] "push     %ecx",['R'] "push     %edx",['S'] "push     %ebx",
	['T'] "push    %esp",['U'] "push     %ebp",['V'] "push     %esi",['W'] "push     %edi",
	['X'] "pop     %eax",['Y'] "pop      %ecx",['Z'] "pop      %edx",['['] "pop      %ebx",
	['\\'] "pop    %esp",[']'] "pop      %ebp",['^'] "pop      %esi",['_'] "pop      %edi",
	['/'] "das",['.'] "cs",
};

char *i386_table_1[256] = {
	[0x04] = "add \t$0x%.2x, %%al\n",
	[0x2c] = "sub \t$0x%.2x, %%al\n",
};

char *i386_table_2[256] = {
	[0xc1] = "add \t$0x%.2x, %%cl\n",
	[0xc2] = "add \t$0x%.2x, %%dl\n",
	[0xc3] = "add \t$0x%.2x, %%bl\n",
	[0xc4] = "add \t$0x%.2x, %%esp\n",	/* special \x83\xc4\x.. */
	[0xe9] = "sub \t$0x%.2x, %%cl\n",
	[0xea] = "sub \t$0x%.2x, %%dl\n",
	[0xeb] = "sub \t$0x%.2x, %%bl\n",
};


char *i386_table_3[256] = {
	[0xc6] = "add \t$0x%.2x, %%si\n",
	[0xc7] = "add \t$0x%.2x, %%di\n",
	[0xc5] = "add \t$0x%.2x, %%bp\n",
	[0xc4] = "add \t$0x%.2x, %%sp\n",
	[0xee] = "sub \t$0x%.2x, %%si\n",
	[0xef] = "sub \t$0x%.2x, %%di\n",
	[0xed] = "sub \t$0x%.2x, %%bp\n",
	[0xec] = "sub \t$0x%.2x, %%sp\n",
};



/* 
 * fatal 
 */

void fatal(char *pattern,...) __attribute__((noreturn));
void
fatal(char *pattern,...)
{
        va_list ap;

        va_start(ap, pattern);

        if (errno)
                verr(EX_SOFTWARE,pattern,ap);
        else
                verrx(EX_SOFTWARE,pattern,ap);
}


void version(void) __attribute__((noreturn));
void
version()
{
        MSG("Dante %s. (c) Bonelli Nicola <bonelli@antifork.org>\n", VERSION);
        exit(0);
}



void usage(void) __attribute__((noreturn));
void
usage()
{
        fprintf(stderr, usage_str, __progname);
        exit(0);
}


inline void
setup_registers()
{
	SETUP_REG(reg.eax, EAX);
	SETUP_REG(reg.ebx, EBX);
	SETUP_REG(reg.ecx, ECX);
	SETUP_REG(reg.edx, EDX);
	SETUP_REG(reg.esp, ESP);
	SETUP_REG(reg.ebp, EBP);
	SETUP_REG(reg.esi, ESI);
	SETUP_REG(reg.edi, EDI);
}


int
inc(dnt_reg_t * r)
{
	if (r == &reg.esp)
		stack.top4--;
	CARRY4(stack.top, stack.top4);
	return ++r->delta;
}


int
dec(dnt_reg_t * r)
{
	if (r == &reg.esp)
		stack.top4++;
	CARRY4(stack.top, stack.top4);
	return --r->delta;
}


int
push(dnt_reg_t * r)
{
	stack.top++;
	stack.rgstr[stack.top].id = r->id;
	stack.rgstr[stack.top].delta = r->delta;
	return stack.top;
}


int
pop(dnt_reg_t * r)
{
	if (stack.top < 0)
		fatal("stack empty");

	r->id    = stack.rgstr[stack.top].id;
	r->delta = stack.rgstr[stack.top].delta;
	return --stack.top;
}


/*
 * Scan the human readable string in order to estimate the depth of the stack
 * identifying poisoned registers...
 */

int
scan_set(unsigned char *r)
{
	int stepi=0;

	if (r == NULL || *r == 0)
		return 0;

	while (*r != '\0') {

		VALIDATE_CHARSET(*r);

		switch (*r) {
		case '.':
		case 'd':
			break;
		case '/':

			/* das istruction ( AL poisoning ) */
			if (!ISSET(reg.poisoned, BIT(EAX)))
				reg.dirty++;
			SET(reg.poisoned, BIT(EAX));
			break;
		default:
			if (INTEL_SEFF(r) == -4) {
				/* -4 identifies a popl */
				if (!ISSET(reg.poisoned, BIT(INTEL_BIT(r))))
					reg.dirty++;
				SET(reg.poisoned, BIT(INTEL_BIT(r)));
			}
			stack.offset += INTEL_SEFF(r) & 3;
			stepi += INTEL_SEFF(r) >> 2;
			stack.depth = MIN(stack.depth, stepi);
			break;
		}
		r++;
	}
	return 0;
}



int
exec_code(char *r)
{
	if (r == NULL || *r == 0)
		return 0;

	while (*r != '\0') {
		switch (*r) {
		case '.':
		case '/':
		case 'd':
			break;
		default:
			(*INTEL_CODE(r)) (INTEL_(r).p_reg);
			break;
		}
		r++;
	}
	return 1;
}


void
print_stack()
{
	int i= stack.top;

	while (i > -1) {
		printf("%d:%d %d\n", i, stack.rgstr[i].id, stack.rgstr[i].delta);
		i--;
	}
	return;
}


void
insert_garbage()
{
	int i;
	int o;

	o = stack.offset / 4 + ((stack.offset % 4) ? SIGN(stack.offset) : 0);
	if (stack.depth + o < 0) {
		for (i = 0; i > (stack.depth + o); i--) {
			HEX(0x55);
			push(&reg.ebp);	/* push garbage into stack */
		}
	}
	fflush(stdout);
	return;
}


void
save_register()
{
	int i;

	for (i = 0; i < 8; i++) {

		if (!ISSET(reg.poisoned, 1 << i))
			continue;	/* the i-th bitfield register is not poisoned */

		/* skip ESP */
		if ( i+1 == ESP)
			continue;
		
		/*
		 * saving poisoned registers
		 */

		HEX('P'+i);

		switch (i+1) {
		case EAX:
			push(&reg.eax);
			break;
		case ECX:
			push(&reg.ecx);
			break;
		case EDX:
			push(&reg.edx);
			break;
		case EBX:
			push(&reg.ebx);
			break;
		case EBP:
			push(&reg.ebp);
			break;
		case ESI:
			push(&reg.esi);
			break;
		case EDI:
			push(&reg.edi);
			break;
		}
	}
	fflush(stdout);
	return;
}


void
load_register()
{
	int i;

	for (i = 7; i > -1; i--) {

		if (!ISSET(reg.poisoned, 1 << i))
			continue;	/* the i-th bitfield register is not poisoned */

		if (i == 4)
			continue;	/* skip ESP */

		HEX('X' + i);

		switch (i + 1) {
		case EAX:
			pop(&reg.eax); break;
		case ECX:
			pop(&reg.ecx); break;
		case EDX:
			pop(&reg.edx); break;
		case EBX:
			pop(&reg.ebx); break;
		case EBP:
			pop(&reg.ebp); break;
		case ESI:
			pop(&reg.esi); break;
		case EDI:
			pop(&reg.edi); break;
		}
	}
	fflush(stdout);
	return;
}


void
remove_remainder()
{
	int i;

	i = stack.top + 1 - reg.dirty;

	if (i > 0) {
		HEX(0x83);
		HEX(0xc4);
		HEX(i*4 );
	}
	stack.top -= i;

	return;
}


void
update_offset(dnt_reg_t * r, int regi)
{
	int diff;

	if (r->id != regi) {
		printf("r->id:%d into register:%d\n", r->id, regi);
		fatal("Internal error! Please report the string \"%s\" to <bonelli@antifork.org>", std_in);
	}

	diff = r->offset - r->delta;	/* diff */

	switch (diff) {
		case -1:
			DEC(regi);
			break;
		case -2:
			DEC(regi);
                	DEC(regi);
                	break;
		case 0:
			return;
		case 1:
			INC(regi);
			break;
		case 2:
			INC(regi);
                        INC(regi);
			break;
		
		default:
			if (diff > 0) {

				if ( diff >= 255 )
					fatal("increment exceed the max value (255)");
 
				ADD(diff, regi)
				return;
			}

			/* diff < 0 */

			if ( diff <= -255 )
				fatal("decrement exceed the max value (255)");

                       	SUB(-diff, regi)
	}
	return;
}


void
disasse_handler(int i)
{
	unsigned char *s = (unsigned char *)std_code;

	if (s == NULL)
		return;

	printf("\nDump of assembler code for the current string:\n");

	while (*s) {
		int len;
		len = i386_len[(int) *s];

		switch (len) {
		case 0:
			if (i386_table_0[*s] == NULL)
				fatal("[\\x%.2x] unknown opcode", *s);
			printf("%s\t\t/* %c */\n", i386_table_0[*s], *s);
			s++;
			break;
		case 1:
			printf(i386_table_1[*s], *(s + 1));
			s += 2;
			break;
		case 2:
			if (i386_table_2[*(s + 1)] == NULL)
				fatal("[\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1));
			printf(i386_table_2[*(s + 1)], *(s + 2));
			s += 3;
			break;
		case 3:
			if (*(s + 1) != 0x83)
				fatal("[\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1));
			if (i386_table_3[*(s + 2)] == NULL)
				fatal("[\\x%.2x\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1), *(s + 2));
			printf(i386_table_3[*(s + 2)], *(s + 3));
			s += 4;

			break;
		}
	}
}


void
objdump_handler(int i)
{
	unsigned char *s = (unsigned char *)std_code;

	if (s == NULL)
		return;

	printf("\nDisassembly of the current string:\n");

	while (*s) {
		int len;
		len = i386_len[(int) *s];

		switch (len) {
		case 0:
			if (i386_table_0[*s] == NULL)
				fatal("[\\x%.2x] unknown opcode", *s);
			printf("%.2x\t%s\n", *s, i386_table_0[*s]);
			s++;
			break;
		case 1:
			printf("%.2x %.2x\t", *s, *(s + 1));
			printf(i386_table_1[*s], *(s + 1));
			s += 2;
			break;
		case 2:
			if (i386_table_2[*(s + 1)] == NULL)
				fatal("[\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1));
			printf("%.2x %.2x %.2x\t", *s, *(s + 1), *(s + 2));
			printf(i386_table_2[*(s + 1)], *(s + 2));
			s += 3;
			break;
		case 3:
			if (*(s + 1) != 0x83)
				fatal("[\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1));
			if (i386_table_3[*(s + 2)] == NULL)
				fatal("[\\x%.2x\\x%.2x\\x%.2x] unknown opcode", *s, *(s + 1), *(s + 2));
			printf("%.2x %.2x %.2x %.2x\t", *s, *(s + 1), *(s + 2), *(s + 3));
			printf(i386_table_3[*(s + 2)], *(s + 3));
			s += 4;

			break;
		}
	}
}


void
reallign_esp()
{

	int i;
	int j;

	i = stack.offset % 4;

	if (i == 0)
		return;

	if (i > 0) {
		for (j = 0; j < i; j++)
			HEX(0x44);	/* inc %esp */
		return;
	}
	for (j = 0; j > i; j--)
		HEX(0x4c);	/* dec %esp */
	return;
}


int
main(int argc, char **argv)
{
	int es;

	while ((es = getopt(argc, argv, "a:b:c:d:S:D:vh")) != EOF)
		switch (es) {
		case 'a':
			reg.eax.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'b':
			reg.ebx.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'c':
			reg.ecx.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'd':
			reg.edx.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'S':
			reg.esi.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'D':
			reg.edi.offset = strtol(optarg, (char **) NULL, 0);
			break;
		case 'h':
			usage();
			break;
		case 'v':
			version();
			break;
		}

        argc -= optind;
        argv += optind;

	signal(SIGTSTP, objdump_handler);	/* ignore SIGTSTP  */
	signal(SIGINT, disasse_handler);	/* ignore SIGINT */
	signal(SIGQUIT, SIG_IGN);		/* ignore SIGQUIT */

	/* reading from stdin */

	MSG("Dante %s. (c) Bonelli Nicola <bonelli@antifork.org>\n", VERSION);
	MSG("\nwrite a plaintext string (^D exit | ^C disassemble | ^Z objdump ):\n");

	for (;;) {

		printf(">");
		fflush(stdout);

		std_in[0] = 0;	/* reset */

		if (fgets(std_in, MAX_SIZE, stdin) == NULL)
			break;

		setup_registers(); /* clear registers */

		std_out [0]= 0;
		std_code[0]= 0;

		/* parsing */
		NCUT(std_in);
		SPACE(std_in);

		/* registers defaults */
		reg.poisoned = 0;
		reg.dirty= 0;

		/* setup stack */
		stack.top= -1;
		stack.top4= 0;
		stack.offset= 0;
		stack.depth= 0;
		
		if (scan_set((unsigned char *) std_in)) {

			/* save poisoned registers */
			save_register();

			/* insert garbage layer */
			insert_garbage();

			/* print plaintext code */
			TEXT(std_in);

			/* exec code */ 
			exec_code(std_in);

			/* reallign esp */
			reallign_esp();

			/* remove remainder */
			remove_remainder();

			/* load saved value */
			load_register();

			/* tune registers with offsets */
			update_offset(&reg.eax, EAX);
			update_offset(&reg.ebx, EBX);
			update_offset(&reg.ecx, ECX);
			update_offset(&reg.edx, EDX);
			update_offset(&reg.esi, ESI);
			update_offset(&reg.edi, EDI);
			update_offset(&reg.ebp, EBP);

			/* print the output */
			printf("\n\"%s\"\n", std_out);
		}
	}

	MSG("\nbye.\n");
	exit(0);
}
