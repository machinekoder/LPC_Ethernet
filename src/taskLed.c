#include "taskLed.h"
#include "app.h"
#include <led.h>

void App_TaskLed (void *p_arg)
{
    OS_ERR       err;
    (void)p_arg;                                             /* Prevent Compiler Warning */
    
    Led_initialize(1u,29u,Led_LowActive_Yes); // onboard LED Led1
    
    while (DEF_TRUE)
    {
        OSTimeDlyHMSM(0u, 0u, 0u, 500u,OS_OPT_TIME_HMSM_STRICT,&err);
        Led_toggle(Led1);
    }
}
