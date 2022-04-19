/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 1999 John D. Polstra
 * Copyright (c) 1999,2001 Peter Wemm <peter@FreeBSD.org>
 * All rights reserved.
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
 * $FreeBSD$
 */

#ifndef _SYS_LINKER_SET_H_
#define _SYS_LINKER_SET_H_

#ifndef _SYS_CDEFS_H_
#error this file needs sys/cdefs.h as a prerequisite
#endif

/*
 * The following macros are used to declare global sets of objects, which
 * are collected by the linker into a `linker_set' as defined below.
 * For ELF, this is done by constructing a separate segment for each set.
 */

#if defined(__powerpc64__) && (!defined(_CALL_ELF) || _CALL_ELF == 1)
/*
 * ELFv1 pointers to functions are actaully pointers to function
 * descriptors.
 *
 * Move the symbol pointer from ".text" to ".data" segment, to make
 * the GCC compiler happy:
 */
#define	__MAKE_SET_CONST
#else
#define	__MAKE_SET_CONST const
#endif
 /*
  * Private macros, not to be used outside this header file.
  */
#ifdef __GNUCLIKE___SECTION

  /*
   * The userspace address sanitizer inserts redzones around global variables,
   * violating the assumption that linker set elements are packed.
   */
#ifdef _KERNEL
#define	__NOASAN
#else
#define	__NOASAN	__nosanitizeaddress
#endif
#if defined(__APPLE__) && defined(__MACH__)
#define __MAKE_SET_QV(set, sym, qv)			\
	__GLOBL(__CONCAT(__start_set_,set));		\
	__GLOBL(__CONCAT(__stop_set_,set));		\
	static void const * qv				\
	__NOASAN					\
	__set_##set##_sym_##sym __section(set)	\
	__used = &(sym)
#define __MAKE_SET(set, sym)	__MAKE_SET_QV(set, sym, __MAKE_SET_CONST)

#define DEFINE_WSEC(x)
#else
#define __MAKE_SET_QV(set, sym, qv)			\
	__WEAK(__CONCAT(__start_set_,set));		\
	__WEAK(__CONCAT(__stop_set_,set));		\
	static void const * qv				\
	__NOASAN					\
	__set_##set##_sym_##sym __section("set_" #set)	\
	__used = &(sym)
#define __MAKE_SET(set, sym)	__MAKE_SET_QV(set, sym, __MAKE_SET_CONST)

#define DEFINE_WSEC(x)
#endif
#else /* !__GNUCLIKE___SECTION */
#ifdef _WIN32


/*_Pragma("section(\".set_sysinit_set\", read)")*/
#define _ST2(x)		.set_##x
#define SECSTR(x) __XSTRING(_ST2(x)), write, read

#define __WSECTION(x) section(x)
#define _WSEC(x) __WSECTION(SECSTR(x))
#define DEFINE_WSEC(x) _Pragma(__XSTRING(_WSEC(x)))
  /*
   * Private macros, not to be used outside this header file.
   */
#define __MAKE_SET_QV(set, sym, qv)			\
__GLOBL(__CONCAT(__start_set_,set));		\
__GLOBL(__CONCAT(__stop_set_,set));		\
DEFINE_WSEC(set) \
__declspec(allocate(__XSTRING(__CONCAT(.set_,set)))) static void const* qv __set_##set##_sym_##sym = &(sym)
#define __MAKE_SET(set, sym)	__MAKE_SET_QV(set, sym, __MAKE_SET_CONST)
#elif defined(__APPLE__) && defined(__MACH__)
#define __MAKE_SET_QV(set, sym, qv)			\
	__GLOBL(__CONCAT(__start_set_,set));		\
	__GLOBL(__CONCAT(__stop_set_,set));		\
	static void const * qv				\
	__NOASAN					\
	__set_##set##_sym_##sym __section("set_" #set)	\
	__used = &(sym)
#define __MAKE_SET(set, sym)	__MAKE_SET_QV(set, sym, __MAKE_SET_CONST)

#define DEFINE_WSEC(x)
#else
  /*
   * Private macros, not to be used outside this header file.
   */
#define __MAKE_SET_QV(set, sym, qv)			\
	__GLOBL(__CONCAT(__start_set_,set));		\
	__GLOBL(__CONCAT(__stop_set_,set));		\
	static void const * qv				\
	__set_##set##_sym_##sym = &(sym)
#define __MAKE_SET(set, sym)	__MAKE_SET_QV(set, sym, __MAKE_SET_CONST)
#endif
#endif /* __GNUCLIKE___SECTION */

/*
 * Public macros.
 */
#define TEXT_SET(set, sym)	__MAKE_SET(set, sym)
#define DATA_SET(set, sym)	__MAKE_SET(set, sym)
#define DATA_WSET(set, sym)	__MAKE_SET_QV(set, sym, )
#define BSS_SET(set, sym)	__MAKE_SET(set, sym)
#define ABS_SET(set, sym)	__MAKE_SET(set, sym)
#define SET_ENTRY(set, sym)	__MAKE_SET(set, sym)

/*
 * Initialize before referring to a given linker set.
 */
#define SET_DEFINE(set, ptype)					\
	ptype __CONCAT(d__start_set_,set);	\
	ptype __CONCAT(d__stop_set_,set);\
	ptype *__CONCAT(__start_set_,set)=&__CONCAT(d__start_set_, set);	\
	ptype *__CONCAT(__stop_set_,set) = &__CONCAT(d__start_set_, set)

#define SET_DECLARE(set, ptype)					\
	extern ptype *__CONCAT(__start_set_,set);	\
	extern ptype *__CONCAT(__stop_set_,set)

#define SET_BEGIN(set)							\
	(&__CONCAT(__start_set_,set))
#define SET_LIMIT(set)							\
	(&__CONCAT(__stop_set_,set))

/*
 * Iterate over all the elements of a set.
 *
 * Sets always contain addresses of things, and "pvar" points to words
 * containing those addresses.  Thus is must be declared as "type **pvar",
 * and the address of each set item is obtained inside the loop by "*pvar".
 */
#define SET_FOREACH(pvar, set)						\
	for (pvar = SET_BEGIN(set); pvar < SET_LIMIT(set); pvar++)

#define SET_ITEM(set, i)						\
	((SET_BEGIN(set))[i])

/*
 * Provide a count of the items in a set.
 */
#define SET_COUNT(set)							\
	(SET_LIMIT(set) - SET_BEGIN(set))

#endif	/* _SYS_LINKER_SET_H_ */
