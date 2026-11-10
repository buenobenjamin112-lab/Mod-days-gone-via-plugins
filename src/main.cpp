
#include <Common.h>
#include <stdio.h>
#include <string.h>

extern "C" {

const char *g_pluginName = "DaysGoneSpawner";

int module_start(size_t args, const void *argp) {
    (void)args;
    (void)argp;
    return 0;
}

int module_stop(size_t args, const void *argp) {
    (void)args;
    (void)argp;
    return 0;
}

static void notify_user(const char *message) {
    OrbisNotificationRequest request = {};

    request.type = NotificationRequest;
    request.unk3 = 0;
    request.useIconImageUri = 1;
    request.targetId = -1;

    snprintf(request.message, sizeof(request.message), "%s", message);
    snprintf(request.iconUri, sizeof(request.iconUri),
             "%s", "cxml://psnotification/tex_icon_champions_league");

    sceKernelSendNotificationRequest(0, &request, sizeof(request), 0);
}

static int get_main_module(uint64_t *base, uint32_t *module_size) {
    OrbisKernelModule handles[256];
    size_t count = 0;

    if (!base || !module_size)
        return -1;

    *base = 0;
    *module_size = 0;

    int result = sceKernelGetModuleList(handles, sizeof(handles), &count);
    if (result != 0)
        return result;

    if (count > 256)
        count = 256;

    for (size_t i = 0; i < count; ++i) {
        OrbisKernelModuleInfo module_info = {};
        module_info.size = sizeof(OrbisKernelModuleInfo);

        result = sceKernelGetModuleInfo(handles[i], &module_info);
        if (result != 0)
            continue;

        if (module_info.segmentInfo[0].address &&
            module_info.segmentInfo[0].size >= 4) {
            *base = (uint64_t)module_info.segmentInfo[0].address;
            *module_size = module_info.segmentInfo[0].size;
            return 0;
        }
    }

    return -2;
}

int plugin_load(int argc, char **argv) {
    (void)argc;
    (void)argv;

    proc_info info = {};
    int result = sys_sdk_proc_info(&info);

    if (result != 0) {
        notify_user("Days Gone: erro ao consultar processo.");
        return 0;
    }

    if (strcmp(info.titleid, "CUSA08966") != 0) {
        notify_user("Outro processo detectado.");
        return 0;
    }

    uint64_t module_base = 0;
    uint32_t module_size = 0;

    result = get_main_module(&module_base, &module_size);

    if (result != 0 || !module_base || module_size < 4) {
        notify_user("Days Gone: modulo nao localizado.");
        return 0;
    }

    const volatile unsigned char *memory =
        (const volatile unsigned char *)module_base;

    unsigned int b0 = memory[0];
    unsigned int b1 = memory[1];
    unsigned int b2 = memory[2];
    unsigned int b3 = memory[3];

    char message[256];

    snprintf(message, sizeof(message),
             "DG %s PID %d | MOD %llX | SZ %X | READ %02X%02X%02X%02X",
             info.version,
             info.pid,
             (unsigned long long)module_base,
             (unsigned int)module_size,
             b0, b1, b2, b3);

    notify_user(message);
    return 0;
}

int plugin_unload(int argc, char **argv) {
    (void)argc;
    (void)argv;

    notify_user("Days Gone Plugin descarregado.");
    return 0;
}

}
