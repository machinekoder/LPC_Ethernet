#include "taskButton.h"
#include "ip.h"
#include "app.h"

static const uint8_t targetIp[4u] = {10u, 42u, 0u, 1u};

void buttonInit();

void App_TaskButton (void *p_arg)
{
    OS_ERR       err;
    CPU_INT08U count;
    ButtonValue value;
    (void)p_arg;                                                    /* Prevent Compiler Warning */

    buttonInit();
        
    while(DEF_TRUE) {
        for(count=0u; count<=10u; count++)
        {
            Button_task();        // reads the button values
            OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &err);
            if (count==10u)
            {
                while (Button_getPress(&value) != (int8)(-1))
                {
                    if (value.id == (uint8)Button1)
                    {
                        // do something
                        Ip_sendPing((uint8_t*)targetIp);
                    }
                }
            }
        }
    }
}

void buttonInit ()
{
    Button_initialize2(10000u, 100000u);    // 10ms sampling, 100ms timeout
    //+++++++++++++++++++++++++++++++++++++++++BUTTON++++++++++++++++++++++++++++++++++++++++++++++++++
    Button_initializeButton(Button1, 2u, 10u, Button_Type_LowActive);
 }
