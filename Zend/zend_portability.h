/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Zend Technologies Ltd. (http://www.zend.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@php.net>                                 |
   |          Zeev Suraski <zeev@php.net>                                 |
   |          Dmitry Stogov <dmitry@php.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_PORTABILITY_H
#define ZEND_PORTABILITY_H

#ifdef __cplusplus
#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }
#else
#define BEGIN_EXTERN_C()
#define END_EXTERN_C()
#endif

/*
 * general definitions
 */

#ifdef ZEND_WIN32
# include "zend_config.w32.h"
# define ZEND_PATHS_SEPARATOR		';'
#elif defined(__riscos__)
# include <zend_config.h>
# define ZEND_PATHS_SEPARATOR		';'
#else
# include <zend_config.h>
# define ZEND_PATHS_SEPARATOR		':'
#endif

#include "../TSRM/TSRM.h"

#include <stdio.h>
#if ZEND_DEBUG && defined(NDEBUG)
# error "NDEBUG must not be defined when ZEND_DEBUG is enabled"
#endif
#include <assert.h>
#include <math.h>

#ifdef HAVE_UNIX_H
# include <unix.h>
#endif

#include <stdarg.h>
#include <stddef.h>

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#include <limits.h>

#if defined(ZEND_WIN32) && !defined(__clang__)
#include <intrin.h>
#endif

#include "zend_range_check.h"

/* GCC x.y.z supplies __GNUC__ = x and __GNUC_MINOR__ = y */
#ifdef __GNUC__
# define ZEND_GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
# define ZEND_GCC_VERSION 0
#endif

/* Compatibility with non-clang compilers */
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif
#ifndef __has_feature
# define __has_feature(x) 0
#endif

#if defined(ZEND_WIN32) && !defined(__clang__)
# define ZEND_ASSUME(c)	__assume(c)
#elif defined(__clang__) && __has_builtin(__builtin_assume)
# pragma clang diagnostic ignored "-Wassume"
# define ZEND_ASSUME(c)	__builtin_assume(c)
#elif defined(PHP_HAVE_BUILTIN_UNREACHABLE) && defined(PHP_HAVE_BUILTIN_EXPECT)
# define ZEND_ASSUME(c)	do { \
		if (__builtin_expect(!(c), 0)) __builtin_unreachable(); \
	} while (0)
#else
# define ZEND_ASSUME(c)
#endif

#if ZEND_DEBUG
# define ZEND_ASSERT(c)	assert(c)
#else
# define ZEND_ASSERT(c) ZEND_ASSUME(c)
#endif

/* use C23 unreachable() from <stddef.h> if possible */
#ifdef unreachable
# define _ZEND_UNREACHABLE() unreachable()
#elif defined(PHP_HAVE_BUILTIN_UNREACHABLE)
# define _ZEND_UNREACHABLE() __builtin_unreachable()
#else
# define _ZEND_UNREACHABLE() ZEND_ASSUME(0)
#endif

#if ZEND_DEBUG
# define ZEND_UNREACHABLE() do {ZEND_ASSERT(0); _ZEND_UNREACHABLE();} while (0)
#else
# define ZEND_UNREACHABLE() _ZEND_UNREACHABLE()
#endif

/* pseudo fallthrough keyword; */
#if defined(__GNUC__) && __GNUC__ >= 7
# define ZEND_FALLTHROUGH __attribute__((__fallthrough__))
#else
# define ZEND_FALLTHROUGH ((void)0)
#endif

/* Only use this macro if you know for sure that all of the switches values
   are covered by its case statements */
#define EMPTY_SWITCH_DEFAULT_CASE() default: ZEND_UNREACHABLE(); break;

#if defined(__GNUC__) && __GNUC__ >= 4
# define ZEND_IGNORE_VALUE(x) (({ __typeof__ (x) __x = (x); (void) __x; }))
#else
# define ZEND_IGNORE_VALUE(x) ((void) (x))
#endif

#define zend_quiet_write(...) ZEND_IGNORE_VALUE(write(__VA_ARGS__))

/* all HAVE_XXX test have to be after the include of zend_config above */

#if defined(HAVE_LIBDL) && !defined(ZEND_WIN32)

# if __has_feature(address_sanitizer) && !defined(__SANITIZE_ADDRESS__)
#  define __SANITIZE_ADDRESS__
# endif

# ifndef RTLD_LAZY
#  define RTLD_LAZY 1    /* Solaris 1, FreeBSD's (2.1.7.1 and older) */
# endif

# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif

# ifdef PHP_USE_RTLD_NOW
#  define PHP_RTLD_MODE  RTLD_NOW
# else
#  define PHP_RTLD_MODE  RTLD_LAZY
# endif

# if defined(RTLD_GROUP) && defined(RTLD_WORLD) && defined(RTLD_PARENT)
#  define DL_LOAD(libname)			dlopen(libname, PHP_RTLD_MODE | RTLD_GLOBAL | RTLD_GROUP | RTLD_WORLD | RTLD_PARENT)
# elif defined(RTLD_DEEPBIND) && !defined(__SANITIZE_ADDRESS__) && !__has_feature(memory_sanitizer)
#  if defined(LM_ID_NEWLM)
     ZEND_API extern bool zend_dl_use_deepbind;
#    define DL_LOAD(libname)			dlopen(libname, PHP_RTLD_MODE | RTLD_GLOBAL | (zend_dl_use_deepbind ? RTLD_DEEPBIND : 0))
#  else
#    define DL_LOAD(libname)			dlopen(libname, PHP_RTLD_MODE | RTLD_GLOBAL | RTLD_DEEPBIND)
#  endif
# else
#  define DL_LOAD(libname)			dlopen(libname, PHP_RTLD_MODE | RTLD_GLOBAL)
# endif
# define DL_UNLOAD					dlclose
# if defined(DLSYM_NEEDS_UNDERSCORE)
#  define DL_FETCH_SYMBOL(h,s)		dlsym((h), "_" s)
# else
#  define DL_FETCH_SYMBOL			dlsym
# endif
# define DL_ERROR					dlerror
# define DL_HANDLE					void *
# define ZEND_EXTENSIONS_SUPPORT	1
#elif defined(ZEND_WIN32)
# define DL_LOAD(libname)			LoadLibrary(libname)
# define DL_FETCH_SYMBOL			GetProcAddress
# define DL_UNLOAD					FreeLibrary
# define DL_HANDLE					HMODULE
# define ZEND_EXTENSIONS_SUPPORT	1
#else
# define DL_HANDLE					void *
# define ZEND_EXTENSIONS_SUPPORT	0
#endif

#if defined(HAVE_ALLOCA_H) && !defined(_ALLOCA_H)
# include <alloca.h>
#endif
/* AIX requires this to be the first thing in the file.  */
#ifndef __GNUC__
# ifndef HAVE_ALLOCA_H
#  ifdef _AIX
#   pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca();
#   endif
#  endif
# endif
#endif

#if !ZEND_DEBUG && (defined(HAVE_ALLOCA) || (defined (__GNUC__) && __GNUC__ >= 2)) && !(defined(ZTS) && defined(HPUX)) && !defined(__APPLE__)
# define ZEND_ALLOCA_MAX_SIZE (32 * 1024)
# define ALLOCA_FLAG(name) \
	bool name;
# define SET_ALLOCA_FLAG(name) \
	name = true
# define do_alloca_ex(size, limit, use_heap) \
	((use_heap = (UNEXPECTED((size) > (limit)))) ? emalloc(size) : alloca(size))
# define do_alloca(size, use_heap) \
	do_alloca_ex(size, ZEND_ALLOCA_MAX_SIZE, use_heap)
# define free_alloca(p, use_heap) \
	do { if (UNEXPECTED(use_heap)) efree(p); } while (0)
#else
# define ALLOCA_FLAG(name)
# define SET_ALLOCA_FLAG(name)
# define do_alloca(p, use_heap)		emalloc(p)
# define free_alloca(p, use_heap)	efree(p)
#endif

#if ZEND_GCC_VERSION >= 2096 || __has_attribute(__malloc__)
# define ZEND_ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#elif defined(ZEND_WIN32)
# define ZEND_ATTRIBUTE_MALLOC __declspec(allocator) __declspec(restrict)
#else
# define ZEND_ATTRIBUTE_MALLOC
#endif

#if ZEND_GCC_VERSION >= 4003 || __has_attribute(alloc_size)
# define ZEND_ATTRIBUTE_ALLOC_SIZE(X) __attribute__ ((alloc_size(X)))
# define ZEND_ATTRIBUTE_ALLOC_SIZE2(X,Y) __attribute__ ((alloc_size(X,Y)))
#else
# define ZEND_ATTRIBUTE_ALLOC_SIZE(X)
# define ZEND_ATTRIBUTE_ALLOC_SIZE2(X,Y)
#endif

