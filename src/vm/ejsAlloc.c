/**
    ejsAlloc.c - EJS Garbage collector.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

//  MOB -- rename to not clash with all-in-one with MPR

#define GET_MEM(ptr)            ((EjsMem*) (((char*) (ptr)) - sizeof(EjsMem)))
#define GET_PTR(mp)             ((char*) (((char*) (mp)) + sizeof(EjsMem)))
#define GET_NEXT(mp)            ((mp)->last) ? NULL : ((EjsMem*) ((char*) mp + mp->size))
#define GET_USIZE(mp)           (mp->size - sizeof(EjsMem))
#define INIT_LIST(mp)           if (1) { mp->next = mp->prev = mp; } else

#if EJS_MEMORY_DEBUG
    #define BREAKPOINT(mp)      breakpoint(mp);
    #define CHECK(mp)           ejsCheckBlock(mp)
    #define RESET_MEM(mp)       if (1) { memset(GET_PTR(mp), 0xFE, mp->size - sizeof(EjsMem)); } else
    #define SET_MAGIC(mp)       if (1) { (mp)->magic = EJS_ALLOC_MAGIC; } else
    #define SET_SEQ(mp)         if (1) { (mp)->seqno = heap->nextSeqno++; } else
    #define INIT_BLK(mp, len)   if (1) { SET_MAGIC(mp); SET_SEQ(mp); mp->size = len; } else
    #define VALID_BLK(mp)       ejsIsBlkValid(mp)

/*
    Set this address to break when this address is allocated or freed
 */
static EjsMem *stopAlloc = 0;
static int stopSeqno = -1;

#else /* !EJS_MEMORY_DEBUG */
    #define BREAKPOINT(mp)
    #define CHECK(mp)           
    #define RESET_MEM(mp)           
    #define SET_MAGIC(mp)
    #define SET_SEQ(mp)           
    #define INIT_BLK(mp, len)       if (1) { mp->size = len; } else
    #define VALID_BLK(mp)           1
#endif

#if EJS_MEMORY_STATS
    #define ADD(field, amt)         if (1) { heap->field += amt; } else 
    #define INC(field)              if (1) { heap->field++; } else 
    #define SUB(field, amt)         if (1) { heap->field -= amt; mprAssert(heap->field >= 0); } else 
#else
    #define ADD(field, amt)
    #define INC(field)
    #define SUB(field)
#endif

#if !MACOSX && !FREEBSD
    #define NEED_FFSL 1
    #if WIN
    #elif BLD_HOST_CPU_ARCH == MPR_CPU_IX86 || BLD_HOST_CPU_ARCH == MPR_CPU_IX64
        #define USE_FFSL_ASM_X86 1
    #endif
    static inline int ffsl(ulong word);
    static inline int flsl(ulong word);
#elif BSD_EMULATION
    #define ffsl FFSL
    #define flsl FLSL
    #define NEED_FFSL 1
    #define USE_FFSL_ASM_X86 1
    static inline int ffsl(ulong word);
    static inline int flsl(ulong word);
#endif

#define percent(a,b) ((int) ((a) * 100 / (b)))

/***************************** Forward Declarations ***************************/

//  MOB -- rename to not clash with MPR
static void deq(EjsMem *mp);
static void destroyHeap(EjsHeap *heap);
static void enq(EjsHeap *heap, EjsMem *mp); 
static int getQueueIndex(EjsHeap *heap, size_t size, int roundup);
static EjsMem *growHeap(EjsHeap *heap, size_t size);
static void initFree(EjsHeap *heap);
static void initGen(EjsHeap *heap);
static void initType(EjsHeap *heap);
static void linkBlock(EjsMem *head, EjsMem *mp);
static EjsMem *searchFree(EjsHeap *heap, size_t size, int *indexp);
static EjsMem *splitBlock(EjsHeap *heap, EjsMem *mp, size_t required, int qspare);
static void unlinkBlock(EjsMem *mp);

#if BLD_CC_MMU && !BLD_WIN_LIKE
static void virtFree(EjsHeap *heap, EjsMem *mp);
#endif

#if EJS_MEMORY_DEBUG
    static void breakpoint(EjsMem *mp);
