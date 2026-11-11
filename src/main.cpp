
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

    snprintf(request.message, sizeof(request.message),
             "%s", message);

    snprintf(request.iconUri, sizeof(request.iconUri),
             "%s",
             "cxml://psnotification/tex_icon_champions_league");

    sceKernelSendNotificationRequest(
        0, &request, sizeof(request), 0
    );
}

int plugin_load(int argc, char **argv) {
    (void)argc;
    (void)argv;

    proc_info info = {};
    int result = sys_sdk_proc_info(&info);

    if (result != 0) {
        notify_user("Erro ao consultar processo.");
        return 0;
    }

    if (strcmp(info.titleid, "CUSA08966") != 0) {
        notify_user("Outro processo detectado.");
        return 0;
    }

    OrbisKernelModule handles[256] = {};
    size_t count = 0;

    result = sceKernelGetModuleList(
        handles, sizeof(handles), &count
    );

    if (result != 0) {
        notify_user("Erro ao listar modulos.");
        return 0;
    }

    if (count > 256)
        count = 256;

    char message[240];

    snprintf(message, sizeof(message),
             "DAYS GONE %s | PID %d | MODULOS %u",
             info.version, info.pid, (unsigned int)count);

    notify_user(message);

    for (size_t i = 0; i < count; i++) {
        OrbisKernelModuleInfo module_info = {};
        module_info.size = sizeof(OrbisKernelModuleInfo);

        result = sceKernelGetModuleInfo(
            handles[i], &module_info
        );

        if (result != 0)
            continue;

        if (i == 0) {
            size_t segments = module_info.segmentCount;

            if (segments > 4)
                segments = 4;

            snprintf(message, sizeof(message),
                     "EBOOT: %u segmentos",
                     (unsigned int)segments);

            notify_user(message);

            for (size_t s = 0; s < segments; s++) {
                snprintf(message, sizeof(message),
                         "EBOOT SEG %u @%llX TAM %X",
                         (unsigned int)s,
                         (unsigned long long)
                         module_info.segmentInfo[s].address,
                         (unsigned int)
                         module_info.segmentInfo[s].size);

                notify_user(message);
            }
        } else {
            snprintf(message, sizeof(message),
                     "#%u %.12s @%llX/%X",
                     (unsigned int)i,
                     module_info.name,
                     (unsigned long long)
                     module_info.segmentInfo[0].address,
                     (unsigned int)
                     module_info.segmentInfo[0].size);

            notify_user(message);
        }
    }

    return 0;
}

int plugin_unload(int argc, char **argv) {
    (void)argc;
    (void)argv;

    notify_user("Days Gone Plugin descarregado.");
    return 0;
}

}
