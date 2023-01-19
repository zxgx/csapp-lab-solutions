/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

#define _DEBUG

// ########################################
/* Basic constants and macros            */
// ########################################
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE       4       /* Word and header/footer size (bytes) */ //line:vm:mm:beginconst
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  //line:vm:mm:endconst 

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            //line:vm:mm:get
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    //line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   //line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1)                    //line:vm:mm:getalloc
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      //line:vm:mm:hdrp
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) //line:vm:mm:nextblkp
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) //line:vm:mm:prevblkp


#define GETP(p)             (*(char **)(p))           // get an address value from p
#define PUTP(p, val)        (*(char **)(p) = (val))   // put an address value into p 
#define PREV_PTR(bp)        ((char *)(bp))            // get the address of prev free block's address from this free block
#define NEXT_PTR(bp)        ((char *)(bp)+SIZE_T_SIZE)// get the address of next free block's address from this free block

// ########################################
/* Global variables                      */
// ########################################
static char *heap_listp = 0;  /* Pointer to first block */  

// ########################################
/* Local Helper functions                */
// ########################################
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_PREV_ALLOC(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    char * ptr;

    if(prev_alloc && next_alloc){               /* Case 1 */
        char * prev_ptr = heap_listp;
        ptr = GETP(NEXT_PTR(prev_ptr));
        while(ptr!=0 && ptr < bp){
            prev_ptr = ptr;
            ptr = GETP(NEXT_PTR(ptr));
        }

        PUTP(PREV_PTR(bp), prev_ptr);           /* bp.prev = prev */
        PUTP(NEXT_PTR(bp), ptr);                /* bp.next = next */
        PUTP(NEXT_PTR(prev_ptr), bp);           /* prev.next = bp */
        if(ptr) PUTP(PREV_PTR(ptr), bp);        /* next.prev = bp */

#ifdef _DEBUG
        if(
            (GETP(PREV_PTR(bp))!=heap_listp && GETP(PREV_PTR(bp)) == PREV_BLKP(bp))
        ||  (GETP(NEXT_PTR(bp))!=0 && GETP(NEXT_PTR(bp)) == NEXT_BLKP(bp))
        ) {
            print_free_list("case 1");
            print_block_range("case 1", bp);
            print_free_list_ptr("case 1", bp);
            printf("case 1 error\n");
            exit(1);
        }
#endif

    }

    else if (prev_alloc && !next_alloc) {       /* Case 2 */
        // update free list pointer
        ptr = GETP(PREV_PTR(NEXT_BLKP(bp)));    /* origin prev */
        PUTP(NEXT_PTR(ptr), bp);                /* prev.next = bp */
        PUTP(PREV_PTR(bp), ptr);                /* bp.prev = prev */

#ifdef _DEBUG
        if(
            GETP(NEXT_PTR(GETP(PREV_PTR(NEXT_BLKP(bp))))) != bp
        ||  GETP(PREV_PTR(bp)) != GETP(PREV_PTR(NEXT_BLKP(bp)))
        ) {
            print_free_list("case 2 - 1");
            print_block_range("case 2 - 1", bp);
            print_free_list_ptr("case 2 - 1", bp);
            printf("case 2 - 1 error\n");
            exit(1);
        }
#endif
        ptr = GETP(NEXT_PTR(NEXT_BLKP(bp)));    /* origin next */
        PUTP(NEXT_PTR(bp), ptr);                /* bp.next = next.next */
        if(ptr) PUTP(PREV_PTR(ptr), bp);        /* next.prev = bp */

#ifdef _DEBUG
        if(
            GETP(NEXT_PTR(NEXT_BLKP(bp))) != GETP(NEXT_PTR(bp))
        ||  (GETP(NEXT_PTR(NEXT_BLKP(bp)))!=0 && GETP(PREV_PTR(GETP(NEXT_PTR(NEXT_BLKP(bp))))) != bp)
        ) {
            print_free_list("case 2 - 2");
            print_block_range("case 2 - 2", bp);
            print_free_list_ptr("case 2 - 2", bp);
            printf("case 2 - 2 error\n");
            exit(1);
        }
#endif

        // update header & footer
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 2));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) {       /* Case 3 */
        // update free list
        // do nothing

        // update header & footer
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 2));
    }

    else if (!prev_alloc && !next_alloc){       /* Case 4 */
        // update free list
        ptr = GETP(NEXT_PTR(NEXT_BLKP(bp)));        /* origin next.next*/
        PUTP(NEXT_PTR(PREV_BLKP(bp)), ptr);         /* prev.next = next.next */
        if(ptr) PUTP(PREV_PTR(ptr), PREV_BLKP(bp)); /* next.next.prev = prev */

#ifdef _DEBUG
        if(
            (GETP(NEXT_PTR(PREV_BLKP(bp))) != GETP(NEXT_PTR(NEXT_BLKP(bp))))
        ||  (GETP(NEXT_PTR(NEXT_BLKP(bp)))!=0 && GETP(PREV_PTR(GETP(NEXT_PTR(NEXT_BLKP(bp))))) != PREV_BLKP(bp))
        ) {
            print_free_list("case 4");
            print_block_range("case 4", bp);
            print_free_list_ptr("case 4", bp);
            printf("case 4 error\n");
            exit(1);
        }
#endif

        // update header & footer
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 2));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    // clear the prev_alloc tag of the next block
    PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) & (~0x2));

