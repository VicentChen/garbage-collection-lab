#ifndef VM_H__
#define VM_H__

/* vm function return values */
enum {
    VM_OK,
    VM_OUT_OF_MEMORY,
    VM_OUT_OF_PHYSICAL_MEMORY,
    VM_NOT_START
};

/* default vm info */
#define K 1024
#define M (1024 * K)
#define DEFAULT_VM_NAME "GC Virtual Mahcine"
#define DEFAULT_VM_MEMORY_SIZE 16 * M;

typedef struct {
    char* name;
    long memory_size; // bytes
    char* memory_begin; // memory begin address
    char* memory_end; // memory end address
} vm_status;

/* default vm params */
#define DEFAULT_VM_PROMPT 1

typedef struct {
    int prompt;
} vm_option;

void vm_shell();

#endif
