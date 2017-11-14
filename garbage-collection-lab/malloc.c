
/*
*  CS213 - Lab assignment 3
*  Linux related code deleted by Vicent Chen 17/11/11
*  code added by Vicent Chen 17/11/11
*  collect version: origin
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )

#include "memlib.h"
#include "malloc.h"
#include "useful.h"

team_t team = {
    /* Team name to be displayed on webpage */
    "Vic",
    /* First member full name */
    "Vicent Chen",
    /* First member email address */
    "1336754721@qq.com",
    /* Second member full name (leave blank if none) */
    "",
    /* Second member email address (blank if none) */
    ""
};

/*
* The free list has a ptr to its head at dseg_lo, and the tree of
* allocated blocks has a ptr to its root stored at dseg_lo + sizeof(long).
* gc_add_free just dumps blocks onto the free list
*
* gc_malloc currently searches the free list; if that fails
* to get a large enough block, then it resorts to mem_sbrk.
* You will need to decide when to call garbage_collect.
*
* Used blocks have a header that consists of a Tree structure, which
* contains the size of the allocated area and ptrs to the headers of
* other allocated blocks.
* Free blocks have the linked list which keeps track of them stored
* in the header space.
*/

typedef struct list_struct {
    int size;
    struct list_struct *next, *prev;
} list_t;

/* Initialize the heap by getting 1 page of memory;
* The free list pointer and allocated tree pointers
* occupy the first two words of this page. The rest of the page
* is our first free block. Set the free list pointer to
* point to it, and set the allocated tree pointer to NULL
*/
int gc_init(void) {
    list_t *list;

    mem_sbrk(mem_pagesize());

    /* initialize freelist ptr */
    list = (list_t *)(dseg_lo + (sizeof(long) << 1));
    *(list_t **)dseg_lo = list;
    list->next = list->prev = list;
    /* got 1 page, used first 2 words for free list ptr, alloc'd ptr, */
    /* of the remaining bytes, need space for a Tree structure */
    list->size = mem_pagesize() - (sizeof(ptr_t) << 1) - sizeof(Tree);

    /* initialize the allocated tree pointer to NULL since nothing alloc'd yet */
    *(Tree **)(dseg_lo + sizeof(long)) = NULL;
    return 0;
}

/* Remove a block from a list; "addr" is the address where
* the pointer to the head of the list is stored; "blk"
* is a pointer to the block to remove from the list.
*/
void gc_remove_free(char *addr, list_t *blk) {
    if (blk == blk->next) {
        *(list_t **)addr = NULL;
        return;
    }

    if (*(list_t **)addr == blk) {
        *(list_t **)addr = blk->next;
        if (!blk->next->size)
            assert(0);
    }

    blk->next->prev = blk->prev;
    blk->prev->next = blk->next;
}

/* Add a block to a list; "addr" is the address where
* the pointer to the head of the list is stored,
* "blk" is a pointer to the block to add to the list.
*/
void gc_add_free(char *addr, list_t *blk) {
    list_t *freelist;

    freelist = *(list_t **)addr;

    if (freelist == NULL) {
        blk->next = blk->prev = blk;
        *(list_t **)addr = blk;
        return;
    }
    blk->next = freelist;
    blk->prev = freelist->prev;
    freelist->prev->next = blk;
    freelist->prev = blk;

    /* make this the first thing on the list */
    *(list_t **)addr = blk;
}

/************************************************************/
/* Functions and types that you will find useful for        */
/* garbage collection. Define any additional functions here */
/************************************************************/

typedef struct stack_s {
    Tree *val;
    struct stack_s *next;
} stack_t;

stack_t *stack = NULL;

void push(Tree* val) {
    stack_t *curr = (stack_t*)malloc(sizeof(stack_t));
    curr->val = val;
    curr->next = stack;
    stack = curr;
}

Tree* pop() {
    Tree *ret = stack->val;
    stack_t *curr = stack;
    stack = stack->next;
    free(curr);
    return ret;
}

/**
* added By Vicent Chen
* ======================
* author: Nawaz & phonetagger
* start: start address of data segment
* length: amount of bytes in data segment
* link: https://stackoverflow.com/questions/4308996/finding-the-address-range-of-the-data-segment
*
*/
void get_data_area(char **start, int *length) {
    HANDLE hModule = GetModuleHandle(NULL);
    // get the location of the module's IMAGE_NT_HEADERS structure
    IMAGE_NT_HEADERS *pNtHdr = ImageNtHeader(hModule);

    // section table immediately follows the IMAGE_NT_HEADERS
    IMAGE_SECTION_HEADER *pSectionHdr = (IMAGE_SECTION_HEADER *)(pNtHdr + 1);

    const char* imageBase = (const char*)hModule;
    char scnName[sizeof(pSectionHdr->Name) + 1];
    scnName[sizeof(scnName) - 1] = '\0'; // enforce nul-termination for scn names that are the whole length of pSectionHdr->Name[]

    for (int scn = 0; scn < pNtHdr->FileHeader.NumberOfSections; ++scn) {
        // Note: pSectionHdr->Name[] is 8 bytes long. If the scn name is 8 bytes long, ->Name[] will
        // not be nul-terminated. For this reason, copy it to a local buffer that's nul-terminated
        // to be sure we only print the real scn name, and no extra garbage beyond it.
        strncpy(scnName, (const char*)pSectionHdr->Name, sizeof(pSectionHdr->Name));
        if (!strcmp(".data", scnName)) {
            *start = (void*)(imageBase + pSectionHdr->VirtualAddress);
            *length = (int)(pSectionHdr->Misc.VirtualSize);
        }
        ++pSectionHdr;
    }
}