#ifdef _DEBUG
        if(
            (GETP(PREV_PTR(bp))>=bp)
        ||  (GETP(NEXT_PTR(bp))!=0 && GETP(NEXT_PTR(bp)) <= bp)        
        ) {
            print_free_list("general pos");
            print_block_range("general pos", bp);
            print_free_list_ptr("general pos", bp);
            printf("general pos error\n");
            exit(1);
        }
#endif
    return bp;
}

static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        //line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, GET_PREV_ALLOC(HDRP(bp))));         /* Free block header */   //line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   //line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ //line:vm:mm:newepihdr

    /* Coalesce if the previous block was free */
    return coalesce(bp);  
}

static void *find_fit(size_t asize)
{
    void *bp;

    for (bp = GETP(NEXT_PTR(heap_listp)); bp!=0; bp = GETP(NEXT_PTR(bp))) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            return bp;
        }
    }
    return NULL;
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));   
    char *prev_ptr = GETP(PREV_PTR(bp)), *next_ptr = GETP(NEXT_PTR(bp));
    
    if ((csize - asize) >= (DSIZE + 2*SIZE_T_SIZE)) {  // header + footer + 2' free block pointer
        PUT(HDRP(bp), PACK(asize, 1|GET_PREV_ALLOC(HDRP(bp))));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 2));
        PUT(FTRP(bp), PACK(csize-asize, 0));

        // update free list
        PUTP(NEXT_PTR(prev_ptr), bp);               /* prev.next = bp */
        PUTP(PREV_PTR(bp), prev_ptr);               /* bp.prev = prev */
        PUTP(NEXT_PTR(bp), next_ptr);               /* bp.next = next */
        if(next_ptr) PUTP(PREV_PTR(next_ptr), bp);  /* next.prev = bp */
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1|GET_PREV_ALLOC(HDRP(bp))));
        PUT(HDRP(NEXT_BLKP(bp)), GET(HDRP(NEXT_BLKP(bp))) | 2);

        PUTP(NEXT_PTR(prev_ptr), next_ptr);                 /* prev.next = next */
        if(next_ptr) PUTP(PREV_PTR(next_ptr), prev_ptr);    /* next.prev = prev */
    }
}