#endif

/************************************* Code ***********************************/

int ejsCreateAllocService(Ejs *ejs)
{
    EjsHeap     *heap;

    mprAssert(ejs);

    if ((heap = mprAllocObj(ejs, EjsHeap, destroyHeap)) == NULL) {
        return MPR_ERR_NO_MEMORY;
    }
    ejs->heap = heap;
    heap->ejs = ejs;
    heap->enabled = 1;
    heap->regionSize = EJS_REGION_MIN_SIZE;
    heap->pageSize = mprGetPageSize();
    heap->workQuota = EJS_WORK_QUOTA;
    initFree(heap);
    initGen(heap);
    initType(heap);
    return 0;
}


static void destroyHeap(EjsHeap *heap)
{
    //  Should unpin all memory - can this auto happed via deleting heap?
    mprFree(heap);
}


void *ejsAlloc(Ejs *ejs, size_t usize)
{
    EjsMem      *mp;
    EjsHeap     *heap;
    size_t      maxBlock, size;
    int         bucket, group, index;

    heap = ejs->heap;
    size = EJS_ALLOC_ALIGN(usize + sizeof(EjsMem));
    if ((mp = searchFree(heap, size, &index)) == NULL) {
        if ((mp = growHeap(heap, size)) == NULL) {
            ejsThrowMemoryError(ejs);
            return NULL;
        }
        index = getQueueIndex(heap, size, 1);
    }
    BREAKPOINT(mp);
    mprAssert(mp->size >= size);
    if (mp->size >= (size + EJS_ALLOC_MIN_SPLIT)) {
        group = index / EJS_ALLOC_NUM_BUCKETS;
        bucket = index % EJS_ALLOC_NUM_BUCKETS;
        maxBlock = (((size_t) 1 ) << group | (((size_t) bucket) << (max(0, group - 1)))) << EJS_ALIGN_SHIFT;
        maxBlock += sizeof(EjsMem);
        if (mp->size > maxBlock) {
            splitBlock(heap, mp, size, 1);
        }
    }
#if UNUSED
    mp->type = heap->type;
#endif
    memset(GET_PTR(mp), 0, usize);
    linkBlock(heap->gen, mp);
    CHECK(mp);
    INC(requests);
    ADD(bytesAllocated, mp->size);
    SUB(bytesFree, mp->size);
    //  MOB -- how to do this less often. Perhaps push into searchFree
    if (++heap->workDone >= heap->workQuota && !heap->gcRequired && heap->enabled) {
        heap->gcRequired = 1;
        ejsAttention(ejs);
    }
    return GET_PTR(mp);
}


void ejsFree(Ejs *ejs, void *ptr)
{
    EjsHeap     *heap;
    EjsMem      *mp, *prev, *next, *after;
    size_t      size;

    heap = ejs->heap;

    if (unlikely(ptr == 0)) {
        return;
    }
    mp = GET_MEM(ptr);
    CHECK(mp);
    BREAKPOINT(mp);
    mprAssert(!mp->free);

    if (mp->hasType) {
        if (mp->type->helpers.destroy) {
            (mp->type->helpers.destroy)(ejs, ptr);
        }
    } else if (mp->manager) {
        (*mp->manager)(ejs, ptr, EJS_MANAGE_FREE);
    }
    unlinkBlock(mp);
    size = mp->size;
    RESET_MEM(mp);

    /*
        Coalesce with next if it is also free.
     */
    next = GET_NEXT(mp);
    if (next && next->free) {
        BREAKPOINT(next);
        deq(next);
        if ((after = GET_NEXT(next)) != NULL) {
            mprAssert(after->prior == next);
            after->prior = mp;
        } else {
            mp->last = 1;
        }
        size += next->size;
        mp->size = size;
        INC(joins);
    }
    /*
        Coalesce with previous if it is also free.
     */
    prev = mp->prior;
    if (prev && prev->free) {
        BREAKPOINT(prev);
        deq(prev);
        if ((after = GET_NEXT(mp)) != NULL) {
            mprAssert(after->prior == mp);
            after->prior = prev;
        } else {
            prev->last = 1;
        }
        size += prev->size;
        prev->size = size;
        mp = prev;
        INC(joins);
    }
#if EJS_MEMORY_DEBUG
    if ((after = GET_NEXT(mp)) != 0) {
        mprAssert(after->prior == mp);
    }
#endif
    SUB(bytesAllocated, mp->size);
    ADD(bytesFree, mp->size);

#if BLD_CC_MMU && !BLD_WIN_LIKE
    //  MOB -- do we want to ever do this?
    /*
        Windows can't easily release portions of a prior virtual allocation. You can decommit memory on windows, 
        but the pages are still part of the virtual address space -- so this will result in a loss of virtual space.
     */
    if (mp->size >= EJS_ALLOC_RETURN && heap->bytesFree > (EJS_REGION_MIN_SIZE * 4)) {
        virtFree(heap, mp);
    } else
#endif
    {
        enq(heap, mp);
    }
}


