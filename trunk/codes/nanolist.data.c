/*
 * $Id: nanolist.data.c,v 1.5 2005/07/19 17:57:19 awgn Exp $
 * nanolist- add/replace headers to local-delivery mails.
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * compile with:
 * gcc nanolist.data.c -fPIC -shared -o ml-conf.so  
 * 
 * Tested on some linux and BSD flavors
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

#include <stdlib.h>
#include "nanolist.h"

/***
 *** header: list of headers which will be changed in traversing mail 
 *** value : NULL -> means suppressed header
 ***/

struct tag headers[] = {
{ mode: replace, tag: "Reply-To:", value: "nanolist <xxx@antifork.org>" },
{ mode: replace, tag: "List-id:",  value: "xxx.antifork.org" },
{ mode: append , tag: "Subject:",  value: "[nanolist]" },
};

int headerlen=sizeof(headers)/sizeof(headers[0]);

/*** 
 *** account for mail delivery
 ***/

char mailto[]= "all";

/*** 
 *** command line used for local delivery. The last token must be NULL 
 ***/

char *args[] = { "/usr/sbin/sendmail", "-oem", mailto, NULL };