#if ZEND_GCC_VERSION >= 3000
# define ZEND_ATTRIBUTE_CONST __attribute__((const))
#else
# define ZEND_ATTRIBUTE_CONST
#endif

#if ZEND_GCC_VERSION >= 2007 || __has_attribute(format)
# define ZEND_ATTRIBUTE_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define ZEND_ATTRIBUTE_FORMAT(type, idx, first)
#endif

#if (ZEND_GCC_VERSION >= 3001 && !defined(__INTEL_COMPILER)) || __has_attribute(format)
# define ZEND_ATTRIBUTE_PTR_FORMAT(type, idx, first) __attribute__ ((format(type, idx, first)))
#else
# define ZEND_ATTRIBUTE_PTR_FORMAT(type, idx, first)
#endif

#if ZEND_GCC_VERSION >= 3001 || __has_attribute(deprecated)
# define ZEND_ATTRIBUTE_DEPRECATED  __attribute__((deprecated))
#elif defined(ZEND_WIN32)
# define ZEND_ATTRIBUTE_DEPRECATED  __declspec(deprecated)
#else
# define ZEND_ATTRIBUTE_DEPRECATED
#endif

#if ZEND_GCC_VERSION >= 4003 || __has_attribute(unused)
# define ZEND_ATTRIBUTE_UNUSED __attribute__((unused))
#else
# define ZEND_ATTRIBUTE_UNUSED
#endif

#if ZEND_GCC_VERSION >= 3003 || __has_attribute(nonnull)
/* All pointer arguments must be non-null */
# define ZEND_ATTRIBUTE_NONNULL  __attribute__((nonnull))
/* Specified arguments must be non-null (1-based) */
# define ZEND_ATTRIBUTE_NONNULL_ARGS(...)  __attribute__((nonnull(__VA_ARGS__)))
#else
# define ZEND_ATTRIBUTE_NONNULL
# define ZEND_ATTRIBUTE_NONNULL_ARGS(...)
#endif

#if defined(__GNUC__) && ZEND_GCC_VERSION >= 4003
# define ZEND_COLD __attribute__((cold))
# ifdef __OPTIMIZE__
#  define ZEND_OPT_SIZE  __attribute__((optimize("Os")))
#  define ZEND_OPT_SPEED __attribute__((optimize("Ofast")))
# else
#  define ZEND_OPT_SIZE
#  define ZEND_OPT_SPEED
# endif
#else
# define ZEND_COLD
# define ZEND_OPT_SIZE
# define ZEND_OPT_SPEED
#endif

#if defined(__GNUC__) && ZEND_GCC_VERSION >= 5000
# define ZEND_ATTRIBUTE_UNUSED_LABEL __attribute__((unused));
# define ZEND_ATTRIBUTE_COLD_LABEL __attribute__((cold));
#else
# define ZEND_ATTRIBUTE_UNUSED_LABEL
# define ZEND_ATTRIBUTE_COLD_LABEL
#endif

#if defined(__GNUC__) && ZEND_GCC_VERSION >= 3004 && defined(__i386__)
# define ZEND_FASTCALL __attribute__((fastcall))
#elif defined(_MSC_VER)
# define ZEND_FASTCALL __vectorcall
#else
# define ZEND_FASTCALL
#endif

#if (defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(__APPLE__) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__)) || __has_attribute(noreturn)
# define HAVE_NORETURN
# define ZEND_NORETURN __attribute__((noreturn))
#elif defined(ZEND_WIN32)
# define HAVE_NORETURN
# define ZEND_NORETURN __declspec(noreturn)
#else
# define ZEND_NORETURN
#endif

#if __has_attribute(force_align_arg_pointer)
# define ZEND_STACK_ALIGNED __attribute__((force_align_arg_pointer))
#else
# define ZEND_STACK_ALIGNED
#endif

#if (defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(__APPLE__) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__))
# define HAVE_NORETURN_ALIAS
# define HAVE_ATTRIBUTE_WEAK
#endif

#if ZEND_GCC_VERSION >= 3001 || __has_builtin(__builtin_constant_p)
# define HAVE_BUILTIN_CONSTANT_P
#endif

#if __has_attribute(counted_by)
#define ZEND_ELEMENT_COUNT(m) __attribute__((counted_by(m)))
#else
#define ZEND_ELEMENT_COUNT(m)
#endif

