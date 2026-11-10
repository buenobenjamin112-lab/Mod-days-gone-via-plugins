
#include <kernel.h>
#include <systemservice.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern "C" int orbis_syscall(int num, ...);

__asm__(
    ".att_syntax prefix\n"
    ".globl orbis_syscall\n"
    "orbis_syscall:\n"
    "  movq $0, %rax\n"
    "  movq %rcx, %r10\n"
    "  syscall\n"
    "  jb 1f\n"
    "  retq\n"
    "1:\n"
    "  pushq %rax\n"
    "  callq __error\n"
    "  popq %rcx\n"
    "  movl %ecx, 0(%rax)\n"
    "  movq $0xFFFFFFFFFFFFFFFF, %rax\n"
    "  movq $0xFFFFFFFFFFFFFFFF, %rdx\n"
    "  retq\n"
);

struct proc_info {
    int pid;
    char name[40];
    char path[64];
    char titleid[16];
    char contentid[64];
    char version[6];
    uint64_t base_address;
} __attribute__((packed));

static void notify(const char *message) {
    sceKernelSendNotificationRequest(0, message, -1);
}

extern "C" {
int module_start(size_t args, const void *argp) {
    (void)args;
    (void)argp;

    proc_info info = {};
    int result = orbis_syscall(500, 4, &info);

    if (result != 0) {
        notify("Days Gone: falha ao consultar processo.");
        return 0;
    }

    if (strcmp(info.titleid, "CUSA08966") == 0) {
        char message[256];
        snprintf(message, sizeof(message),
            "Days Gone detectado | %s | PID %d | base 0x%llX",
            info.version, info.pid,
            (unsigned long long)info.base_address);
        notify(message);
    } else {
        char message[192];
        snprintf(message, sizeof(message),
            "Plugin ativo em outro processo: %.15s",
            info.titleid);
        notify(message);
    }

    return 0;
}

int module_stop(size_t args, const void *argp) {
    (void)args;
    (void)argp;
    notify("Days Gone: plugin descarregado.");
    return 0;
}
}
