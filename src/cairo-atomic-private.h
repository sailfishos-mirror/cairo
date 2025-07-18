/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2007 Chris Wilson
 * Copyright © 2010 Andrea Canciani
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Chris Wilson <chris@chris-wilson.co.uk>
 *	Andrea Canciani <ranma42@gmail.com>
 */

#ifndef CAIRO_ATOMIC_PRIVATE_H
#define CAIRO_ATOMIC_PRIVATE_H

#include "cairo-compiler-private.h"

#include "config.h"

#include <assert.h>

CAIRO_BEGIN_DECLS

#if HAVE_C11_ATOMIC_PRIMITIVES

#include <stdatomic.h>

#define HAS_ATOMIC_OPS 1

typedef atomic_int cairo_atomic_int_t;
typedef _Atomic(void *) cairo_atomic_intptr_t;

static cairo_always_inline int
_cairo_atomic_int_get (cairo_atomic_int_t *x)
{
    return atomic_load_explicit (x, memory_order_seq_cst);
}

static cairo_always_inline int
_cairo_atomic_int_get_relaxed (cairo_atomic_int_t *x)
{
    return atomic_load_explicit (x, memory_order_relaxed);
}

static cairo_always_inline void
_cairo_atomic_int_set_relaxed (cairo_atomic_int_t *x, int val)
{
    atomic_store_explicit (x, val, memory_order_relaxed);
}

static cairo_always_inline void *
_cairo_atomic_ptr_get (cairo_atomic_intptr_t *x)
{
    return atomic_load_explicit (x, memory_order_seq_cst);
}

# define _cairo_atomic_int_inc(x) ((void) atomic_fetch_add_explicit(x, 1, memory_order_seq_cst))
# define _cairo_atomic_int_dec(x) ((void) atomic_fetch_sub_explicit(x, 1, memory_order_seq_cst))
# define _cairo_atomic_int_dec_and_test(x) (atomic_fetch_sub_explicit(x, 1, memory_order_seq_cst) == 1)


static cairo_always_inline cairo_bool_t
_cairo_atomic_int_cmpxchg_impl(cairo_atomic_int_t *x,
			       int                oldv,
			       int                newv)
{
    int expected = oldv;
    return atomic_compare_exchange_strong_explicit (x, &expected, newv, memory_order_seq_cst, memory_order_seq_cst);
}

#define _cairo_atomic_int_cmpxchg(x, oldv, newv) \
  _cairo_atomic_int_cmpxchg_impl(x, oldv, newv)

static cairo_always_inline int
_cairo_atomic_int_cmpxchg_return_old_impl(cairo_atomic_int_t *x,
					  int                 oldv,
					  int                 newv)
{
    int expected = oldv;
    (void) atomic_compare_exchange_strong_explicit (x, &expected, newv, memory_order_seq_cst, memory_order_seq_cst);
    return expected;
}

#define _cairo_atomic_int_cmpxchg_return_old(x, oldv, newv) \
  _cairo_atomic_int_cmpxchg_return_old_impl(x, oldv, newv)

static cairo_always_inline cairo_bool_t
_cairo_atomic_ptr_cmpxchg_impl(cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    void *expected = oldv;
    return atomic_compare_exchange_strong_explicit (x, &expected, newv, memory_order_seq_cst, memory_order_seq_cst);
}

#define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
  _cairo_atomic_ptr_cmpxchg_impl(x, oldv, newv)

static cairo_always_inline void *
_cairo_atomic_ptr_cmpxchg_return_old_impl(cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    void *expected = oldv;
    (void) atomic_compare_exchange_strong_explicit (x, &expected, newv, memory_order_seq_cst, memory_order_seq_cst);
    return expected;
}

#define _cairo_atomic_ptr_cmpxchg_return_old(x, oldv, newv) \
  _cairo_atomic_ptr_cmpxchg_return_old_impl(x, oldv, newv)

#endif /* HAVE_C11_ATOMIC_PRIMITIVES */

/* C++11 atomic primitives were designed to be more flexible than the
 * __sync_* family of primitives.  Despite the name, they are available
 * in C as well as C++.  The motivating reason for using them is that
 * for _cairo_atomic_{int,ptr}_get, the compiler is able to see that
 * the load is intended to be atomic, as opposed to the __sync_*
 * version, below, where the load looks like a plain load.  Having
 * the load appear atomic to the compiler is particular important for
 * tools like ThreadSanitizer so they don't report false positives on
 * memory operations that we intend to be atomic.
 */
