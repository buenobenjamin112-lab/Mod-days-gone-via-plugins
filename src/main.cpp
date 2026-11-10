
#include <Common.h>
#include <stdio.h>
#include <string.h>

static void notify_user(const char *message) {
    OrbisNotificationRequest request = {};

    request.type = NotificationRequest;
    request.unk3 = 0;
    request.useIconImageUri = 1;
    request.targetId = -1;

    snprintf(request.message, sizeof(request.message), "%s", message);
    snprintf(request.iconUri, sizeof(request.iconUri),
             "%s", "cxml://psnotification/tex_icon_champions_league");

    sceKernelSendNotificationRequest(
        0, &request, sizeof(request), 0
    );
}

extern "C" {

int module_start(size_t args, const void *argp) {
    (void)args;
    (void)argp;

    proc_info info = {};
    int result = sys_sdk_proc_info(&info);

    if (result != 0) {
        notify_user("Plugin: falha na consulta GoldHEN.");
        return 0;
    }

    if (strcmp(info.titleid, "CUSA08966") == 0) {
        char message[256];

        snprintf(message, sizeof(message),
                 "Days Gone detectado | %.5s | PID %d",
                 info.version, info.pid);

        notify_user(message);
    } else {
        char message[192];

        snprintf(message, sizeof(message),
                 "Plugin ativo | processo: %.39s",
                 info.name);

        notify_user(message);
    }

    return 0;
}

int module_stop(size_t args, const void *argp) {
    (void)args;
    (void)argp;

    notify_user("Days Gone plugin descarregado.");
    return 0;
}

}
