
#include <Common.h>
#include <orbis/libkernel.h>
#include <stdio.h>
#include <string.h>

extern "C" {

const char *g_pluginName = "DaysGoneSpawner";

static volatile int g_monitorRunning = 0;
static int g_monitorThreadStarted = 0;
static OrbisPthread g_monitorThread;

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
        &g_monitorThread,
        NULL,
        monitor_thread,
        NULL,
        "dg_monitor"
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