#if HAVE_CXX11_ATOMIC_PRIMITIVES

#define HAS_ATOMIC_OPS 1

typedef int cairo_atomic_int_t;
typedef intptr_t cairo_atomic_intptr_t;

static cairo_always_inline int
_cairo_atomic_int_get (cairo_atomic_int_t *x)
{
    return __atomic_load_n(x, __ATOMIC_SEQ_CST);
}

static cairo_always_inline int
_cairo_atomic_int_get_relaxed (cairo_atomic_int_t *x)
{
    return __atomic_load_n(x, __ATOMIC_RELAXED);
}

static cairo_always_inline void
_cairo_atomic_int_set_relaxed (cairo_atomic_int_t *x, int val)
{
    __atomic_store_n(x, val, __ATOMIC_RELAXED);
}

static cairo_always_inline void *
_cairo_atomic_ptr_get (cairo_atomic_intptr_t *x)
{
    return (void*)__atomic_load_n(x, __ATOMIC_SEQ_CST);
}

# define _cairo_atomic_int_inc(x) ((void) __atomic_fetch_add(x, 1, __ATOMIC_SEQ_CST))
# define _cairo_atomic_int_dec(x) ((void) __atomic_fetch_sub(x, 1, __ATOMIC_SEQ_CST))
# define _cairo_atomic_int_dec_and_test(x) (__atomic_fetch_sub(x, 1, __ATOMIC_SEQ_CST) == 1)

