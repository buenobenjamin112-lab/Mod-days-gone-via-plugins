#include <kernel.h>
#include <systemservice.h>

extern "C" {
    int module_start(size_t args, const void *argp) {
        sceKernelSendNotificationRequest(0, "Days Gone: Hordas Gigantes Ativo!", -1);
        return 0;
    }

    int module_stop(size_t args, const void *argp) {
        sceKernelSendNotificationRequest(0, "Days Gone: Hordas Descarregado.", -1);
        return 0;
    }
}
