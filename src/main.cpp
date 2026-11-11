
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
static const size_t MAX_SCAN_SIZE = 1024 * 1024;

static const char *keywords[] = {
    "Horde",
    "horde",
    "Swarm",
    "swarm",
    "Freaker",
    "freaker",
    "Spawn",
    "spawn"
};

static void notify_user(const char *message) {
    OrbisNotificationRequest request = {};

    request.type = NotificationRequest;
    request.unk3 = 0;
    request.useIconImageUri = 1;
    request.targetId = -1;

    snprintf(request.message, sizeof(request.message),
             "%s", message);
    snprintf(request.iconUri, sizeof(request.iconUri),
             "%s", "cxml://psnotification/tex_icon_champions_league");

    sceKernelSendNotificationRequest(
        0, &request, sizeof(request), 0
    );
}

static int read_game_memory(
    uint64_t address,
    unsigned char *buffer,
    uint64_t length
) {
    if (!address || !buffer || !length)
        return -1;

    struct proc_rw request = {};
    request.address = address;
    request.data = buffer;
    request.length = length;
    request.write_flags = 0;

    return sys_sdk_proc_rw(&request);
}

static void scan_segment(
    uint64_t address,
    uint32_t segmentSize,
    unsigned int segmentNumber
) {
    static unsigned char buffer[MAX_SCAN_SIZE];

    if (!address || segmentSize == 0)
        return;

    uint64_t scanSize = segmentSize;

    if (scanSize > MAX_SCAN_SIZE)
        scanSize = MAX_SCAN_SIZE;

    int result = read_game_memory(
        address, buffer, scanSize
    );

    if (result != 0) {
        char message[128];

        snprintf(message, sizeof(message),
                 "SCAN S%u | ERRO LEITURA %d",
                 segmentNumber, result);

        notify_user(message);
        return;
    }

    unsigned int foundCount = 0;

    for (size_t k = 0;
         k < sizeof(keywords) / sizeof(keywords[0]);
         ++k) {

        size_t keyLength = strlen(keywords[k]);

        for (uint64_t i = 0;
             i + keyLength <= scanSize;
             ++i) {

            if (memcmp(
                &buffer[i],
                keywords[k],
                keyLength
            ) == 0) {

                char message[160];

                snprintf(
                    message, sizeof(message),
                    "REFERENCIA S%u | %s | @%llX",
                    segmentNumber,
                    keywords[k],
                    (unsigned long long)(address + i)
                );

                notify_user(message);
                foundCount++;

                if (foundCount >= 8) {
                    notify_user(
                        "LIMITE DE 8 REFERENCIAS POR SEGMENTO"
                    );
                    return;
                }
            }
        }
    }

    if (foundCount == 0) {
        char message[100];

        snprintf(message, sizeof(message),
                 "SCAN S%u | NENHUMA REFERENCIA",
                 segmentNumber);

        notify_user(message);
    }
}

static void inspect_eboot(void) {
    OrbisKernelModule modules[MAX_MODULES] = {};
    size_t available = 0;

    int result = sceKernelGetModuleList(
        modules, sizeof(modules), &available
    );

    if (result != 0) {
        notify_user("ERRO AO LISTAR MODULOS");
        return;
    }

    if (available > MAX_MODULES)
        available = MAX_MODULES;

    for (size_t i = 0; i < available; ++i) {
        OrbisKernelModuleInfo info = {};
        info.size = sizeof(info);

        if (sceKernelGetModuleInfo(
            modules[i], &info
        ) != 0)
            continue;

        if (strcmp(info.name, "eboot.bin") != 0)
            continue;

        notify_user("INICIANDO BUSCA DE REFERENCIAS");

        uint32_t count = info.segmentCount;

        if (count > 4)
            count = 4;

        for (uint32_t s = 0; s < count; ++s) {
            uint64_t address =
                (uint64_t)(uintptr_t)info.segmentInfo[s].address;

            uint32_t size = info.segmentInfo[s].size;

            scan_segment(address, size, s);
        }

        notify_user("BUSCA DE REFERENCIAS FINALIZADA");
        return;
    }

    notify_user("EBOOT NAO ENCONTRADO");
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
                char message[128];

                snprintf(
                    message, sizeof(message),
                    "DAYS GONE %s | PID %d",
                    info.version, info.pid
                );

                notify_user(message);
                last_pid = info.pid;

                inspect_eboot();
            }
        } else if (result == 0) {
            last_pid = -1;
        }

        sceKernelUsleep(2000000);
    }

    scePthreadExit(NULL);
    return NULL;
}

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

int plugin_load(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (g_monitorThreadStarted)
        return 0;

    g_monitorRunning = 1;

    int result = scePthreadCreate(
        &g_monitorThread,
        NULL,
        monitor_thread,
        NULL,
        "dg_monitor"
    );

    if (result != 0) {
        g_monitorRunning = 0;
        notify_user("ERRO AO INICIAR MONITOR");
        return 0;
    }

    g_monitorThreadStarted = 1;
    notify_user("DAYS GONE SCANNER INICIADO");

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

    notify_user("SCANNER DESCARREGADO");
    return 0;
}

}
