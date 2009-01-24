/*
 *  fgets_th: fgets with history and tab completion
 *
 *  Copyright (c) 02,2003 Nicola Bonelli <bonelli@antifork.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include "fgets_th.h"

static const char cvsid[] = "$Id: fgets_th.c,v 1.11 2003/05/06 22:20:40 awgn Exp $";

/* gloabals */

typedef unsigned char REGISTER;

static char *history[TH_HISTSIZE];
static char esc_up[] = {0x1b, 0x5b, 0x41, 0x0};
static char esc_dw[] = {0x1b, 0x5b, 0x42, 0x0};
static char esc_rg[] = {0x1b, 0x5b, 0x43, 0x0};
static char esc_lf[] = {0x1b, 0x5b, 0x44, 0x0};
static char esc_cl[] = {0x1b, 0x5b,'H',0x1b, 0x5b,'2','J'};	/* clear screen */
static char esc_kl[] = {0x1b, 0x5b, '2', 'K'};			/* Clear entire line */

static int hist_index = -1;
static int esc_state;

struct termios otty;

void
reset_terminal()
{
	 tcsetattr(0, TCSANOW, &otty);
}

static void
fifo_push(char *a)
{
	int i;
	free(history[TH_HISTSIZE - 1]);
	for (i = TH_HISTSIZE - 1; i > 0; i--)
		history[i] = history[i - 1];

	history[0] = strdup(a);
}

static int
getchar_raw()
{
	struct termios tty;
	int c;

	tcgetattr(0, &otty);

	tty = otty;
	tty.c_lflag &= ~(ECHO | ICANON);
	tty.c_cc[VMIN]= 1;
	tty.c_cc[VTIME]= 0;
	tcsetattr(0, TCSANOW, &tty);

	c = getchar();

	tcsetattr(0, TCSANOW, &otty);
	return c;
}

static int
fgets_raw(char *s, int size, char *prompt)
{
	int i, j;
	char c;

	LINE_CL();

	/* setup vars */
	j = i = strlen(s), esc_state = 0;

	/* print prompt */
	printf("\r%s%s", prompt, s), fflush(stdout);

	while (i < (size - 1)) {
		c = getchar_raw();
		switch (c) {
		case KEY_BS:
		case KEY_DEL:
			esc_state = 0;
			if (i > 0) {
				i--;
				BACKSPACE();
				s[i] = ' ';
			} else
				BELL();
			break;
		case KEY_ESC:
			esc_state += (esc_state == 0 ? 1 : 0);
			break;
		case KEY_BRACKET:
			if (esc_state == 1) {
				esc_state++;
			} else {
				esc_state = 0;
				goto label;
			}
			break;
		case KEY_A:
			if (esc_state == 2) {
				return GETS_EUP;
			}
			esc_state = 0;
			goto label;
		case KEY_B:
			if (esc_state == 2) {
				return GETS_EDOWN;
			}
			esc_state = 0;
			goto label;
		case KEY_C:
			if (esc_state == 2) {
				esc_state = 0;
				ESC_RG();
				i++;
			} else
				goto label;
			break;
		case KEY_D:
			if (esc_state == 2) {
				esc_state = 0;
				if (i > 0) {
					ESC_LF();
					i--;
				} else
					BELL();
			} else
				goto label;
			break;
		case KEY_LN:
			s[j] = 0;
			goto end;
		case KEY_HT:
			s[j] = 0;
			RMSPACE(s);
			return GETS_ETAB;
		default:
	label:
			esc_state = 0;
			putchar(c);
			if (i == j)
				j++;
			s[i++] = c;
			break;
		}
	}
end:
	s[size - 1] = 0;
	RMSPACE(s);
	return strlen(s);
}

/*** strings variant ***/

static char *
strscpy(char *dest, const char *src)
{
	char eos[] = "\0";
	char *tmp = dest;

	if (src == NULL)	/* prevent segfault if src == NULL */
		src = eos;

	while ((*dest++ = *src++) != '\0')
		 /* nothing */ ;
	return tmp;
}

static int
strscmp(char *a, char *b)
{
	while (*a != '\0' && *b != '\0' && *a == *b)
		a++, b++;
	return *a == '\0';	/* dont' care about b */
}

/*** public ***/

int
fgets_th(char *s, int size, char *prompt, char **comm)
{

	REGISTER reg[TH_INDEX bit];	/* register for commands */
	int i, j, n, cnt, lst, and, nand;

	s[0] = 0;
	hist_index = -1;

#if defined(__linux__) || defined (__solaris__)
	__fpurge(stdin);	
#else
	if ( fseek(stdin, 0L, SEEK_END) == -1 )	 {	/* flush stdin */ 
		fprintf(stderr,	"%d: WARNING: stdin cannot be purged properly!\n",__FUNCTION__);	
	}	
#endif
	/* reset terminal at exit */
	atexit(reset_terminal);
 
	while ((j = fgets_raw(s, size, prompt)) < 0) {
		while (s != NULL && *s == ' ')
			DELETE(s);
		switch (j) {
		case GETS_ETAB:
			BELL();

			/*** reset values ***/
			cnt = 0, lst = 0, i = 0;
			memset(reg, 0, TH_INDEX bit);

			/*** scan the command list for partial sub match ***/
			while (comm[i] != NULL && i < TH_INDEX) {
				if (strscmp(s, comm[i])) {
					R_SET(reg, i);
					cnt++;
					lst = i;
				}
				i++;
			}

			if (cnt == 0)	/* ensure that at least a word matches */
				break;

			switch (cnt) {
			case 1:/* a single word found */
				s[0] = 0;
				strncat(s, comm[lst], size - 2), strcat(s, " ");
				break;
			default:	/* accelerated tab completion */
				n = 0;
				do {
					and = 0xff, nand = 0xff, i = 0;

					while (comm[i] != NULL && i < TH_INDEX ) {
						if (R_CHK(reg, i)) {
							and &= comm[i][n], nand &= ~comm[i][n];
						}
						i++;
					}
					n++;
				}
				while ((and | nand) == 0xff);

				s[0] = '\0';
				strncat(s, comm[lst], MIN(size - 2, n - 1));
				printf("\n");

				/* print the list: at the most TH_MAXTAB words */
				for (i = 0, n = 0; i < TH_INDEX && n < TH_MAXTAB ; i++)
					if (R_CHK(reg, i)) {
						printf("  %s\n", comm[i]);
						n++;	
					}
				if ( cnt > TH_MAXTAB)	
					printf("  ...\n"); 
			}
			break;
		case GETS_EUP:
			if ( hist_index < (TH_HISTSIZE - 2) && history[hist_index + 1] != NULL) {
				hist_index++;
				strscpy(s, (char *) history[hist_index]);
			} else
				BELL();
			break;
		case GETS_EDOWN:
			if ( hist_index > -1 ) {
				hist_index--;
				strscpy(s, (char *) history[hist_index]);
			} else {
				strscpy(s, NULL);
				BELL();
			}
		}
	}

	if (strlen(s) > 0)
		fifo_push(s);

	return strlen(s);
}


#ifdef TEST
char *comm[] = {"g", "gina", "greenhouse","ghetto", "ghosted", "ghastliness", "ghosts", "ghost", NULL };

int
main(int argc, char **argv)
{
	int i;
	char a[80];

	for (;;) {
		i = fgets_th(a, 80, "enter command # ", comm);
		printf("\n{\"%s\" %d}\n", a, i);
	}
}
#endif