static cairo_always_inline cairo_bool_t
_cairo_atomic_int_cmpxchg_impl(cairo_atomic_int_t *x,
			       int                 oldv,
			       int                 newv)
{
    int expected = oldv;
    return __atomic_compare_exchange_n(x, &expected, newv, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#define _cairo_atomic_int_cmpxchg(x, oldv, newv) \
  _cairo_atomic_int_cmpxchg_impl(x, oldv, newv)

static cairo_always_inline int
_cairo_atomic_int_cmpxchg_return_old_impl(cairo_atomic_int_t *x,
					  int                 oldv,
					  int                 newv)
{
    int expected = oldv;
    (void) __atomic_compare_exchange_n(x, &expected, newv, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

#define _cairo_atomic_int_cmpxchg_return_old(x, oldv, newv) \
  _cairo_atomic_int_cmpxchg_return_old_impl(x, oldv, newv)

static cairo_always_inline cairo_bool_t
_cairo_atomic_ptr_cmpxchg_impl(cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    intptr_t expected = (intptr_t)oldv;
    return __atomic_compare_exchange_n(x, &expected, (intptr_t)newv, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
  _cairo_atomic_ptr_cmpxchg_impl(x, oldv, newv)

static cairo_always_inline void *
_cairo_atomic_ptr_cmpxchg_return_old_impl(cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    intptr_t expected = (intptr_t)oldv;
    (void) __atomic_compare_exchange_n(x, &expected, (intptr_t)newv, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return (void*)expected;
}

#define _cairo_atomic_ptr_cmpxchg_return_old(x, oldv, newv) \
  _cairo_atomic_ptr_cmpxchg_return_old_impl(x, oldv, newv)

#endif /* HAVE_CXX11_ATOMIC_PRIMITIVES */

#if HAVE_GCC_LEGACY_ATOMICS

#define HAS_ATOMIC_OPS 1

typedef int cairo_atomic_int_t;
typedef intptr_t cairo_atomic_intptr_t;

static cairo_always_inline int
_cairo_atomic_int_get (cairo_atomic_int_t *x)
{
    __sync_synchronize ();
    return *x;
}

static cairo_always_inline int
_cairo_atomic_int_get_relaxed (cairo_atomic_int_t *x)
{
    return *x;
}

static cairo_always_inline void
_cairo_atomic_int_set_relaxed (cairo_atomic_int_t *x, int val)
{
    *x = val;
}

static cairo_always_inline void *
_cairo_atomic_ptr_get (cairo_atomic_intptr_t *x)
{
    __sync_synchronize ();
    return (void*)*x;
}

# define _cairo_atomic_int_inc(x) ((void) __sync_fetch_and_add(x, 1))
# define _cairo_atomic_int_dec(x) ((void) __sync_fetch_and_add(x, -1))
# define _cairo_atomic_int_dec_and_test(x) (__sync_fetch_and_add(x, -1) == 1)
# define _cairo_atomic_int_cmpxchg(x, oldv, newv) __sync_bool_compare_and_swap (x, oldv, newv)
# define _cairo_atomic_int_cmpxchg_return_old(x, oldv, newv) __sync_val_compare_and_swap (x, oldv, newv)

# define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
    __sync_bool_compare_and_swap ((cairo_atomic_intptr_t*)x, (cairo_atomic_intptr_t)oldv, (cairo_atomic_intptr_t)newv)

# define _cairo_atomic_ptr_cmpxchg_return_old(x, oldv, newv) \
    _cairo_atomic_intptr_to_voidptr (__sync_val_compare_and_swap ((cairo_atomic_intptr_t*)x, (cairo_atomic_intptr_t)oldv, (cairo_atomic_intptr_t)newv))

#endif /* HAVE_GCC_LEGACY_ATOMICS */

#if HAVE_LIB_ATOMIC_OPS
#include <atomic_ops.h>

#define HAS_ATOMIC_OPS 1

typedef  AO_t cairo_atomic_int_t;

# define _cairo_atomic_int_get(x) (AO_load_full (x))
# define _cairo_atomic_int_get_relaxed(x) (AO_load_full (x))
# define _cairo_atomic_int_set_relaxed(x, val) (AO_store_full ((x), (val)))

# define _cairo_atomic_int_inc(x) ((void) AO_fetch_and_add1_full(x))
# define _cairo_atomic_int_dec(x) ((void) AO_fetch_and_sub1_full(x))
# define _cairo_atomic_int_dec_and_test(x) (AO_fetch_and_sub1_full(x) == 1)
# define _cairo_atomic_int_cmpxchg(x, oldv, newv) AO_compare_and_swap_full(x, oldv, newv)

typedef intptr_t cairo_atomic_intptr_t;

# define _cairo_atomic_ptr_get(x) _cairo_atomic_intptr_to_voidptr (AO_load_full (x))
# define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
    _cairo_atomic_int_cmpxchg ((cairo_atomic_intptr_t*)(x), (cairo_atomic_intptr_t)oldv, (cairo_atomic_intptr_t)newv)

#endif

#if HAVE_OS_ATOMIC_OPS
#include <libkern/OSAtomic.h>

#define HAS_ATOMIC_OPS 1

typedef int32_t cairo_atomic_int_t;
typedef intptr_t cairo_atomic_intptr_t;

# define _cairo_atomic_int_get(x) (OSMemoryBarrier(), *(x))
# define _cairo_atomic_int_get_relaxed(x) *(x)
# define _cairo_atomic_int_set_relaxed(x, val) *(x) = (val)

# define _cairo_atomic_int_inc(x) ((void) OSAtomicIncrement32Barrier (x))
# define _cairo_atomic_int_dec(x) ((void) OSAtomicDecrement32Barrier (x))
# define _cairo_atomic_int_dec_and_test(x) (OSAtomicDecrement32Barrier (x) == 0)
# define _cairo_atomic_int_cmpxchg(x, oldv, newv) OSAtomicCompareAndSwap32Barrier(oldv, newv, x)

#if SIZEOF_VOID_P==4
# define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
    OSAtomicCompareAndSwap32Barrier((cairo_atomic_intptr_t)oldv, (cairo_atomic_intptr_t)newv, (cairo_atomic_intptr_t *)x)

#elif SIZEOF_VOID_P==8
# define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) \
    OSAtomicCompareAndSwap64Barrier((cairo_atomic_intptr_t)oldv, (cairo_atomic_intptr_t)newv, (cairo_atomic_intptr_t *)x)

#else
#error No matching integer pointer type
#endif

# define _cairo_atomic_ptr_get(x) (OSMemoryBarrier(), *(x))

#endif /* HAVE_OS_ATOMIC_OPS */

#if !defined(HAS_ATOMIC_OPS) && defined(_WIN32)
#include <windows.h>

#define HAS_ATOMIC_OPS 1

typedef LONG cairo_atomic_int_t;
typedef PVOID cairo_atomic_intptr_t;

static cairo_always_inline int
_cairo_atomic_int_get (cairo_atomic_int_t *x)
{
    MemoryBarrier ();
    return *x;
}

# define _cairo_atomic_int_get_relaxed(x) *(x)
# define _cairo_atomic_int_set_relaxed(x, val) *(x) = (val)

# define _cairo_atomic_int_inc(x) ((void) InterlockedIncrement (x))
# define _cairo_atomic_int_dec(x) ((void) InterlockedDecrement (x))
# define _cairo_atomic_int_dec_and_test(x) (InterlockedDecrement (x) == 0)

static cairo_always_inline cairo_bool_t
_cairo_atomic_int_cmpxchg (cairo_atomic_int_t *x,
                           int                 oldv,
                           int                 newv)
{
    return InterlockedCompareExchange (x, (LONG)newv, (LONG)oldv) == oldv;
}

static cairo_always_inline void *
_cairo_atomic_ptr_get (cairo_atomic_intptr_t *x)
{
    MemoryBarrier ();
    return (void *) *x;
}

static cairo_always_inline cairo_bool_t
_cairo_atomic_ptr_cmpxchg (cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    return InterlockedCompareExchangePointer (x, newv, oldv) == oldv;
}

static cairo_always_inline void *
_cairo_atomic_ptr_cmpxchg_return_old (cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    return InterlockedCompareExchangePointer (x, newv, oldv);
}

#endif /* !defined(HAS_ATOMIC_OPS) && defined(_WIN32) */


#ifndef HAS_ATOMIC_OPS

typedef int cairo_atomic_int_t;
typedef intptr_t cairo_atomic_intptr_t;

cairo_private void
_cairo_atomic_int_inc (cairo_atomic_int_t *x);

#define _cairo_atomic_int_dec(x) _cairo_atomic_int_dec_and_test(x)

cairo_private cairo_bool_t
_cairo_atomic_int_dec_and_test (cairo_atomic_int_t *x);

cairo_private int
_cairo_atomic_int_cmpxchg_return_old_impl (cairo_atomic_int_t *x, int oldv, int newv);

cairo_private void *
_cairo_atomic_ptr_cmpxchg_return_old_impl (cairo_atomic_intptr_t *x, void *oldv, void *newv);

#define _cairo_atomic_int_cmpxchg_return_old(x, oldv, newv) _cairo_atomic_int_cmpxchg_return_old_impl (x, oldv, newv)
#define _cairo_atomic_ptr_cmpxchg_return_old(x, oldv, newv) _cairo_atomic_ptr_cmpxchg_return_old_impl (x, oldv, newv)

#ifdef ATOMIC_OP_NEEDS_MEMORY_BARRIER
cairo_private int
_cairo_atomic_int_get (cairo_atomic_int_t *x);
cairo_private int
_cairo_atomic_int_get_relaxed (cairo_atomic_int_t *x);
void
_cairo_atomic_int_set_relaxed (cairo_atomic_int_t *x, int val);
cairo_private void*
_cairo_atomic_ptr_get(cairo_atomic_intptr_t *x);
#else
# define _cairo_atomic_int_get(x) (*x)
# define _cairo_atomic_int_get_relaxed(x) (*x)
# define _cairo_atomic_int_set_relaxed(x, val) (*x) = (val)
# define _cairo_atomic_ptr_get(x) (*x)
#endif

#else

/* Workaround GCC complaining about casts */
static cairo_always_inline void *
_cairo_atomic_intptr_to_voidptr (cairo_atomic_intptr_t x)
{
  return (void *) x;
}

static cairo_always_inline int
_cairo_atomic_int_cmpxchg_return_old_fallback(cairo_atomic_int_t *x, int oldv, int newv)
{
    int curr;

    do {
        curr = _cairo_atomic_int_get (x);
    } while (curr == oldv && !_cairo_atomic_int_cmpxchg (x, oldv, newv));

    return curr;
}

static cairo_always_inline void *
_cairo_atomic_ptr_cmpxchg_return_old_fallback(cairo_atomic_intptr_t *x, void *oldv, void *newv)
{
    void *curr;

    do {
        curr = _cairo_atomic_ptr_get (x);
    } while (curr == oldv && !_cairo_atomic_ptr_cmpxchg (x, oldv, newv));

    return curr;
}
#endif

#ifndef _cairo_atomic_int_cmpxchg_return_old
#define _cairo_atomic_int_cmpxchg_return_old(x, oldv, newv) _cairo_atomic_int_cmpxchg_return_old_fallback (x, oldv, newv)
#endif

#ifndef _cairo_atomic_ptr_cmpxchg_return_old
#define _cairo_atomic_ptr_cmpxchg_return_old(x, oldv, newv) _cairo_atomic_ptr_cmpxchg_return_old_fallback (x, oldv, newv)
#endif

#ifndef _cairo_atomic_int_cmpxchg
#define _cairo_atomic_int_cmpxchg(x, oldv, newv) (_cairo_atomic_int_cmpxchg_return_old (x, oldv, newv) == oldv)
#endif

#ifndef _cairo_atomic_ptr_cmpxchg
#define _cairo_atomic_ptr_cmpxchg(x, oldv, newv) (_cairo_atomic_ptr_cmpxchg_return_old (x, oldv, newv) == oldv)
#endif

#define _cairo_atomic_uint_get(x) _cairo_atomic_int_get(x)
#define _cairo_atomic_uint_cmpxchg(x, oldv, newv) \
    _cairo_atomic_int_cmpxchg((cairo_atomic_int_t *)x, oldv, newv)

#define _cairo_status_set_error(status, err) do { \
    int ret__; \
    assert (err < CAIRO_STATUS_LAST_STATUS); \
    assert (sizeof(*status) == sizeof(cairo_atomic_int_t)); \
    /* hide compiler warnings about cairo_status_t != int (gcc treats its as \
     * an unsigned integer instead, and about ignoring the return value. */  \
    ret__ = _cairo_atomic_int_cmpxchg ((cairo_atomic_int_t *) status, CAIRO_STATUS_SUCCESS, err); \
    (void) ret__; \
} while (0)

#if defined (_WIN32)

#include <windows.h>

typedef INIT_ONCE cairo_atomic_once_t;
#define CAIRO_ATOMIC_ONCE_INIT INIT_ONCE_STATIC_INIT

static cairo_always_inline cairo_bool_t
_cairo_atomic_init_once_enter(cairo_atomic_once_t *once)
{
    BOOL pending;

    if (unlikely (!InitOnceBeginInitialize (once, 0, &pending, NULL))) {
        assert (0 && "InitOnceBeginInitialize failed");
    }

    if (likely (!pending))
        return 0;

    return 1;
}

static cairo_always_inline void
_cairo_atomic_init_once_leave(cairo_atomic_once_t *once)
{
    if (unlikely (!InitOnceComplete (once, 0, NULL))) {
        assert (0 && "InitOnceComplete failed");
    }
}

#else

typedef cairo_atomic_int_t cairo_atomic_once_t;

#define CAIRO_ATOMIC_ONCE_UNINITIALIZED (0)
#define CAIRO_ATOMIC_ONCE_INITIALIZING  (1)
#define CAIRO_ATOMIC_ONCE_INITIALIZED   (2)
#define CAIRO_ATOMIC_ONCE_INIT          CAIRO_ATOMIC_ONCE_UNINITIALIZED

static cairo_always_inline cairo_bool_t
_cairo_atomic_init_once_enter(cairo_atomic_once_t *once)
{
    if (likely(_cairo_atomic_int_get(once) == CAIRO_ATOMIC_ONCE_INITIALIZED))
	return 0;

    if (_cairo_atomic_int_cmpxchg(once,
				  CAIRO_ATOMIC_ONCE_UNINITIALIZED,
				  CAIRO_ATOMIC_ONCE_INITIALIZING))
	return 1;

    while (_cairo_atomic_int_get(once) != CAIRO_ATOMIC_ONCE_INITIALIZED) {}
    return 0;
}

static cairo_always_inline void
_cairo_atomic_init_once_leave(cairo_atomic_once_t *once)
{
    if (unlikely(!_cairo_atomic_int_cmpxchg(once,
					    CAIRO_ATOMIC_ONCE_INITIALIZING,
					    CAIRO_ATOMIC_ONCE_INITIALIZED)))
	assert (0 && "incorrect use of _cairo_atomic_init_once API (once != CAIRO_ATOMIC_ONCE_INITIALIZING)");
}

#endif /* !_WIN32 */

CAIRO_END_DECLS

#endif
