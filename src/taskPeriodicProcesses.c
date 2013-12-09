#include "os.h"
#include "app.h"
#include <ip.h>

static const uint8_t targetIp[4u] = {10u, 42u, 0u, 1u};

void App_PeriodicProcesses_Task (void *p_arg)
{
    OS_ERR       err;
    (void)p_arg;                                             /* Prevent Compiler Warning */

    
    while (DEF_TRUE)
    {
        OSTimeDlyHMSM(0u, 0u, 1u, 0u,OS_OPT_TIME_HMSM_STRICT,&err);
        Ip_sendPing((uint8_t*)targetIp);
    }
}
