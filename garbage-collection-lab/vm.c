#define _WINDOWS
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

vm_option vm_params;
vm_status vm;

#define vm_init_default()\
    do {\
        vm_params.prompt = DEFAULT_VM_PROMPT;\
        vm.name = DEFAULT_VM_NAME;\
        vm.memory_size = DEFAULT_VM_MEMORY_SIZE;\
        vm.memory_begin = NULL;\
        vm.memory_end = NULL;\
    }while(0)

#define vm_prompt(s)\
    do {\
        if (vm_params.prompt) printf(s);\
        putchar('\n');\
    }while(0)

int vm_init(vm_status *vm_info) {
    if (vm_info != NULL) memcpy(&vm, vm_info, sizeof(vm_status));
    vm_init_default();
    return VM_OK;
}

int vm_boot() {
    vm.memory_begin = (char*)malloc(vm.memory_size);
    if (vm.memory_begin == NULL) {
        vm_prompt("out of physical memory");
        return VM_OUT_OF_PHYSICAL_MEMORY;
    }
    vm.memory_end = vm.memory_begin + vm.memory_size;
    show_vm_info();
    vm_prompt("vitural machine boot");
    return VM_OK;
}

int vm_reboot() {
    if (vm.memory_begin == NULL) {
        vm_prompt("virtual machine not boot yet");
        return VM_NOT_START;
    }
    system("cls");
    memset(vm.memory_begin, 0, vm.memory_size);
    show_vm_info();
    vm_prompt("virtual machine reboot");
    return VM_OK;
}

int vm_shutdown() {
    if (vm.memory_begin == NULL) {
        vm_prompt("virtual machine not boot yet");
        return VM_NOT_START;
    }
    free(vm.memory_begin);
    vm_prompt("virtual machine shutdown");
    return VM_OK;
}

#define CHECK_COMMAND(s, command, size)\
    (strlen(command) == size && strncmp(s, command, size) == 0)
#define PROMPT_UNKNOWN()\
    do{\
        printf("Unknown Options\n");\
        vm_shell_help();\
    }while(0)
#define MAX_COMMAND_CHARS 10
void vm_shell() {
    char command[MAX_COMMAND_CHARS];
    vm_prompt("VM Shell 1.0");
    printf("> ");
    while (scanf_s("%s", command, MAX_COMMAND_CHARS)) {
        switch (*command) {
            case 't':
                if (CHECK_COMMAND("test", command, 4)) {
                    printf("test running\n");
                    // \TODO test program
                }
                else PROMPT_UNKNOWN();
                break;
            case 'h':
                if (CHECK_COMMAND("help", command, 4)) vm_shell_help();
                else PROMPT_UNKNOWN();
                break;
            case 'q':
                if (CHECK_COMMAND("quiet", command, 5)) {
                    vm_params.prompt = 0;
                    printf("Run in quiet mode\n");
                }
                else PROMPT_UNKNOWN();
                break;
            case 'v':
                if (CHECK_COMMAND("verbose", command, 7)) {
                    vm_params.prompt = 1;
                    printf("Run in verbose mode\n");
                }
                else PROMPT_UNKNOWN();
                break;
            case 'r':
                if (CHECK_COMMAND("reboot", command, 6)) vm_reboot();
                else PROMPT_UNKNOWN();
                break;
            case 's':
                if (CHECK_COMMAND("shutdown", command, 8)) return;
                else PROMPT_UNKNOWN();
                break;
            case 'e':
                if (CHECK_COMMAND("exit", command, 4)) return;
                else PROMPT_UNKNOWN();
                break;
            case 'c':
                if (CHECK_COMMAND("clear", command, 5)) {
                    system("cls");
                    show_vm_info();
                }
                else PROMPT_UNKNOWN();
                break;
            default: PROMPT_UNKNOWN(); break;
        }
        printf("> ");
    }
}

int vm_allocate(long memory_size) {
    // \TODO
    return VM_OUT_OF_MEMORY;
}

void show_vm_info() {
    printf("%s\n", vm.name);
    printf("==========\n");
    printf("Memory Size: %ld\n", vm.memory_size);
    printf("Memory Region: %x - %x\n", vm.memory_begin, vm.memory_end);
    printf("==========\n");
}

void vm_shell_help() {
    printf("\thelp: Show this message\n");
    printf("\ttest: Run GC test program\n");
    printf("\tquiet: Run in quiet mode\n");
    printf("\tverbose: Run in verbose mode\n");
    printf("\treboot: Reboot this machine\n");
    printf("\tshutdown: Shutdown this machine\n");
    printf("\texit: Shutdown this machine\n");
}

int main(int argc, char* argv[]) {
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    int ret;
    if((ret = vm_init(NULL)) != VM_OK) EXCEPTION_THROW_INT(ret);
    if ((ret = vm_boot()) != VM_OK) EXCEPTION_THROW_INT(ret);
    vm_shell();
    if ((ret = vm_shutdown()) != VM_OK) EXCEPTION_THROW_INT(ret);
    return 0;
}
