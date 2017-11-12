/*
                An implementation of top-down splaying
                    D. Sleator <sleator@cs.cmu.edu>
    	                     March 1992

  "Splay trees", or "self-adjusting search trees" are a simple and
  efficient data structure for storing an ordered set.  The data
  structure consists of a binary tree, without parent pointers, and no
  additional fields.  It allows searching, insertion, deletion,
  deletemin, deletemax, splitting, joining, and many other operations,
  all with amortized logarithmic performance.  Since the trees adapt to
  the sequence of requests, their performance on real access patterns is
  typically even better.  Splay trees are described in a number of texts
  and papers [1,2,3,4,5].

  The code here is adapted from simple top-down splay, at the bottom of
  page 669 of [3].  It can be obtained via anonymous ftp from
  spade.pc.cs.cmu.edu in directory /usr/sleator/public.

  The chief modification here is that the splay operation works even if the
  item being splayed is not in the tree, and even if the tree root of the
  tree is NULL.  So the line:

                              t = splay(i, t);

  causes it to search for item with key i in the tree rooted at t.  If it's
  there, it is splayed to the root.  If it isn't there, then the node put
  at the root is the last one before NULL that would have been reached in a
  normal binary search for i.  (It's a neighbor of i in the tree.)  This
  allows many other operations to be easily implemented, as shown below.

  [1] "Fundamentals of data structures in C", Horowitz, Sahni,
       and Anderson-Freed, Computer Science Press, pp 542-547.
  [2] "Data Structures and Their Algorithms", Lewis and Denenberg,
       Harper Collins, 1991, pp 243-251.
  [3] "Self-adjusting Binary Search Trees" Sleator and Tarjan,
       JACM Volume 32, No 3, July 1985, pp 652-686.
  [4] "Data Structure and Algorithm Analysis", Mark Weiss,
       Benjamin Cummins, 1992, pp 119-130.
  [5] "Data Structures, Algorithms, and Performance", Derick Wood,
       Addison-Wesley, 1993, pp 367-375.

  - modified by A. Brown, March 2000 
  - trees do not store the key explicitly, instead, the key value for a particular
    node is that node's address. (i.e. the tree is sorted by the addresses of the 
    allocated blocks)
  - each node stores that amount of allocated space returned by gc_malloc in the 
    "size" field

*/

#include <stdio.h>
#include <stdlib.h>
#include "useful.h"

/****************************************************************/
/* Simple top down splay, not requiring i to be in the tree t.  */
/* What it does is described above.                             */
/****************************************************************/

Tree * splay (ptr_t i, Tree *t) {
    Tree N, *l, *r, *y;
    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
	if (i < (ptr_t)t) {
	    if (t->left == NULL) break;
	    if (i < (ptr_t)t->left) {
		y = t->left;                           /* rotate right */
		t->left = y->right;
		y->right = t;
		t = y;
		if (t->left == NULL) break;
	    }
	    r->left = t;                               /* link right */
	    r = t;
	    t = t->left;
	} else if (i > (ptr_t)t) {
	    if (t->right == NULL) break;
	    if (i > (ptr_t)t->right) {
		y = t->right;                          /* rotate left */
		t->right = y->left;
		y->left = t;
		t = y;
		if (t->right == NULL) break;
	    }
	    l->right = t;                              /* link left */
	    l = t;
	    t = t->right;
	} else {
	    break;
	}
    }
    l->right = t->left;                                /* assemble */
    r->left = t->right;
    t->left = N.right;
    t->right = N.left;
    return t;
}

/***************************************************************/
/* Insert "new" allocated block into the tree t, unless it's   */
/* already there. Return a pointer to the resulting tree.      */
/***************************************************************/

Tree * insert(Tree * t, Tree *new) {

    if (t == NULL) {
	new->left = new->right = NULL;
	return new;
    }
    t = splay((ptr_t)new, t);
    if (new < t) {
	new->left = t->left;
	new->right = t;
	t->left = NULL;
	return new;
    } else if (new >  t) {
	new->right = t->right;
	new->left = t;
	t->right = NULL;
	return new;
    } else { /* We get here if it's already in the tree */
        fprintf(stderr,"Error, allocating block with same address as previously allocated block, %p\n",new);
	fprintf(stderr,"Please run your program under gdb\n");
	return 0;
    }
}

/*******************************************************/
/* Deletes i from the tree if it's there.              */
/* Returns a pointer to the resulting tree.            */
/*******************************************************/
Tree * delete(ptr_t i, Tree * t) {
    Tree * x;

    if (t == NULL) return NULL;

    t = splay(i, t);
    if (i == (ptr_t)t) {         /* found it */
	if (t->left == NULL) {
	    x = t->right;
	} else {
	    x = splay(i, t->left);
	    x->right = t->right;
	}
	t->right = NULL;
	t->left = NULL;
	return x;
    }
    /* It wasn't there */
    fprintf(stderr,"Error, attempt to delete block that was not in the allocated tree\n");
    fprintf(stderr,"\t Block Address:  %p\n",(void *)i);
    fprintf(stderr,"Please run your program under gdb\n");
    exit(1);

	/* Syntax Purpose */
	return NULL;
}

/******************************************************************************/
/* Return the node which logically contains the given address, if one exists; */
/* eg, a node with address x and size s contains all addresses in             */
/* [x+sizeof(Tree),x+sizeof(Tree)+s).     				      */
/* To do this, we first find the node with the largest address smaller than   */
/* the one given (i.e., i),then check if i is within the data portion of this */
/* node. If i is not contained in any node in the allocated tree, NULL is     */
/* returned.                                                                  */
/* splay is used to find a neighbour of i in the tree, and the modified tree  */
/* is returned in the second parameter.                                       */
/******************************************************************************/
Tree * contains(ptr_t i, Tree **the_tree)
{
  Tree *tmp;
  int size;

  *the_tree = splay(i, *the_tree);
  tmp = *the_tree;

  if(i < (ptr_t)(tmp) ) {
    tmp = tmp->left;
    while(tmp && tmp->right != NULL) {
      tmp = tmp->right;
    }
  }

  if (tmp) {
    /* mask out mark bit of size, check if i is in tmp's data area */
    size = tmp->size & 0xfffffffe;
    if((i < (ptr_t)tmp + sizeof(Tree)) || (i >= ((ptr_t)tmp + sizeof(Tree) + size)) )
      tmp = NULL;
  }

  return tmp;

}

/*************************************************************/
/* dead simple tree printing routine; prints left and right  */
/* children along with the current node at each step         */
/*************************************************************/
void print_tree(Tree *root, int level)
{
  int i;
  for (i=0; i<level; i++) {
    fprintf(stderr,"  ");
  }
  fprintf(stderr,"cur = %p, l=%p, r=%p\n",root,root->left, root->right);
  if(root->left != NULL)
    print_tree(root->left, level+1);
  if(root->right != NULL)
    print_tree(root->right, level+1);
}