#ifdef HAVE_BUILTIN_CONSTANT_P
# define ZEND_CONST_COND(_condition, _default) \
	(__builtin_constant_p(_condition) ? (_condition) : (_default))
#else
# define ZEND_CONST_COND(_condition, _default) \
	(_default)
#endif

#if ZEND_DEBUG || defined(ZEND_WIN32_NEVER_INLINE)
# define zend_always_inline inline
# define zend_never_inline
#else
# if defined(__GNUC__)
#  if __GNUC__ >= 3
#   define zend_always_inline inline __attribute__((always_inline))
#   define zend_never_inline __attribute__((noinline))
#  else
#   define zend_always_inline inline
#   define zend_never_inline
#  endif
# elif defined(_MSC_VER)
#  define zend_always_inline __forceinline
#  define zend_never_inline __declspec(noinline)
# else
#  if __has_attribute(always_inline)
#   define zend_always_inline inline __attribute__((always_inline))
#  else
#   define zend_always_inline inline
#  endif
#  if __has_attribute(noinline)
#   define zend_never_inline __attribute__((noinline))
#  else
#   define zend_never_inline
#  endif
# endif
#endif /* ZEND_DEBUG */

#ifdef PHP_HAVE_BUILTIN_EXPECT
# define EXPECTED(condition)   __builtin_expect(!!(condition), 1)
# define UNEXPECTED(condition) __builtin_expect(!!(condition), 0)
#else
# define EXPECTED(condition)   (condition)
# define UNEXPECTED(condition) (condition)
#endif

#ifndef XtOffsetOf
# define XtOffsetOf(s_type, field) offsetof(s_type, field)
#endif

#ifndef ZEND_WIN32
# define SETJMP(a) sigsetjmp(a, 0)
# define LONGJMP(a,b) siglongjmp(a, b)
# define JMP_BUF sigjmp_buf
#else
# define SETJMP(a) setjmp(a)
# define LONGJMP(a,b) longjmp(a, b)
# define JMP_BUF jmp_buf
#endif

#if ZEND_DEBUG
# define ZEND_FILE_LINE_D				const char *__zend_filename, const uint32_t __zend_lineno
# define ZEND_FILE_LINE_DC				, ZEND_FILE_LINE_D
# define ZEND_FILE_LINE_ORIG_D			const char *__zend_orig_filename, const uint32_t __zend_orig_lineno
# define ZEND_FILE_LINE_ORIG_DC			, ZEND_FILE_LINE_ORIG_D
# define ZEND_FILE_LINE_RELAY_C			__zend_filename, __zend_lineno
# define ZEND_FILE_LINE_RELAY_CC		, ZEND_FILE_LINE_RELAY_C
# define ZEND_FILE_LINE_C				__FILE__, __LINE__
# define ZEND_FILE_LINE_CC				, ZEND_FILE_LINE_C
# define ZEND_FILE_LINE_EMPTY_C			NULL, 0
# define ZEND_FILE_LINE_EMPTY_CC		, ZEND_FILE_LINE_EMPTY_C
# define ZEND_FILE_LINE_ORIG_RELAY_C	__zend_orig_filename, __zend_orig_lineno
# define ZEND_FILE_LINE_ORIG_RELAY_CC	, ZEND_FILE_LINE_ORIG_RELAY_C
#else
# define ZEND_FILE_LINE_D				void
# define ZEND_FILE_LINE_DC
# define ZEND_FILE_LINE_ORIG_D			void
# define ZEND_FILE_LINE_ORIG_DC
# define ZEND_FILE_LINE_RELAY_C
# define ZEND_FILE_LINE_RELAY_CC
# define ZEND_FILE_LINE_C
# define ZEND_FILE_LINE_CC
# define ZEND_FILE_LINE_EMPTY_C
# define ZEND_FILE_LINE_EMPTY_CC
# define ZEND_FILE_LINE_ORIG_RELAY_C
# define ZEND_FILE_LINE_ORIG_RELAY_CC
#endif	/* ZEND_DEBUG */

#if ZEND_DEBUG
# define Z_DBG(expr)		(expr)
#else
# define Z_DBG(expr)
#endif

#ifdef ZTS
# define ZTS_V 1
#else
# define ZTS_V 0
#endif

#define MAX_LENGTH_OF_DOUBLE 32

#undef MIN
#undef MAX
#define MAX(a, b)  (((a)>(b))?(a):(b))
#define MIN(a, b)  (((a)<(b))?(a):(b))

