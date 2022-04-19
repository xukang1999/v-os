/*-
 * Copyright (c) 1986, 1988, 1991, 1993
 *  The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)subr_prf.c  8.3 (Berkeley) 1/21/94
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/kdb.h>
#include <sys/mutex.h>
#include <sys/sx.h>
#include <sys/kernel.h>
#include <sys/msgbuf.h>
#include <sys/malloc.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/stddef.h>
#include <sys/sysctl.h>
#include <sys/tty.h>
#include <sys/syslog.h>
#include <sys/cons.h>
#include <sys/uio.h>
#include <sys/ctype.h>
#include <sys/sbuf.h>

#ifdef DDB
#include <ddb/ddb.h>
#endif

/*
 * Note that stdarg.h and the ANSI style va_start macro is used for both
 * ANSI and traditional C compilers.
 */
#include <machine/stdarg.h>


#include "context.h"

#define TOCONS    0x01
#define TOTTY    0x02
#define TOLOG    0x04
int ctx_putchar(int c);
int ctx_puts(const char* str);
/* Max number conversion buffer length: a u_quad_t in base 2, plus NUL byte. */
#define MAXNBUF    (sizeof(intmax_t) * NBBY + 1)
struct msgbuf* msgbufp;
struct putchar_arg {
    int flags;
    int pri;
    struct tty *tty;
    char *p_bufr;
    size_t n_bufr;
    char *p_next;
    size_t remain;
};

struct snprintf_arg {
    char *str;
    size_t remain;
};

int ctx_putchar(int c)
{
    return context_putchar(c);
}

int ctx_puts(const char* str)
{
    return context_puts(str);
}

static char *ksprintn(char *nbuf, uintmax_t num, int base, int *len, int upper);

/*
 * Put a NUL-terminated ASCII number (base <= 36) in a buffer in reverse
 * order; return an optional length and a pointer to the last character
 * written in the buffer (i.e., the first character of the string).
 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
static char *
ksprintn(char *nbuf, uintmax_t num, int base, int *lenp, int upper)
{
    char *p, c;

    p = nbuf;
    *p = '\0';
    do {
        c = hex2ascii(num % base);
        *++p = upper ? toupper(c) : c;
    } while (num /= base);
    if (lenp)
        *lenp = p - nbuf;
    return (p);
}

static void
putbuf(int c, struct putchar_arg *ap)
{
    /* Check if no console output buffer was provided. */
    if (ap->p_bufr == NULL) {
        /* Output direct to the console. */
        if (ap->flags & TOCONS)
            ctx_putchar(c);

    } else {
        /* Buffer the character: */
        *ap->p_next++ = c;
        ap->remain--;

        /* Always leave the buffer zero terminated. */
        *ap->p_next = '\0';

        /* Check if the buffer needs to be flushed. */
        if (ap->remain == 2 || c == '\n') {

            if (ap->flags & TOCONS) {
                ctx_puts(ap->p_bufr);
            }

            ap->p_next = ap->p_bufr;
            ap->remain = ap->n_bufr;
            *ap->p_next = '\0';
        }

        /*
         * Since we fill the buffer up one character at a time,
         * this should not happen.  We should always catch it when
         * ap->remain == 2 (if not sooner due to a newline), flush
         * the buffer and move on.  One way this could happen is
         * if someone sets PRINTF_BUFR_SIZE to 1 or something
         * similarly silly.
         */
        KASSERT(ap->remain > 2, ("Bad buffer logic, remain = %zd",
            ap->remain));
    }
}

/*
 * Print a character on console or users terminal.  If destination is
 * the console then the last bunch of characters are saved in msgbuf for
 * inspection later.
 */
static void
kputchar(int c, void *arg)
{
    struct putchar_arg *ap = (struct putchar_arg*) arg;
    int flags = ap->flags;
    int putbuf_done = 0;

    if (flags & TOCONS) {
        putbuf(c, ap);
        putbuf_done = 1;
    }

    if ((flags & TOLOG) && (putbuf_done == 0)) {
        if (c != '\0')
            putbuf(c, ap);
    }
}


#ifndef _MSC_VER
#if defined(__APPLE__) && defined(__MACH__)

#else
/*
 * Scaled down version of snprintf(3).
 */
int
snprintf(char* str, size_t size, const char* format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = vsnprintf(str, size, format, ap);
    va_end(ap);
    return(retval);
}
#endif
#else
int
snprintf(char* str, size_t size, const char* format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = vsnprintf(str, size, format, ap);
    va_end(ap);
    return(retval);
}
#endif

#if 0

int	sscanf(const char* a, char const* b _Nonnull, ...)
{
    return 0;
}

int _vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    return 0;
}
#endif
extern void
snprintf_func(int ch, void* arg);
int
_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    struct snprintf_arg info;
    int retval;

    info.str = str;
    info.remain = size;
    retval = kvprintf(format, snprintf_func, &info, 10, ap);
    if (info.remain >= 1)
        *info.str++ = '\0';
    return (retval);
}
