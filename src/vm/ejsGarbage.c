/**
    ejsGarbage.c - Generational garbage collector.

    This implements a generational, non-compacting, mark and sweep collection algorithm with fast slab object allocations.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/

#if EJS_MEMORY_DEBUG
    #define BREAKPOINT(bp)      breakpoint(bp);
    #define CHECK(bp)           ejsCheckBlock(bp)
#else /* !EJS_MEMORY_DEBUG */
    #define BREAKPOINT(bp)
    #define CHECK(bp)           
#endif

//  MOB - should be once in ejs.h
#define GET_BLK(ptr)            ((EjsMem*) (((char*) (ptr)) - sizeof(EjsMem)))
#define INIT_LIST(bp)           if (1) { bp->next = bp->prev = bp; } else

/***************************** Forward Declarations ***************************/

static void mark(Ejs *ejs, int generation);
static void markGlobal(Ejs *ejs, int generation);
static void resetMarks(Ejs *ejs);
static void sweep(Ejs *ejs, int generation);

/************************************* Code ***********************************/
/*
    Create the GC service
 */
int ejsCreateGCService(Ejs *ejs)
{
    EjsHeap     *heap;

    mprAssert(ejs);
    heap = ejs->heap;

#if UNUSED
    heap->firstGlobal = (ejs->empty) ? 0 : ES_global_NUM_CLASS_PROP;
//  MOB -- Some types are immutable App.config
heap->firstGlobal = 0;
#endif
    return 0;
}


void ejsDestroyGCService(Ejs *ejs)
{
    EjsHeap     *heap;
    EjsMem      *gp;
    EjsMem      *bp, *next;
    
    heap = ejs->heap;
    for (gp = heap->generations; gp < &heap->generations[EJS_MAX_GEN]; gp++) {
        for (bp = gp->next; bp != gp; bp = next) {
            next = bp->next;
            ejsFree(ejs, bp);
        }
    }
}




/*
    Collect the garbage. This is a mark and sweep over all possible objects. If an object is not referenced, it and 
    all contained properties will be freed. Collection is done in generations.
 */
void ejsCollectGarbage(Ejs *ejs, int gen)
{
    EjsHeap     *heap;
    
    heap = ejs->heap;
    if (!heap->enabled || heap->collecting || !ejs->initialized) {
        return;
    }
    heap->collecting = 1;
    mark(ejs, gen);
    sweep(ejs, gen);
    heap->workDone = 0;
    heap->gcRequired = 0;
    heap->collecting = 0;
    //  MOB
    heap->gen = &heap->generations[EJS_GEN_NEW];
}


/*
    Mark phase. Mark objects that are still in use and should not be collected.
 */
static void mark(Ejs *ejs, int generation)
{
    EjsModule       *mp;
    EjsHeap         *heap;
    EjsMem          *gp;
    EjsBlock        *block;
    EV              *vp, **sp, **top;
    EjsMem          *bp;
    int             next;

    heap = ejs->heap;
    heap->gen = &heap->generations[generation];

    resetMarks(ejs);
    markGlobal(ejs, generation);

    if (ejs->result) {
        ejsMark(ejs, ejs->result);
    }
    if (ejs->exception) {
        ejsMark(ejs, ejs->exception);
    }
    if (ejs->exceptionArg) {
        ejsMark(ejs, ejs->exceptionArg);
    }
    if (ejs->memoryCallback) {
        ejsMark(ejs, (EjsObj*) ejs->memoryCallback);
    }
    if (ejs->search) {
        ejsMark(ejs, ejs->search);
    }
    if (ejs->applications) {
        ejsMark(ejs, ejs->applications);
    }
    
    /*
        Mark initializers
     */
    for (next = 0; (mp = (EjsModule*) ejsGetNextItem(ejs, ejs->modules, &next)) != 0;) {
        if (mp->initializer) {
            ejsMark(ejs, mp->initializer);
        }
    }

    /*
        Mark blocks. This includes frames and blocks.
     */
    for (block = ejs->state->bp; block; block = block->prev) {
        ejsMark(ejs, block);
    }

    /*
        Mark the evaluation stack
     */
    top = ejs->state->stack;
    for (sp = ejs->state->stackBase; sp <= top; sp++) {
        if ((vp = *sp) != NULL) {
            ejsMark(ejs, vp);
        }
    }

    /*
        Mark all permanent
     */
    for (gp = heap->generations; gp < &heap->generations[EJS_MAX_GEN]; gp++) {
        for (bp = gp->next; bp != gp; bp = bp->next) {
            if (!bp->visited && bp->permanent) {
                ejsMark(ejs, bp);
            }
        }
    }
}


/*
    Sweep up the garbage for a given generation
 */
static void sweep(Ejs *ejs, int maxGeneration)
{
    EjsHeap     *heap;
    EjsMem      *gp, *bp;
    
    heap = ejs->heap;
    for (gp = &heap->generations[EJS_MAX_GEN - 1]; gp >= heap->generations; gp--) {
        for (bp = gp->next; bp != gp; bp = bp->next) {
            if (!bp->visited && !bp->permanent) {
                ejsFree(ejs, bp);
            }
        }
    }
}


//  MOB -- is slow to have to do this twice
/*
    Reset all marks prior to doing a mark/sweep
 */
