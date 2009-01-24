/*
 * Copyright (c) 2006 Nicola Bonelli <bonelli@antifork.org>
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
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus 
#include <iostream>
#endif

#define __INIT_LOGGER__	{ -2 }

struct debug_msg {

	int	level;

#if defined(__cplusplus) && defined(EBUG)  
	debug_msg() { 
		char *r = getenv("LOG_LEVEL"); 
		if (r!= NULL) { 
			level = atoi(r) & 0x7; 
		}
		else {
			level = -1; 
		}
		std::cout << __PRETTY_FUNCTION__ << ": LOG_LEVEL=" << level << std::endl; 
	};
#endif

};

#ifdef __cplusplus
extern debug_msg __logger__;
#else
extern struct debug_msg __logger__;

#ifdef EBUG
static void __debug_msg() __attribute__((constructor));
static void __debug_msg() {

	if ( __logger__.level != -2 )
		return;

	char *r = getenv("LOG_LEVEL"); 
	if (r!= NULL) { 
		__logger__.level = atoi(r) & 0x7; 
	}
	else {
		__logger__.level = -1; 
	}
	fprintf(stdout,"%s: LOG_LEVEL=%d\n",__PRETTY_FUNCTION__,__logger__.level);
}
#endif
#endif

#ifdef EBUG
#if defined (__cplusplus) 

#define LOG_EMERG(x)		do { if ( 0 <= __logger__.level ) (x); } while (0)
#define LOG_ALER(x)		do { if ( 1 <= __logger__.level ) (x); } while (0)
#define LOG_CRIT(x)		do { if ( 2 <= __logger__.level ) (x); } while (0)
#define LOG_ERR(x)		do { if ( 3 <= __logger__.level ) (x); } while (0)
#define LOG_WARNING(x)		do { if ( 4 <= __logger__.level ) (x); } while (0)
#define LOG_NOTICE(x)		do { if ( 5 <= __logger__.level ) (x); } while (0)
#define LOG_INFO(x)		do { if ( 6 <= __logger__.level ) (x); } while (0)
#define LOG_DEBUG(x)		do { if ( 7 <= __logger__.level ) (x); } while (0)

#else

#define LOG_EMERG(x,f, ...)    	do { if ( 0 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_ALER(x,f,...)	do { if ( 1 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)    
#define LOG_CRIT(x,f,...)	do { if ( 2 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_ERR(x,f,...)	do { if ( 3 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_WARNING(x,f,...)	do { if ( 4 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_NOTICE(x,f,...)	do { if ( 5 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_INFO(x,f,...)	do { if ( 6 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)
#define LOG_DEBUG(x,f,...)	do { if ( 7 <= __logger__.level ) fprintf (x, f, ## __VA_ARGS__); } while (0)

#endif
#else

#define LOG_EMERG(x,...) 	do { } while(0)	  
#define LOG_ALER(x,...) 	do { } while(0)    
#define LOG_CRIT(x,...)     	do { } while(0)	
#define LOG_ERR(x,...)      	do { } while(0)	
#define LOG_WARNING(x,...)  	do { } while(0)	
#define LOG_NOTICE(x,...)   	do { } while(0)
#define LOG_INFO(x,...)     	do { } while(0)
#define LOG_DEBUG(x,...)    	do { } while(0)

#endif
#endif /* _DEBUG_H_ */