/*
    This duplicates an MPR block and frees it
 */
void *ejsAllocAndFree(Ejs *ejs, void *ptr, size_t len)
{
    void    *result;

    if (len < 0) {
        len = strlen(ptr);
    }
    if ((result = ejsAlloc(ejs, len)) == NULL) {
        return NULL;
    }
    memcpy(result, ptr, len);
    mprFree(ptr);
    return result;
}


EV *ejsAllocValue(Ejs *ejs, EjsType *type, int extra)
{
    EV  *ev;

    mprAssert(type);
    mprAssert(extra >= 0);

    if ((ev = (EV*) ejsAlloc(ejs, type->instanceSize + extra)) == NULL) {
        return NULL;
    }
    TYPE(ev) = type;
    GET_MEM(ev)->hasType = 1;
    return ev;
}


void *ejsAllocWithManager(Ejs *ejs, size_t size, EjsManager manager)
{
    void    *ptr;

    if ((ptr = ejsAlloc(ejs, size)) == NULL) {
        return NULL;
    }
    MANAGER(ptr) = manager;
    return ptr;
}


void *ejsRealloc(Ejs *ejs, void *ptr, size_t required)
{
    EjsMem      *mp;
    char        *newptr;
    int         usize;

    mprAssert(required > 0);

    if (ptr == 0) {
        return ejsAlloc(ejs, required);
    }
    mp = GET_MEM(ptr);
    CHECK(mp);
    usize = GET_USIZE(mp);
    if (required <= usize) {
        return ptr;
    }
    if ((newptr = ejsAlloc(ejs, required)) == NULL) {
        return 0;
    }
    memcpy(newptr, ptr, usize);
    memset(&newptr[usize], 0, GET_MEM(newptr)->size - mp->size);
    return newptr;
}


void *ejsDup(Ejs *ejs, void *ptr, bool deep)
{
    size_t      size;
    void        *nptr;
    EjsMem      *mp, *nmp;

    mp = GET_MEM(ptr);
    size = mp->size;
    nmp = ejsAlloc(ejs, size);
    nptr = GET_PTR(nmp);
    memcpy(nptr, ptr, GET_USIZE(mp));
    return nptr;
}


size_t ejsGetBlockSize(cvoid *ptr)
{
    EjsMem      *mp;

    mp = GET_MEM(ptr);
    if (ptr == 0 || !VALID_BLK(mp)) {
        return 0;
    }
    return GET_USIZE(mp);
}



/*
    Initialize the free space map and queues.

    The free map is a two dimensional array of free queues. The first dimension is indexed by
    the most significant bit (MSB) set in the requested block size. The second dimension is the next 
    EJS_ALLOC_BUCKET_SHIFT (4) bits below the MSB.

    +-------------------------------+
    |       |MSB|  Bucket   | rest  |
    +-------------------------------+
    | 0 | 0 | 1 | 1 | 1 | 1 | X | X |
    +-------------------------------+
 */

