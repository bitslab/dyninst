/*
 * Copyright (c) 1996 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 *
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 *
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 *
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/************************************************************************
 *
 * RTthread-solaris.c: platform dependent runtime instrumentation functions for threads
 *
 ************************************************************************/

#if !defined(ONE_THREAD)

#ifdef SHM_SAMPLING
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
 
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/sys.h"

#include <thread.h>
#include <sys/lwp.h>
#include <stdlib.h>
#include "RTthread.h"


/* The following must be cleanned up */
typedef struct {
        long    sp;
        long    pc;
        long    fsr;
        long    fpu_en;
        long    g2;
        long    g3;
        long    g4;
} resumestate_t;

/*
 * thread stack layout.
 *
 *      -----------------       high address
 *      |               |
 *      | struct thread |
 *      |               |
 *      |               |
 *      |               |
 *      |       tls     |
 *      |               |
 *      -----------------
 *      |               | <-    thread stack bottom
 *      |               |
 *      |               |
 *      |               |
 *      |               |
 *      -----------------       low address
 */

typedef struct uthread {
        struct uthread   *t_link;  /* run/sleep queue */
        char            *t_stk;         /* stack base */
        unsigned int    t_stksize;      /* size of stack */
        char            *t_tls;         /* pointer to thread local storage */
        resumestate_t   t_resumestate;  /* any extra state needed by resume */
        long            t_startpc;      /* start func called by thr_create() */
        thread_t        t_tid;          /* thread id */
        lwpid_t         t_lwpid;        /* lwp id */
        int             t_usropts;      /* usr options, (THR_BOUND, ...) */

} uthread_t ;

extern void* DYNINST_allthreads_p ;
void idtot(int tid) {
  if ( DYNINST_allthreads_p ) {
    uthread_t *ct = (uthread_t*) DYNINST_idtot(tid, DYNINST_allthreads_p);
    fprintf(stderr, "stk=0x%x, startpc=0x%x, lwpid=0x%x, resumestate=0x%x",
	    ct->t_stk, ct->t_startpc, ct->t_lwpid, &(ct->t_resumestate));
  }
}

void DYNINST_ThreadPInfo(
    void* tls,
    void** stackbase, 
    int* tidp, 
    long *startpc, 
    int* lwpidp,
    void** resumestate_p) {
  uthread_t *ptr = (uthread_t *) tls ;
  *stackbase = (void*) (ptr->t_stk);
  *tidp = (int) ptr->t_tid ;
  *startpc = ptr->t_startpc ;
  *lwpidp = ptr->t_lwpid ;
  *resumestate_p = &(ptr->t_resumestate);
/* fprintf(stderr, "------ tid=%d, stk=0x%x, stksize=0x%x\n",  ptr->t_tid, ptr->t_stk, ptr->t_stksize); */
}

int DYNINST_ThreadInfo(void** stackbase, 
int* tidp, 
long *startpc, 
int* lwpidp,
void** resumestate_p) {
  extern uthread_t *DYNINST_curthread(void) ;
  uthread_t *curthread ;
  if ( (curthread = DYNINST_curthread()) ) {
    DYNINST_ThreadPInfo((void*)curthread,stackbase,tidp,startpc,lwpidp,resumestate_p);
    return 1 ;
  }
  return 0;
}

#endif /* !defined(ONE_THREAD) */