static void resetMarks(Ejs *ejs)
{
    EjsHeap     *heap;
    EjsBlock    *block, *b;
    EjsMem      *gp, *bp;

    heap = ejs->heap;
    for (gp = heap->generations; gp < &heap->generations[EJS_MAX_GEN]; gp++) {
        for (bp = gp->next; bp != gp; bp = bp->next) {
            bp->visited = 0;
        }
    }
    for (block = ejs->state->bp; block; block = block->prev) {
        GET_BLK(block)->visited = 0;
        if (block->prevException) {
            GET_BLK(block->prevException)->visited = 0;
        }
        for (b = block->scope; b; b = b->scope) {
            GET_BLK(b)->visited = 0;
        }
    }
}

    
static void markGlobal(Ejs *ejs, int generation)
{
#if MOB
    EjsHeap     *heap;
    EjsObj      *obj;
    EjsBlock    *block;
    EjsObj      *item;
    MprHash     *hp;
    int         i, next;

    heap = ejs->heap;

    obj = (EjsObj*) ejs->global;
    GET_BLK(obj)->visited = 1;

    if (generation == EJS_GEN_ETERNAL) {
        for (i = 0; i < obj->numSlots; i++) {
            ejsMark(ejs, obj->slots[i].value.ref);
        }
        for (hp = 0; (hp = mprGetNextHash(ejs->standardSpaces, hp)) != 0; ) {
            ejsMark(ejs, (void*) hp->data);
        }

    } else {
        i = heap->firstGlobal;
        for (; i < obj->numSlots; i++) {
            ejsMark(ejs, obj->slots[i].value.ref);
        }
    }
    block = ejs->globalBlock;
    if (block->prevException) {
        ejsMark(ejs, block->prevException);
    }
    if (block->namespaces.length > 0) {
        for (next = 0; ((item = (EjsObj*) ejsGetNextItem(&block->namespaces, &next)) != 0); ) {
            ejsMark(ejs, item);
        }
    }
#endif
}


/*
    Mark a variable as used. All variable marking comes through here.
 */
void ejsMark(Ejs *ejs, void *ptr)
{
    EjsMem      *bp;

    if (ptr) {
        bp = GET_BLK(ptr);
        if (bp && !bp->visited) {
            CHECK(bp);
            bp->visited = 1;
            if (bp->hasType) {
                (bp->type->helpers.mark)(ejs, ptr);
                if (!GET_BLK(bp->type)->visited) {
                    ejsMark(ejs, bp->type);
                }
            } else if (bp->manager) {
                (bp->manager)(ejs, ptr, EJS_MANAGE_MARK);
            }
#if BLD_DEBUG
            if (!GET_BLK(bp->name)->visited) {
                ejsMark(ejs, bp->name);
            }
#endif
        }
    }
}


/*
    Make all eternal allocations permanent. This prevents an eternal GC from collecting core essential values like
    ejs->zeroValue. Do this to keep markGlobal() simple, otherwise it would have to enumerate values like this.
 */
void ejsMakeEternalPermanent(Ejs *ejs)
{
    EjsMem      *gp, *bp;

    gp = &ejs->heap->generations[EJS_GEN_ETERNAL];
    for (bp = gp->next; bp != gp; bp = bp->next) {
        bp->permanent = 1;
    }
}


void ejsMakePermanent(Ejs *ejs, void *ptr)
{
    GET_BLK(ptr)->permanent = 1;
}


void ejsMakeTransient(Ejs *ejs, void *ptr)
{
    GET_BLK(ptr)->permanent = 0;
}


/*
    Return true if there is time to do a garbage collection and if we will benefit from it.
    Currently not used.
 */
int ejsIsTimeForGC(Ejs *ejs, int timeTillNextEvent)
{
    EjsHeap       *heap;

    if (timeTillNextEvent < EJS_MIN_TIME_FOR_GC) {
        /*
            This is a heuristic where we want a good amount of idle time so that a proactive garbage collection won't 
            delay any I/O events.
         */
        return 0;
    }

    /*
        Return if we haven't done enough work to warrant a collection. Trigger a little short of the work quota to try 
        to run GC before a demand allocation requires it.
     */
    heap = ejs->heap;
    if (!heap->enabled || heap->workDone < (heap->workQuota - EJS_SHORT_WORK_QUOTA)) {
        return 0;
    }
    mprLog(ejs, 7, "Time for GC. Work done %d, time till next event %d", heap->workDone, timeTillNextEvent);
    return 1;
}


int ejsEnableGC(Ejs *ejs, bool on)
{
    int     old;

    old = ejs->heap->enabled;
    ejs->heap->enabled = on;
    return old;
}


/*
    On a memory allocation failure, go into graceful degrade mode. Set all slab allocation chunk increments to 1 
    so we can create an exception block to throw.
 */
void ejsGracefulDegrade(Ejs *ejs)
{
    mprLog(ejs, 1, "WARNING: Memory almost depleted. In graceful degrade mode");
    ejs->heap->degraded = 1;
    mprSignalExit(ejs);
}


int ejsSetGeneration(Ejs *ejs, int generation)
{
    EjsHeap     *heap;
    EjsMem      *gen;
    
    mprAssert(0 <= generation && generation < EJS_MAX_GEN);
    if (generation < 0 || generation >= EJS_MAX_GEN) {
        return -1;
    }
    heap = ejs->heap;
    gen = heap->gen;
    heap->gen = &heap->generations[generation];
    return gen - heap->generations;;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    @end
 */