//  MOB -- need fewer buckets?  Could this work with just 3 bits for NUM_BUCKETS?
static void initFree(EjsHeap *heap)
{
    EjsFreeMem  *freeq;
    
    heap->freeEnd = &heap->free[EJS_ALLOC_NUM_GROUPS * EJS_ALLOC_NUM_BUCKETS];
    for (freeq = heap->free; freeq != heap->freeEnd; freeq++) {
#if EJS_MEMORY_STATS
        size_t      bit, size, groupBits, bucketBits;
        int         index, group, bucket;
        /*
            NOTE: skip the buckets with MSB == 0 (round up)
         */
        index = (freeq - heap->free);
        group = index / EJS_ALLOC_NUM_BUCKETS;
        bucket = index % EJS_ALLOC_NUM_BUCKETS;

        bit = (group != 0);
        groupBits = bit << (group + EJS_ALLOC_BUCKET_SHIFT - 1);
        bucketBits = ((size_t) bucket) << (max(0, group - 1));

        size = groupBits | bucketBits;
        freeq->size = size << EJS_ALIGN_SHIFT;
#endif
        INIT_LIST(freeq);
    }
}


static void initGen(EjsHeap *heap)
{
    EjsMem      *gp;

    for (gp = heap->generations; gp < &heap->generations[EJS_MAX_GEN]; gp++) {
        INIT_LIST(gp);
    }
    heap->gen = &heap->generations[EJS_GEN_ETERNAL];
}


//  MOB -- move out of here into core/src/ejsValue.c

static void missingHelper(Ejs *ejs, EV *obj, cchar *helper) 
{
    EjsType     *type;

    type = TYPE(obj);
    ejsThrowInternalError(ejs, "The \"%s\" helper is not defined for this type \"%S\"", helper, type->qname.name);
}


static void initType(EjsHeap *heap)
{
    Ejs         *ejs;
    EjsHelpers  *helpers;

    ejs = heap->ejs;
    heap->type = ejsAlloc(heap->ejs, sizeof(EjsType));

    //MOB heap->type->qname = N("ejs", "Alloc");
    
    helpers = &heap->type->helpers;
    helpers->create  = ejsAllocValue;
    helpers->mark = ejsMark;
    helpers->clone = ejsDup;

    helpers->cast = (EjsCastHelper) missingHelper;
    helpers->defineProperty = (EjsDefinePropertyHelper) missingHelper;
    helpers->deleteProperty = (EjsDeletePropertyHelper) missingHelper;
    helpers->deletePropertyByName = (EjsDeletePropertyByNameHelper) missingHelper;
    helpers->destroy = (EjsDestroyHelper) NULL;
    helpers->getProperty = (EjsGetPropertyHelper) missingHelper;
    helpers->getPropertyByName = (EjsGetPropertyByNameHelper) missingHelper;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) missingHelper;
    helpers->getPropertyName = (EjsGetPropertyNameHelper) missingHelper;
    helpers->invokeOperator = (EjsInvokeOperatorHelper) missingHelper;
    helpers->lookupProperty = (EjsLookupPropertyHelper) missingHelper;
    helpers->setProperty = (EjsSetPropertyHelper) missingHelper;
    helpers->setPropertyByName = (EjsSetPropertyByNameHelper) missingHelper;
    helpers->setPropertyName = (EjsSetPropertyNameHelper) missingHelper;
}


static int getQueueIndex(EjsHeap *heap, size_t size, int roundup)
{   
    size_t      usize, asize;
    int         aligned, bucket, group, index, msb;
    
    mprAssert(EJS_ALLOC_ALIGN(size) == size);

    /*
        Allocate based on user sizes (sans header). This permits block searches to avoid scanning the next 
        highest queue for common block sizes: eg. 1K.
     */
    usize = (size - sizeof(EjsMem));
    asize = usize >> EJS_ALIGN_SHIFT;

    //  Zero based most significant bit
    msb = flsl(asize) - 1;

    group = max(0, msb - EJS_ALLOC_BUCKET_SHIFT + 1);
    mprAssert(group < EJS_ALLOC_NUM_GROUPS);

    bucket = (asize >> max(0, group - 1)) & (EJS_ALLOC_NUM_BUCKETS - 1);
    mprAssert(bucket < EJS_ALLOC_NUM_BUCKETS);

    index = (group * EJS_ALLOC_NUM_BUCKETS) + bucket;
    mprAssert(index < (heap->freeEnd - heap->free));
    
#if EJS_MEMORY_STATS
    mprAssert(heap->free[index].size <= usize && usize < heap->free[index + 1].size);
#endif
    if (roundup) {
        /*
            Good-fit strategy: check if the requested size is the smallest possible size in a queue. If not the smallest,
            must look at the next queue higher up to guarantee a block of sufficient size.
            Blocks of of size <= 512 bytes (0x20 shifted) are mapped directly to queues. ie. There is only one block size
            per queue. Otherwise, get a mask of the bits below the group and bucket bits. If any are set, then not the 
            lowest size in the queue.
         */
        if (asize > 0x20) {
            size_t mask = (((size_t) 1) << (msb - EJS_ALLOC_BUCKET_SHIFT)) - 1;
            aligned = (asize & mask) == 0;
            if (!aligned) {
                index++;
            }
        }
    }
    return index;
}


