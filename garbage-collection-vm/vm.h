#ifndef VM_H__
#define VM_H__

/* vm function return values */
enum {
    VM_OK,
    VM_OUT_OF_MEMORY,
    VM_OUT_OF_PHYSICAL_MEMORY,
    VM_NOT_START
};

#define EXCEPTION_THROW(err_code)\
    do {\
        switch(err_code) {\
            case 1: printf("\tOut of Memory\n"); break;\
            case 2: printf("\tOut of Physical Memory\n"); break;\
            case 3: printf("\tVirtual Machine Not Start Yet\n"); break;\
            default: printf("\tUnknown Error\n"); break;\
        }\
    } while (0)
#define EXCEPTION_THROW_INT(err_code)\
    do {\
        EXCEPTION_THROW(err_code);\
        return err_code;\
    }while(0)

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

/* vm control */
extern int vm_init(vm_status*);
extern int vm_boot();
extern int vm_reboot();
extern int vm_shutdown();
extern void vm_shell();

/* vm functions */
extern int vm_allocate(long);

/* vm infos */
extern void show_vm_info();
extern void vm_shell_help();

#endif