#define ZEND_BIT_TEST(bits, bit) \
	(((bits)[(bit) / (sizeof((bits)[0])*8)] >> ((bit) & (sizeof((bits)[0])*8-1))) & 1)

#define ZEND_INFINITY INFINITY

#define ZEND_NAN NAN

#if defined(__cplusplus) && __cplusplus >= 201103L
extern "C++" {
# include <cmath>
}
# define zend_isnan std::isnan
# define zend_isinf std::isinf
# define zend_finite std::isfinite
#else
# include <math.h>
# define zend_isnan(a) isnan(a)
# define zend_isinf(a) isinf(a)
# define zend_finite(a) isfinite(a)
#endif

#define ZEND_STRL(str)		(str), (sizeof(str)-1)
#define ZEND_STRS(str)		(str), (sizeof(str))
#define ZEND_NORMALIZE_BOOL(n)			\
	((n) ? (((n)<0) ? -1 : 1) : 0)
#define ZEND_TRUTH(x)		((x) ? 1 : 0)
#define ZEND_LOG_XOR(a, b)		(ZEND_TRUTH(a) ^ ZEND_TRUTH(b))

/**
 * Do a three-way comparison of two integers and returns -1, 0 or 1
 * depending on whether #a is smaller, equal or larger than #b.
 */
#define ZEND_THREEWAY_COMPARE(a, b) ((a) == (b) ? 0 : ((a) < (b) ? -1 : 1))

#define ZEND_MAX_RESERVED_RESOURCES	6

/* excpt.h on Digital Unix 4.0 defines function_table */
#undef function_table

#ifdef ZEND_WIN32
#define ZEND_SECURE_ZERO(var, size) RtlSecureZeroMemory((var), (size))
#elif defined(HAVE_MEMSET_EXPLICIT)
#define ZEND_SECURE_ZERO(var, size) memset_explicit((var), 0, (size))
#else
#define ZEND_SECURE_ZERO(var, size) explicit_bzero((var), (size))
#endif

/* This check should only be used on network socket, not file descriptors */
#ifdef ZEND_WIN32
#define ZEND_VALID_SOCKET(sock) (INVALID_SOCKET != (sock))
#else
#define ZEND_VALID_SOCKET(sock) ((sock) >= 0)
#endif

/* Intrinsics macros start. */

/* Memory sanitizer is incompatible with ifunc resolvers. Even if the resolver
 * is marked as no_sanitize("memory") it will still be instrumented and crash. */
#if __has_feature(memory_sanitizer) || __has_feature(thread_sanitizer) || \
	__has_feature(dataflow_sanitizer)
# undef HAVE_FUNC_ATTRIBUTE_IFUNC
#endif

#if __has_feature(memory_sanitizer)
# include <sanitizer/msan_interface.h>
# define MSAN_UNPOISON(value) __msan_unpoison(&(value), sizeof(value))
#else
# define MSAN_UNPOISON(value)
#endif

/* Only use ifunc resolvers if we have __builtin_cpu_supports() and __builtin_cpu_init(),
 * otherwise the use of zend_cpu_supports() may not be safe inside ifunc resolvers. */
#if defined(HAVE_FUNC_ATTRIBUTE_IFUNC) && defined(HAVE_FUNC_ATTRIBUTE_TARGET) && \
	defined(PHP_HAVE_BUILTIN_CPU_SUPPORTS) && defined(PHP_HAVE_BUILTIN_CPU_INIT)
# define ZEND_INTRIN_HAVE_IFUNC_TARGET 1
#endif

#if (defined(__i386__) || defined(__x86_64__))
# if defined(HAVE_TMMINTRIN_H)
#  define PHP_HAVE_SSSE3
# endif

# if defined(HAVE_NMMINTRIN_H)
#  define PHP_HAVE_SSE4_2
# endif

# if defined(HAVE_WMMINTRIN_H)
#  define PHP_HAVE_PCLMUL
# endif

/*
 * AVX2 support was added in gcc 4.7, but AVX2 intrinsics don't work in
 * __attribute__((target("avx2"))) functions until gcc 4.9.
 */
# if defined(HAVE_IMMINTRIN_H) && \
  (defined(__llvm__) || defined(__clang__) || (defined(__GNUC__) && ZEND_GCC_VERSION >= 4009))
#  define PHP_HAVE_AVX2
# endif
#endif

