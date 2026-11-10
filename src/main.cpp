
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

int plugin_load(int argc, char **argv) {
    (void)argc;
    (void)argv;

    proc_info info = {};
    int result = sys_sdk_proc_info(&info);

    if (result != 0) {
        notify_user("Days Gone Plugin: falha ao consultar processo.");
        return 0;
    }

    if (strcmp(info.titleid, "CUSA08966") == 0) {
        char message[256];

        snprintf(message, sizeof(message),
                 "Days Gone detectado | %.5s | PID %d",
                 info.version, info.pid);

        notify_user(message);
    } else {
        notify_user("Days Gone Plugin carregado.");
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
