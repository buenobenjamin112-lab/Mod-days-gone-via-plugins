#include <kernel.h>
#include <systemservice.h>
#include <stdio.h>

extern "C" {
    int module_start(size_t args, const void *argp) {
        // Envia uma notificação direta para o sistema do PS4 (GoldHEN)
        sceKernelSendNotificationRequest(0, "Days Gone: Hordas Gigantes Ativo!", -1);
        return 0;
    }

    int module_stop(size_t args, const void *argp) {
        sceKernelSendNotificationRequest(0, "Days Gone: Plugin Descarregado.", -1);
        return 0;
    }
}