#if EJS_MEMORY_STATS && UNUSED
static EjsFreeMem *getQueue(EjsHeap *heap, size_t size)
{   
    EjsFreeMem  *freeq;
    int         index;
    
    index = getQueueIndex(heap, size, 0);
    freeq = &heap->free[index];
    return freeq;
}
#endif


static EjsMem *searchFree(EjsHeap *heap, size_t size, int *indexp)
{
    EjsFreeMem  *freeq;
    EjsMem      *mp;
    size_t      groupMap, bucketMap;
    int         bucket, baseGroup, group, index;
    
    *indexp = index = getQueueIndex(heap, size, 1);
    baseGroup = index / EJS_ALLOC_NUM_BUCKETS;
    bucket = index % EJS_ALLOC_NUM_BUCKETS;

    /* Mask groups lower than the base group */
    groupMap = heap->groupMap & ~((((size_t) 1) << baseGroup) - 1);
    while (groupMap) {
        group = ffsl(groupMap) - 1;
        if (groupMap & ((((size_t) 1) << group))) {
            bucketMap = heap->bucketMap[group];
            if (baseGroup == group) {
                bucketMap &= ~((((size_t) 1) << bucket) - 1);
            }
            while (bucketMap) {
                bucket = ffsl(bucketMap) - 1;
                index = (group * EJS_ALLOC_NUM_BUCKETS) + bucket;
                freeq = &heap->free[index];
                if (freeq->next != freeq) {
                    mp = (EjsMem*) freeq->next;
                    deq(mp);
                    INC(reuse);
                    CHECK(mp);
                    return mp;
                }
                bucketMap &= ~(((size_t) 1) << bucket);
                heap->bucketMap[group] &= ~(((size_t) 1) << bucket);
            }
            groupMap &= ~(((size_t) 1) << group);
            heap->groupMap &= ~(((size_t) 1) << group);
        }
    }
    return NULL;
}


static void linkBlock(EjsMem *head, EjsMem *mp)
{
    CHECK(mp);
    mprAssert(mp != head);

    mp->next = head->next;
    mp->prev = head;
    head->next->prev = mp;
    head->next = mp;
}


static void unlinkBlock(EjsMem *mp)
{
    CHECK(mp);

    mp->prev->next = mp->next;
    mp->next->prev = mp->prev;
}


/*
    Add a block to a free q. Must be called locked.
 */
static void enq(EjsHeap *heap, EjsMem *mp) 
{
    int     index, group, bucket;

    mp->free = 1;
    index = getQueueIndex(heap, mp->size, 0);
    group = index / EJS_ALLOC_NUM_BUCKETS;
    bucket = index % EJS_ALLOC_NUM_BUCKETS;
    heap->groupMap |= (((size_t) 1) << group);
    heap->bucketMap[group] |= (((size_t) 1) << bucket);
    linkBlock((EjsMem*) &heap->free[index], mp);
}


/*
    Remove a block from a free q. Must be called locked.
 */
static void deq(EjsMem *mp) 
{
    unlinkBlock(mp);
    mp->free = 0;
#if EJS_MEMORY_DEBUG
    mp->next = mp->prev = NULL;
#endif
}


/*
    Split a block. Required specifies the number of bytes needed in the block. If swap, then put mp back on the free
    queue instead of the second half.
 */