#ifdef __SSSE3__
/* Instructions compiled directly. */
# define ZEND_INTRIN_SSSE3_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(PHP_HAVE_SSSE3)) || (defined(ZEND_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define ZEND_INTRIN_SSSE3_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(ZEND_INTRIN_SSSE3_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET)
# define ZEND_INTRIN_SSSE3_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_SSSE3_RESOLVER)
# define ZEND_INTRIN_SSSE3_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_SSSE3_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_SSSE3_FUNC_DECL(func) ZEND_API func __attribute__((target("ssse3")))
# else
#  define ZEND_INTRIN_SSSE3_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_SSSE3_FUNC_DECL(func)
#endif

#ifdef __SSE4_2__
/* Instructions compiled directly. */
# define ZEND_INTRIN_SSE4_2_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(PHP_HAVE_SSE4_2)) || (defined(ZEND_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define ZEND_INTRIN_SSE4_2_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(ZEND_INTRIN_SSE4_2_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET)
# define ZEND_INTRIN_SSE4_2_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_SSE4_2_RESOLVER)
# define ZEND_INTRIN_SSE4_2_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_SSE4_2_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_SSE4_2_FUNC_DECL(func) ZEND_API func __attribute__((target("sse4.2")))
# else
#  define ZEND_INTRIN_SSE4_2_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_SSE4_2_FUNC_DECL(func)
#endif

#ifdef __PCLMUL__
/* Instructions compiled directly. */
# define ZEND_INTRIN_PCLMUL_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(PHP_HAVE_PCLMUL)) || (defined(ZEND_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define ZEND_INTRIN_PCLMUL_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(ZEND_INTRIN_PCLMUL_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET) && (!defined(__GNUC__) || (ZEND_GCC_VERSION >= 9000))
/* __builtin_cpu_supports has pclmul from gcc9 */
# define ZEND_INTRIN_PCLMUL_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_PCLMUL_RESOLVER)
# define ZEND_INTRIN_PCLMUL_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_PCLMUL_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_PCLMUL_FUNC_DECL(func) ZEND_API func __attribute__((target("pclmul")))
# else
#  define ZEND_INTRIN_PCLMUL_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_PCLMUL_FUNC_DECL(func)
#endif

#if defined(ZEND_INTRIN_SSE4_2_NATIVE) && defined(ZEND_INTRIN_PCLMUL_NATIVE)
/* Instructions compiled directly. */
# define ZEND_INTRIN_SSE4_2_PCLMUL_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(PHP_HAVE_SSE4_2) && defined(PHP_HAVE_PCLMUL)) || (defined(ZEND_WIN32) && (!defined(_M_ARM64)))
/* Function resolved by ifunc or MINIT. */
# define ZEND_INTRIN_SSE4_2_PCLMUL_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(ZEND_INTRIN_SSE4_2_PCLMUL_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET) && (!defined(__GNUC__) || (ZEND_GCC_VERSION >= 9000))
/* __builtin_cpu_supports has pclmul from gcc9 */
# define ZEND_INTRIN_SSE4_2_PCLMUL_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_SSE4_2_PCLMUL_RESOLVER)
# define ZEND_INTRIN_SSE4_2_PCLMUL_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_SSE4_2_PCLMUL_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func) ZEND_API func __attribute__((target("sse4.2,pclmul")))
# else
#  define ZEND_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_SSE4_2_PCLMUL_FUNC_DECL(func)
#endif

#ifdef __AVX2__
# define ZEND_INTRIN_AVX2_NATIVE 1
#elif (defined(HAVE_FUNC_ATTRIBUTE_TARGET) && defined(PHP_HAVE_AVX2)) || (defined(ZEND_WIN32) && (!defined(_M_ARM64)))
# define ZEND_INTRIN_AVX2_RESOLVER 1
#endif

/* Do not use for conditional declaration of API functions! */
#if defined(ZEND_INTRIN_AVX2_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET)
# define ZEND_INTRIN_AVX2_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_AVX2_RESOLVER)
# define ZEND_INTRIN_AVX2_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_AVX2_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_AVX2_FUNC_DECL(func) ZEND_API func __attribute__((target("avx2")))
# else
#  define ZEND_INTRIN_AVX2_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_AVX2_FUNC_DECL(func)
#endif

#if defined(PHP_HAVE_AVX512_SUPPORTS) && defined(HAVE_FUNC_ATTRIBUTE_TARGET) || defined(ZEND_WIN32)
#define ZEND_INTRIN_AVX512_RESOLVER 1
#endif

