/*
 *  fgets_th: fgets with tab completion and history
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

/* macro */

#define GETS_EOK         0
#define GETS_ETAB       -1
#define GETS_EUP        -2
#define GETS_EDOWN      -3
#define KEY_BS          0x08
#define KEY_DEL         0x7f
#define KEY_ESC         0x1b
#define KEY_BRACKET     0x5b
#define KEY_A           0x41
#define KEY_B           0x42
#define KEY_C           0x43
#define KEY_D           0x44
#define KEY_LN          '\n'
#define KEY_HT          '\t'

#define TH_HISTSIZE     100	/* history */
#define TH_INDEX	1024	/* number of commands indexed */
#define TH_MAXTAB	24	/* max number of wrods listed with tab matching */

#define bit     	>>3
#define R_SET(r,b)      ( r[b>>3] |=  1<<(b & 7) )
#define R_CLR(r,b)      ( r[b>>3] &= ~1<<(b & 7) )
#define R_CHK(r,b)      ( r[b>>3]  &  1<<(b & 7) )
#define R_NOT(r,b)      ( r[b>>3] ^=  1<<(b & 7) )

#define BELL()          ( {fputs("\a",  stdout); 0; })
#define ESC_UP()        (  fputs(esc_up,stdout)  )
#define ESC_DW()        (  fputs(esc_dw,stdout)  )
#define ESC_RG()        (  fputs(esc_rg,stdout)  )
#define ESC_LF()        (  fputs(esc_lf,stdout)  )
#define LINE_CL()       (  fputs(esc_kl,stdout)  )     // vt100 compliant

#define MIN(a,b)        ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

#define STRCPY(x,y)     ( (y != NULL) ? strcpy(x,y) : strcpy(x,"") )
#define DEC(x)          ( (x) > 0 ? (x--) : BELL() )
#define INC(x)          ( (x) < (SH_HISTSIZE-2) && history[x+1] != NULL ? (x++) : BELL() )

#define BACKSPACE() do {                                \
printf("\b \b"); fflush(stdout); } while(0)

#define RMSPACE(s) do {                                 \
int _i = strlen(s)-1;                                   \
while ( _i >= 0 && s[_i]== ' ' )                        \
s[_i--]=0; } while (0)

#define DELETE(x) do {                                  \
char *_p=(x);                                           \
while (*(_p+1)!=0) {                                    \
        *_p=*(_p+1);                                    \
        _p++;                                           \
}                                                       \
*_p=0;                                                  \
} while(0)