static EjsMem *splitBlock(EjsHeap *heap, EjsMem *mp, size_t required, int qspare)
{
    EjsMem      *spare, *after;
    size_t      size, extra;

    mprAssert(mp);
    mprAssert(required > 0);

    CHECK(mp);
    BREAKPOINT(mp);

    size = mp->size;
    extra = size - required;
    mprAssert(extra >= EJS_ALLOC_MIN_SPLIT);

    spare = (EjsMem*) ((char*) mp + required);
    INIT_BLK(spare, extra);
    spare->last = mp->last;
    spare->prior = mp;
    BREAKPOINT(spare);

    mp->size = required;
    mp->last = 0;
    if ((after = GET_NEXT(spare)) != NULL) {
        after->prior = spare;
    }
    INC(splits);
    if (qspare) {
        enq(heap, spare);
    }
    CHECK(spare);
    CHECK(mp);
    return (qspare) ? NULL : spare;
}


#if BLD_CC_MMU && !BLD_WIN_LIKE
static void virtFree(EjsHeap *heap, EjsMem *mp)
{
    EjsMem      *spare, *after;
    size_t      gap;

    /*
        If block is non-aligned, split the front portion and save
     */
    gap = EJS_PAGE_ALIGN(mp, heap->pageSize) - (size_t) mp;
    if (gap) {
        if (gap < EJS_ALLOC_MIN_SPLIT) {
            /* Gap must be useful -- If too small, preserve one extra page with it */
            gap += heap->pageSize;
        }
        spare = splitBlock(heap, mp, gap, 0);
        mp->last = 1;
        enq(heap, mp);
        mp = spare;
    }

    /*
        If non-aligned tail, then split the tail and save
     */
    gap = mp->size % heap->pageSize;
    if (gap) {
        if (gap < EJS_ALLOC_MIN_SPLIT) {
            gap += heap->pageSize;
        }
        splitBlock(heap, mp, mp->size - gap, 1);
    }
    if (mp->prior) {
        mp->prior->last = 1;
    }
    if ((after = GET_NEXT(mp)) != NULL) {
        after->prior = NULL;
    }
    SUB(bytesFree, mp->size);

    mprVirtFree((void*) mp, mp->size);
    INC(unpins);
}
#endif


/*
    Grow the heap and return a block of the required size (unqueued)
 */
static EjsMem *growHeap(EjsHeap *heap, size_t required)
{
    EjsMem      *mp;
    size_t      size;

    mprAssert(required > 0);

    size = max(required, (size_t) heap->regionSize);
    size = EJS_PAGE_ALIGN(size, heap->pageSize);

    if ((mp = (EjsMem*) mprVirtAlloc(size, MPR_MAP_READ | MPR_MAP_WRITE)) == NULL) {
        return 0;
    }
    INIT_BLK(mp, size);
    mp->last = 1;
    INC(allocs)
    ADD(bytesFree, mp->size);
    CHECK(mp);
    return mp;
}


void ejsPrintAllocReport(Ejs *ejs)
{
#if EJS_MEMORY_STATS
#endif
}


#if EJS_MEMORY_DEBUG
int ejsIsBlkValid(EjsMem *mp)
{
    mprAssert(mp->magic == EJS_ALLOC_MAGIC);
    mprAssert(mp->size > 0);
    return (mp->magic == EJS_ALLOC_MAGIC) && (mp->size > 0);
}


void ejsCheckBlock(EjsMem *mp)
{
    mprAssert(VALID_BLK(mp));
}


static void breakpoint(EjsMem *mp) 
{
    if (mp == stopAlloc || mp->seqno == stopSeqno) {
        mprBreakpoint();
    }
}


//  MOB -- these two could be macros
EjsString *ejsGetDebugName(void *ptr)
{
    return GET_MEM(ptr)->name;
}


void ejsSetDebugName(void *ptr, EjsString *name)
{
    EjsMem      *mp;

    mp = GET_MEM(ptr);
    mp->name = name;
}


void ejsCopyDebugName(void *dest, cvoid *src)
{
    EjsMem      *mp, *smp;

    smp = GET_MEM(src);
    mp = GET_MEM(dest);
    mp->name = smp->name;
}
#endif /* EJS_MEMORY_DEBUG */


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