#if defined(ZEND_INTRIN_AVX512_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET)
# define ZEND_INTRIN_AVX512_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_AVX512_RESOLVER)
# define ZEND_INTRIN_AVX512_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_AVX512_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_AVX512_FUNC_DECL(func) ZEND_API func __attribute__((target("avx512f,avx512cd,avx512vl,avx512dq,avx512bw")))
# else
#  define ZEND_INTRIN_AVX512_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_AVX512_FUNC_DECL(func)
#endif

#if defined(PHP_HAVE_AVX512_VBMI_SUPPORTS) && defined(HAVE_FUNC_ATTRIBUTE_TARGET)
#define ZEND_INTRIN_AVX512_VBMI_RESOLVER 1
#endif

#if defined(ZEND_INTRIN_AVX512_VBMI_RESOLVER) && defined(ZEND_INTRIN_HAVE_IFUNC_TARGET)
# define ZEND_INTRIN_AVX512_VBMI_FUNC_PROTO 1
#elif defined(ZEND_INTRIN_AVX512_VBMI_RESOLVER)
# define ZEND_INTRIN_AVX512_VBMI_FUNC_PTR 1
#endif

#ifdef ZEND_INTRIN_AVX512_VBMI_RESOLVER
# ifdef HAVE_FUNC_ATTRIBUTE_TARGET
#  define ZEND_INTRIN_AVX512_VBMI_FUNC_DECL(func) ZEND_API func __attribute__((target("avx512f,avx512cd,avx512vl,avx512dq,avx512bw,avx512vbmi")))
# else
#  define ZEND_INTRIN_AVX512_VBMI_FUNC_DECL(func) func
# endif
#else
# define ZEND_INTRIN_AVX512_VBMI_FUNC_DECL(func)
#endif

/* Intrinsics macros end. */

#ifdef ZEND_WIN32
# define ZEND_SET_ALIGNED(alignment, decl) __declspec(align(alignment)) decl
#elif defined(HAVE_ATTRIBUTE_ALIGNED)
# define ZEND_SET_ALIGNED(alignment, decl) decl __attribute__ ((__aligned__ (alignment)))
#else
# define ZEND_SET_ALIGNED(alignment, decl) decl
#endif

#define ZEND_SLIDE_TO_ALIGNED(alignment, ptr) (((uintptr_t)(ptr) + ((alignment)-1)) & ~((alignment)-1))
#define ZEND_SLIDE_TO_ALIGNED16(ptr) ZEND_SLIDE_TO_ALIGNED(Z_UL(16), ptr)

#ifdef ZEND_WIN32
# define _ZEND_EXPAND_VA(a) a
# define ZEND_EXPAND_VA(code) _ZEND_EXPAND_VA(code)
#else
# define ZEND_EXPAND_VA(code) code
#endif

/* On CPU with few registers, it's cheaper to reload value then use spill slot */
#if defined(__i386__) || (defined(_WIN32) && !defined(_WIN64))
# define ZEND_PREFER_RELOAD
#endif

#if defined(ZEND_WIN32) && defined(_DEBUG)
# define ZEND_IGNORE_LEAKS_BEGIN() _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_ALLOC_MEM_DF)
# define ZEND_IGNORE_LEAKS_END() _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF)
#else
# define ZEND_IGNORE_LEAKS_BEGIN()
# define ZEND_IGNORE_LEAKS_END()
#endif

/* MSVC yields C4090 when a (const T **) is passed to a (void *); ZEND_VOIDP works around that */
#ifdef _MSC_VER
# define ZEND_VOIDP(ptr) ((void *) ptr)
#else
# define ZEND_VOIDP(ptr) (ptr)
#endif

#if __has_attribute(__indirect_return__)
# define ZEND_INDIRECT_RETURN __attribute__((__indirect_return__))
#else
# define ZEND_INDIRECT_RETURN
#endif

#if __has_attribute(nonstring) && defined(__GNUC__) && ((!defined(__clang__) && __GNUC__ >= 15) || (defined(__clang_major__) && __clang_major__ >= 20))
# define ZEND_NONSTRING __attribute__((nonstring))
#else
# define ZEND_NONSTRING
#endif

#define __ZEND_DO_PRAGMA(x) _Pragma(#x)
#define _ZEND_DO_PRAGMA(x) __ZEND_DO_PRAGMA(x)
#if defined(__clang__)
# define ZEND_DIAGNOSTIC_IGNORED_START(warning) \
	_Pragma("clang diagnostic push") \
	_ZEND_DO_PRAGMA(clang diagnostic ignored warning)