// ########################################
/* Helper functions & variables for debugging purpose */
// ########################################
void print_block_range(char * msg, void *bp){
    printf("===== Block Range: %s =====\n", msg);
    printf("\tblock addr: %lx\n\theader: %d\n\tfooter addr: %lx\n", bp, GET(HDRP(bp)), FTRP(bp));
}

void print_free_list_ptr(char * msg, void *bp){
    printf("===== Free List Pointer: %s =====\n", msg);
    char *content = GETP(bp);
    if(!content)
        printf("\tptr: %lx, content: %lx\n", bp, content);
    else
        printf("\tptr: %lx, content: %lx, prev ptr(%lx): %lx, next ptr(%lx): %lx\n", msg, bp, content, PREV_PTR(bp), GETP(PREV_PTR(bp)), NEXT_PTR(bp), GETP(NEXT_PTR(bp)));
}

void print_free_list(char * msg){
    printf("===== Free List: %s =====\n", msg);

    void *bp;

    for (bp = heap_listp; bp!=0; bp = GETP(NEXT_PTR(bp))) {
        printf("\tptr: %lx, size: %ld, prev(%lx): %lx, next(%lx): %lx\n", bp, GET(HDRP(bp)), PREV_PTR(bp), GETP(PREV_PTR(bp)), NEXT_PTR(bp), GETP(NEXT_PTR(bp)));
    }
    printf("===== END Free List =====\n");
}

static size_t malloc_cnt=0, free_cnt=0, realloc_cnt=0;

// ########################################
/* Heap memory management interface      */
// ########################################
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE + 2*SIZE_T_SIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                                                     /* Alignment padding */
    PUT(heap_listp + WSIZE, PACK(DSIZE+2*SIZE_T_SIZE, 3));                  /* Prologue header */ 
    PUTP(heap_listp + (2*WSIZE), 0);                                        /* Dummy node.prev */
    PUTP(heap_listp + (2*WSIZE+SIZE_T_SIZE), 0);                            /* Dummy node.next */
    PUT(heap_listp + (2*WSIZE+2*SIZE_T_SIZE), PACK(DSIZE+2*SIZE_T_SIZE, 1));/* Prologue footer */ 
    PUT(heap_listp + (3*WSIZE+2*SIZE_T_SIZE), PACK(0, 3));                  /* Epilogue header */
    heap_listp += (2*WSIZE);
    
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) 
        return -1;

#ifdef _DEBUG
    print_block_range("heap list", heap_listp);
    print_free_list("init");
#endif
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
#ifdef _DEBUG
    printf("malloc: %ld\n", ++malloc_cnt);
#endif
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;

    if (heap_listp == 0) mm_init();
    /* Ignore spurious requests */
    if (size == 0) return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    asize = DSIZE * ((size + (WSIZE) + (DSIZE-1)) / DSIZE);
#ifdef _DEBUG
    printf("adjust bytes: %ld\n", asize);
#endif
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {  //line:vm:mm:findfitcall
        place(bp, asize);                  //line:vm:mm:findfitplace

#ifdef _DEBUG
    print_block_range("after allocate block in current list", bp);
    print_free_list("after allocate block in current list");
#endif
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 //line:vm:mm:growheap1
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)  
        return NULL;                                  //line:vm:mm:growheap2
    place(bp, asize);                                 //line:vm:mm:growheap3

#ifdef _DEBUG
    print_block_range("after allocate block in new block", bp);
    print_free_list("after allocate block in new block");
#endif
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{   
#ifdef _DEBUG
    printf("free: %ld\n", ++free_cnt);
#endif
    if (ptr == 0) return;

    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, GET_PREV_ALLOC(HDRP(ptr))));
    PUT(FTRP(ptr), PACK(size, 0));
    ptr = coalesce(ptr);

#ifdef _DEBUG
    print_block_range("after free", ptr);
    print_free_list("after free");
#endif
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{   
#ifdef _DEBUG
    printf("realloc: %ld\n", ++realloc_cnt);
#endif
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}
