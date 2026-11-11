
#include <Common.h>
#include <orbis/libkernel.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

extern "C" {

const char *g_pluginName = "DaysGoneSpawner";

static volatile int g_monitorRunning = 0;
static int g_monitorThreadStarted = 0;
static OrbisPthread g_monitorThread;

static const size_t MAX_MODULES = 64;

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
    snprintf(request.iconUri, sizeof(request.iconUri), "%s",
             "cxml://psnotification/tex_icon_champions_league");

    sceKernelSendNotificationRequest(0, &request, sizeof(request), 0);
}

static int read_game_memory(uint64_t address,
                            unsigned char *buffer,
                            uint64_t length) {
    if (!address || !buffer || !length)
        return -1;

    struct proc_rw request = {};
    request.address = address;
    request.data = buffer;
    request.length = length;
    request.write_flags = 0;

    return sys_sdk_proc_rw(&request);
}

static void inspect_eboot_segments(void) {
    OrbisKernelModule modules[MAX_MODULES] = {};
    size_t available = 0;

    int list_result = sceKernelGetModuleList(
        modules, sizeof(modules), &available
    );

    if (list_result != 0) {
        char message[128];
        snprintf(message, sizeof(message),
                 "ERRO LISTA MODULOS | %d", list_result);
        notify_user(message);
        return;
    }

    if (available > MAX_MODULES)
        available = MAX_MODULES;

    for (size_t i = 0; i < available; ++i) {
        OrbisKernelModuleInfo moduleInfo = {};
        moduleInfo.size = sizeof(moduleInfo);

        int info_result = sceKernelGetModuleInfo(
            modules[i], &moduleInfo
        );

        if (info_result != 0)
            continue;

        if (strcmp(moduleInfo.name, "eboot.bin") != 0)
            continue;

        char found[128];
        snprintf(found, sizeof(found),
                 "EBOOT ENCONTRADO | SEGMENTOS %u",
                 moduleInfo.segmentCount);
        notify_user(found);

        uint32_t count = moduleInfo.segmentCount;
        if (count > 4)
            count = 4;

        for (uint32_t s = 0; s < count; ++s) {
            uint64_t address =
                (uint64_t)(uintptr_t)moduleInfo.segmentInfo[s].address;
            uint32_t segmentSize = moduleInfo.segmentInfo[s].size;
            int32_t protection = moduleInfo.segmentInfo[s].prot;

            if (!address || segmentSize < 8)
                continue;

            unsigned char bytes[8] = {};
            int read_result = read_game_memory(
                address, bytes, sizeof(bytes)
            );

            char message[180];

            if (read_result == 0) {
                snprintf(
                    message, sizeof(message),
                    "EBOOT S%u @%llX TAM %X P%X | %02X %02X %02X %02X %02X %02X %02X %02X",
                    s,
                    (unsigned long long)address,
                    segmentSize,
                    (unsigned int)protection,
                    bytes[0], bytes[1], bytes[2], bytes[3],
                    bytes[4], bytes[5], bytes[6], bytes[7]
                );
            } else {
                snprintf(
                    message, sizeof(message),
                    "EBOOT S%u @%llX TAM %X | ERRO LEITURA %d",
                    s,
                    (unsigned long long)address,
                    segmentSize,
                    read_result
                );
            }

            notify_user(message);
        }

        return;
    }

    notify_user("EBOOT.BIN NAO ENCONTRADO NA LISTA.");
}

static void *monitor_thread(void *arg) {
    (void)arg;

    int last_pid = -1;

    while (g_monitorRunning) {
        proc_info info = {};
        int result = sys_sdk_proc_info(&info);

        if (result == 0 &&
            strcmp(info.titleid, "CUSA08966") == 0) {

            if (last_pid != info.pid) {
                char message[160];

                snprintf(message, sizeof(message),
                         "MONITOR ATIVO | DAYS GONE %s | PID %d",
                         info.version, info.pid);
                notify_user(message);

                last_pid = info.pid;
                inspect_eboot_segments();
            }
        } else if (result == 0) {
            last_pid = -1;
        }

        sceKernelUsleep(2000000);
    }

    scePthreadExit(NULL);
    return NULL;
}

int plugin_load(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (g_monitorThreadStarted) {
        notify_user("Monitor Days Gone ja esta ativo.");
        return 0;
    }

    g_monitorRunning = 1;

    int result = scePthreadCreate(
        &g_monitorThread, NULL, monitor_thread, NULL, "dg_monitor"
    );

    if (result != 0) {
        g_monitorRunning = 0;
        notify_user("Erro ao iniciar monitor.");
        return 0;
    }

    g_monitorThreadStarted = 1;
    notify_user("Days Gone: monitor iniciado.");
    return 0;
}

int plugin_unload(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (g_monitorThreadStarted) {
        g_monitorRunning = 0;
        scePthreadJoin(g_monitorThread, NULL);
        g_monitorThreadStarted = 0;
    }

    notify_user("Days Gone Plugin descarregado.");
    return 0;
}

}