# define ZEND_DIAGNOSTIC_IGNORED_END \
	_Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
# define ZEND_DIAGNOSTIC_IGNORED_START(warning) \
	_Pragma("GCC diagnostic push") \
	_ZEND_DO_PRAGMA(GCC diagnostic ignored warning)
# define ZEND_DIAGNOSTIC_IGNORED_END \
	_Pragma("GCC diagnostic pop")
#else
# define ZEND_DIAGNOSTIC_IGNORED_START(warning)
# define ZEND_DIAGNOSTIC_IGNORED_END
#endif

/** @deprecated */
#define ZEND_CGG_DIAGNOSTIC_IGNORED_START ZEND_DIAGNOSTIC_IGNORED_START
/** @deprecated */
#define ZEND_CGG_DIAGNOSTIC_IGNORED_END ZEND_DIAGNOSTIC_IGNORED_END

#if defined(__cplusplus)
# define ZEND_STATIC_ASSERT(c, m) static_assert((c), m)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) /* C11 */
# define ZEND_STATIC_ASSERT(c, m) _Static_assert((c), m)
#else
# define ZEND_STATIC_ASSERT(c, m)
#endif

#if ((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) /* C11 */ \
  || (defined(__cplusplus) && __cplusplus >= 201103L) /* C++11 */) && !defined(ZEND_WIN32)
typedef max_align_t zend_max_align_t;
#else
typedef union {
	char c;
	short s;
	int i;
	long l;
#if SIZEOF_LONG_LONG
	long long ll;
#endif
	float f;
	double d;
	long double ld;
	void *p;
	void (*fun)(void);
} zend_max_align_t;
#endif

/* Bytes swap */
#ifdef _MSC_VER
#  include <stdlib.h>
#  define ZEND_BYTES_SWAP32(u) _byteswap_ulong(u)
#  define ZEND_BYTES_SWAP64(u) _byteswap_uint64(u)
#elif defined(HAVE_BYTESWAP_H)
#  include <byteswap.h>
#  define ZEND_BYTES_SWAP32(u) bswap_32(u)
#  define ZEND_BYTES_SWAP64(u) bswap_64(u)
#elif defined(HAVE_SYS_BSWAP_H)
#  include <sys/bswap.h>
#  define ZEND_BYTES_SWAP32(u) bswap32(u)
#  define ZEND_BYTES_SWAP64(u) bswap64(u)
#elif defined(__GNUC__)
#  define ZEND_BYTES_SWAP32(u) __builtin_bswap32(u)
#  define ZEND_BYTES_SWAP64(u) __builtin_bswap64(u)
#elif defined(__has_builtin)
#  if __has_builtin(__builtin_bswap32)
#    define ZEND_BYTES_SWAP32(u) __builtin_bswap32(u)
#  endif
#  if __has_builtin(__builtin_bswap64)
#    define ZEND_BYTES_SWAP64(u) __builtin_bswap64(u)
#  endif
#endif

#ifndef ZEND_BYTES_SWAP32
static zend_always_inline uint32_t ZEND_BYTES_SWAP32(uint32_t u)
{
  return (((u & 0xff000000) >> 24)
          | ((u & 0x00ff0000) >>  8)
          | ((u & 0x0000ff00) <<  8)
          | ((u & 0x000000ff) << 24));
}
#endif
#ifndef ZEND_BYTES_SWAP64
static zend_always_inline uint64_t ZEND_BYTES_SWAP64(uint64_t u)
{
   return (((u & 0xff00000000000000ULL) >> 56)
          | ((u & 0x00ff000000000000ULL) >> 40)
          | ((u & 0x0000ff0000000000ULL) >> 24)
          | ((u & 0x000000ff00000000ULL) >>  8)
          | ((u & 0x00000000ff000000ULL) <<  8)
          | ((u & 0x0000000000ff0000ULL) << 24)
          | ((u & 0x000000000000ff00ULL) << 40)
          | ((u & 0x00000000000000ffULL) << 56));
}
#endif

#ifdef ZEND_WIN32
/* Whether it's allowed to reattach to a shm segment from different processes on
 * this platform. This prevents pointing to internal structures from shm due to
 * ASLR. Currently only possible on Windows. */
# define ZEND_OPCACHE_SHM_REATTACHMENT 1
#endif

#endif /* ZEND_PORTABILITY_H */
