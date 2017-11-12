
/*
 *  CS213 - Lab assignment 3
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Win32: Use PE Const Instead in program environment */
//#include <elf.h>

#include "memlib.h"
#include "malloc.h"
#include "useful.h"

team_t team = {
    /* Team name to be displayed on webpage */
    "",
    /* First member full name */
    "",
    /* First member email address */
    "",
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
int gc_init (void)
{
    list_t *list;

    mem_sbrk(mem_pagesize());

    /* initialize freelist ptr */
    list = (list_t *)(dseg_lo+(sizeof(long)<<1));
    *(list_t **)dseg_lo = list;
    list->next = list->prev = list;
    /* got 1 page, used first 2 words for free list ptr, alloc'd ptr, */
    /* of the remaining bytes, need space for a Tree structure */
    list->size = mem_pagesize() - (sizeof(ptr_t)<<1) - sizeof(Tree);

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
  /* You can define the stack data type in any way that you like.
   * You are allowed to use the standard libc malloc/free 
   * routines for the stack management, if you wish. 
   * DO NOT use gc_malloc!
   */
  int dummy; // SYNTAX PURPOSE, replace it by your own ones.
} stack_t;


/* return a pointer to the address where the global data segment starts,
 * and the length of the segment in bytes (not words!).
 */
/* Win32: Used PE Constant Value instead in program environment */
/*void get_data_area(void **start, int *length)
{

    Elf32_Ehdr *test = (Elf32_Ehdr *)0x8048000;
    Elf32_Phdr *pgm = (Elf32_Phdr *)((void *)test + test->e_phoff);
    Elf32_Half nhdr = test->e_phnum;

    for( ; pgm < (pgm + nhdr); pgm++) {	
	if((pgm->p_type == PT_LOAD) && !(pgm->p_flags & PF_X)) { 
	    /* Loadable program segment AND not executable, must be data * /
	    *start = (void *)(pgm->p_vaddr);
	    *length = (int)(pgm->p_memsz);
	    break;
 	}
    }	
}*/

/* Traverse the tree of allocated blocks, collecting
 * pointers to unallocated blocks in the stack s
 */
void collect_unmarked(Tree *root, stack_t *s)
{
    if(root) {
		if(!(root->size & 0x1)) { /* unreachable */
		/* Push the root onto your stack.
		* Define "push" in any way that is appropriate for
		* your stack data structure 
		*/
		}
		root->size &= ~0x1; /* clear for next time */
		if(root->left) collect_unmarked(root->left, s);
		if(root->right) collect_unmarked(root->right, s);
    }
}

extern void verify_complete(void);
extern void verify_garbage(void *addr);

void garbage_collect(int *regs, int pgm_stack)
{
  Tree *alloc_tree = NULL;
  stack_t *stack = NULL;
  int registers[3];
  int top;

  GET_CALLEE_REGS(registers);
  PGM_STACK_TOP(top);

  /* 0. Make sure there is already allocated memory to 
   *    garbage collect.
   */


  /* 1. collect root pointers by scanning regs, stack, and global data */


  /* 2. mark nodes that are reachable from the roots and add any pointers 
   *    found in reachable heap memory 
   */


  /* 3. collect pointers to all the unmarked (i.e. unreachable) blocks */

  collect_unmarked(alloc_tree, stack);

  /* 4. delete each unmarked block from the allocated tree and 
   *    then place the block on the free list.
   *    Call verify_garbage with the address previously returned by malloc
   *    for each block.
   */

  verify_complete();
}


void *gc_malloc (size_t size)
{
    list_t *list, *newlist;
    char *addr, *test;
    long foo, mask;
    Tree *alloc_tree;
    int registers[3];
    int top;

    /* start by getting the callee-save registers and the top of the program
     * stack at this point. Doing this here means that we don't have to search
     * the stack frame for gc_malloc or anything that it calls when we collect
     * root pointers.
     */
    GET_CALLEE_REGS(registers);
    PGM_STACK_TOP(top);

    /* round up size to a power of 8 */
    size = (size ? ((size>>3) + !!(size & 7)) << 3 : 8);

    /* search un-coalesced free list */
    addr = dseg_lo;
    list = *(list_t **)addr;
    if (list != NULL) {
      do {
	if ((unsigned)list->size >= size) {
	  break;
	}
	list = list->next;
	if (list == *(list_t **)addr) {
	  list = NULL;
	}
      } while (list);
    }
      
    /* if we couldn't find a fit then try sbrk */
    if(list==NULL) {    
      mask = mem_pagesize()-1;
      size += sizeof(Tree);
      /* number of pages needed, ceil( size/pagesize ) */
      foo = (size & ~mask) + ((!!(size & mask))*mem_pagesize());
      size -= sizeof(Tree);
      list = (list_t *)(dseg_hi+1);
      test = mem_sbrk(foo);
      if (!test) 
	return NULL;
      list->size = foo - sizeof(Tree);
      gc_add_free(dseg_lo, list);
    }


    /* found a fit */
    if ((unsigned)list->size < size + sizeof(list_t) + sizeof(long) ) {
	/* give them the whole chunk (we need sizeof(list_t) to hold accounting
	 * information, want at least one usable word if we're going to 
	 * keep track of this block!) */
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
    } else {
	newlist->prev = list->prev;
	newlist->next = list->next;
	newlist->prev->next = newlist->next->prev = newlist;
    }
    if (*(list_t **)addr == list) {
	*(list_t **)addr = newlist;
    }

    /* put this block in the allocated tree */
    addr = dseg_lo + sizeof(long);
    alloc_tree = *(Tree **)addr;
    *(Tree **)addr = insert(alloc_tree, (Tree *)list);

    return ((char *)list + sizeof(Tree));
}

