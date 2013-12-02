#include "taskButton.h"
#include "app.h"

#define BUTTON_STEP_UM 5000

bool buttonXPlusTest  = FALSE;
bool buttonXMinusTest = FALSE;
bool buttonYPlusTest  = FALSE;
bool buttonYMinusTest = FALSE;
bool buttonZPlusTest  = FALSE;
bool buttonZMinusTest = FALSE;
bool buttonOkTest     = FALSE;
bool endschalterXPlusTest  = FALSE;
bool endschalterXMinusTest = FALSE;
bool endschalterYPlusTest  = FALSE;
bool endschalterYMinusTest = FALSE;
bool endschalterZPlusTest  = FALSE;
bool endschalterZMinusTest = FALSE;

void buttonInit();

void App_TaskButton (void *p_arg)
{
    OS_ERR       err;
    CPU_INT08U count;
    ButtonValue value;
    (void)p_arg;                                                    /* Prevent Compiler Warning */

    buttonInit();
        
    while(DEF_TRUE) {
        for(count=0; count<=10; count++)
        {
            Button_task();        // reads the button values
            OSTimeDlyHMSM(0u, 0u, 0u, 10u, OS_OPT_TIME_HMSM_STRICT, &err);
            if(count==10)
            {
                while (Button_getPress(&value) != (int8)(-1))
                {
                    if (value.id == Button1)
                    {
                        // do something
                    }
                }
            }
        }
    }
}

void buttonInit ()
{
    Button_initialize2(1E4, 1E5);
    //+++++++++++++++++++++++++++++++++++++++++BUTTON++++++++++++++++++++++++++++++++++++++++++++++++++
    Button_initializeButton(Button1, 2u, 10u, Button_Type_LowActive);
 }