/* Traverse the tree of allocated blocks, collecting
* pointers to unallocated blocks in the stack s
*/
void push_unmarked(Tree *root) {
    if (root == NULL) return;
    if (!(root->size & 0x1)) push(root);
    root->size &= ~0x1; /* clear for next time */
    if (root->left) push_unmarked(root->left);
    if (root->right) push_unmarked(root->right);
}

extern void verify_complete(void);
extern void verify_garbage(void *addr);

# define LIST_ROOT_ADDRESS ((list_t**)dseg_lo)
# define LIST_ROOT (*LIST_ROOT_ADDRESS)
# define TREE_ROOT_ADDRESS ((Tree**)(dseg_lo + sizeof(long)))
# define TREE_ROOT (*TREE_ROOT_ADDRESS)
# define MARK(block_ptr)\
  do {\
    block_ptr->size |= 0x1;\
  } while(0)

void garbage_collect(int *regs, int pgm_stack) {
    int data_seg_len;
    int registers[3], top;
    char* data_seg_ptr;
    Tree *alloc_tree, *target;

    /* initialize */
    GET_CALLEE_REGS(registers);
    PGM_STACK_TOP(top);
    alloc_tree = TREE_ROOT;

    if (alloc_tree == NULL) return;

    /* MARK */
    /* collect from register */
    for (unsigned i = 0; i < 3; i++) {
        if (target = contains(regs[i], TREE_ROOT_ADDRESS))
            MARK(target);
    }
    /* collect from stack */
    top = (*(int*)(*(int*)(*(int*)(top)))); /* v: ebp of main */
    for (unsigned i = pgm_stack; i < top; i += 4) {
        if (target = contains(*(int*)i, TREE_ROOT_ADDRESS))
            MARK(target);
    }
    /* collect from data segment */
    get_data_area(&data_seg_ptr, &data_seg_len);
    for (unsigned i = 0; i < data_seg_len; i += 4) {
        /* skip data_lo, data_hi */
        if ((unsigned)data_seg_ptr + i == (unsigned)&dseg_lo || (unsigned)data_seg_ptr + i == (unsigned)&dseg_hi)
            continue;
        if (target = contains(*(int*)(data_seg_ptr + i), TREE_ROOT_ADDRESS))
            MARK(target);
    }

    /* SWEEP */
    alloc_tree = TREE_ROOT;
    push_unmarked(alloc_tree);
    /* add unmarked to list */
    while (stack != NULL) {
        target = pop();
        verify_garbage((char*)target + sizeof(Tree));
        TREE_ROOT = delete((long)target, TREE_ROOT);
        gc_add_free((char*)LIST_ROOT_ADDRESS, (list_t*)target);
    }

    verify_complete(); /* never modify */
}

void *gc_malloc(size_t size) {
    int registers[3], top;
    long foo, mask;
    char *addr, *test;
    list_t *list, *newlist;
    Tree *alloc_tree;


    /* start by getting the callee-save registers and the top of the program
    * stack at this point. Doing this here means that we don't have to search
    * the stack frame for gc_malloc or anything that it calls when we collect
    * root pointers.
    */
    GET_CALLEE_REGS(registers);
    PGM_STACK_TOP(top);

    /* round up size to a power of 8 */
    size = size ? (size >> 3) + !!(size & 7) << 3 : 8;

    /* search un-coalesced free list */
    addr = dseg_lo;
    list = *(list_t **)addr;
    while (list != NULL) {
        if ((unsigned)list->size >= size) break;
        list = list->next;
        if (list == *(list_t **)addr) list = NULL;
    }
    /* failed to find a block then GC */
    if (list == NULL) {
        garbage_collect(registers, top);
        /* v: search the free list again*/
        list = *(list_t **)addr;
        while (list != NULL) {
            if ((unsigned)list->size >= size) break;
            list = list->next;
            if (list == *(list_t **)addr) list = NULL;
        }
    }

    /* if we couldn't find a fit then try sbrk */
    /* v: even after GC */
    if (list == NULL) {
        mask = mem_pagesize() - 1;
        size += sizeof(Tree);
        /* number of pages needed, ceil( size/pagesize ) */
        foo = (size & ~mask) + ((!!(size & mask))*mem_pagesize());
        size -= sizeof(Tree);
        list = (list_t *)(dseg_hi + 1);
        test = mem_sbrk(foo);
        if (!test) {
            printf("fragment too much\n");
            return NULL;
        }
        list->size = foo - sizeof(Tree);
        gc_add_free(dseg_lo, list);
    }


    /**
    * v: not split the block
    * the block to split should have at least
    * a space for list_t, and a space for long(the space to use)
    */
    if ((unsigned)list->size < size + sizeof(list_t) + sizeof(long)) {
        gc_remove_free(addr, list);
        /* put this block in the allocated tree */
        addr = dseg_lo + sizeof(long);
        alloc_tree = *(Tree **)addr;
        *(Tree **)addr = insert(alloc_tree, (Tree *)list);
        return (((char *)list) + sizeof(Tree));
    }

    /* give them the beginning of the block */
    newlist = (list_t *)(((char *)list) + size + sizeof(Tree));
    newlist->size = list->size - size - sizeof(Tree);
    list->size = size;

    if (list->next == list) {
        newlist->next = newlist->prev = newlist;
    }
    else {
        newlist->prev = list->prev;
        newlist->next = list->next;
        newlist->prev->next = newlist->next->prev = newlist;
    }

    if (*(list_t **)addr == list)
        *(list_t **)addr = newlist;

    /* put this block in the allocated tree */
    addr = dseg_lo + sizeof(long);
    alloc_tree = *(Tree **)addr;
    *(Tree **)addr = insert(alloc_tree, (Tree *)list);
    return ((char *)list + sizeof(Tree));
}

