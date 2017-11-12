/* define type ptr_t as "long int" for various pointer arithmetic operations */

#define ptr_t long int

/*********************************************************/
/* balanced tree management (for allocated blocks)       */
/*********************************************************/

typedef struct tree_node Tree;
struct tree_node {
  int size;
  Tree * left, * right;
};

extern Tree * splay (ptr_t i, Tree * t);
extern Tree * contains(ptr_t i, Tree **t);
extern Tree * insert(Tree *t, Tree *new);
extern Tree * delete(ptr_t i, Tree * t);


/*********************************************************/
/* macros to get the root sets                           */
/*********************************************************/

/* r must be an array; the first three elements of r will
 * be assigned the current value of the callee-save regs
 */
#define GET_CALLEE_REGS(r) {\
	_asm { lea eax,(r) }\
	_asm { mov [eax],ebx }\
	_asm { mov [eax+4],esi }\
	_asm { mov [eax+8],edi }\
}


/* put contents of %ebp (the stack base pointer) into "t";
 * this will disregard the contents of the current stack frame when 
 * the macro is used
 */
#define PGM_STACK_TOP(t) _asm {mov (t),ebp } 
