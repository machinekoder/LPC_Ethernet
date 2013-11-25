#include "taskUsbConnection.h"
#include "app.h"

#define COMMAND_BUFFER_SIZE 200u

void commandSplitter(char* data, uint32 length);
void processCommand(char *buffer);

void App_TaskUsbConnection (void *p_arg)
{
    OS_ERR       err;
    (void)p_arg;                                             /* Prevent Compiler Warning */

    if(CSP_IntVectReg((CSP_DEV_NBR   )CSP_INT_CTRL_NBR_MAIN,
                      (CSP_DEV_NBR   )CSP_INT_SRC_NBR_USB_00,
                      (CPU_FNCT_PTR  )USB_IRQHandler,
                      (void         *)0) != DEF_OK) {
        while(DEF_TRUE);
    }                                                                                             /* register Interrupt Handler in RTOS */

    CSP_IntEn(CSP_INT_CTRL_NBR_MAIN, CSP_INT_SRC_NBR_USB_00);   /* Enable USB Interrupt. */

    USBHwConnect(TRUE);

    (void)p_arg;                                              /* Prevent Compiler Warning */
    while(DEF_TRUE)
    {
        if(usbReceiveBufferSize > 0)
        {   /* if a Message was received */
            commandSplitter((char*)&usbReceiveBuffer[2], usbReceiveBufferSize);
            usbReceiveBufferSize = 0;                                    /* reset the Message length of the incoming buffer */
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &err);

    }
}

void commandSplitter(char* data, uint32 length)
{
    uint32 i;
    uint32 commandBufferPos = 0;
    static char commandBuffer[COMMAND_BUFFER_SIZE];

    for (i = 0; i < length; ++i)
    {
        if (data[i] == '\n')
        {
            processCommand(commandBuffer);
            commandBufferPos = 0;
        }
        else
        {
            commandBuffer[commandBufferPos] = data[i];
            commandBufferPos++;
        }
    }
}

void printUnknownCommand(void)
{
    USB_printf("CMD?\n");
}

void printParameterMissing(void)
{
    USB_printf("Missing parameter.\n");
}

void printAcknowledgement(void)
{
    USB_printf("ACK\n");
}

void printError(char *message)
{
    USB_printf("ERR: %s\n", message);
}

void printAliveMessage(void)
{
    USB_printf("yes\n");
}

void printOk(void)
{
    USB_printf("ok\n");
}

bool compareBaseCommand(char *original, char *received)
{
    return (strcmp(original,received) == 0);
}

bool compareExtendedCommand(char *original, char *received)
{
    return (((strlen(received) == 1) && (strncmp(original,received,1) == 0)) ||
            (strcmp(original,received) == 0));
}

void processCommand(char *buffer)
{
    char *dataPointer;
    char *savePointer;

    dataPointer = strtok_r(buffer, " ", &savePointer);

    if (compareBaseCommand("move", dataPointer))
    {
        dataPointer = strtok_r(NULL, " ", &savePointer);
        if (compareExtendedCommand("x",dataPointer))
        {
            // We have a set command
            dataPointer = strtok_r(NULL, " ", &savePointer);
            if (dataPointer != NULL)
            {
#if 0
                int32 movement = (int32)(atof(dataPointer)*1000.0);
                
                if (movement != 0) 
                {
                    targetX += movement;
                    if (targetX < 0)
                    {
                        targetX = 0;
                    }
                    applicationState = ApplicationState_Working;
                }
#endif
            }
            else
            {
                printUnknownCommand();
            }
            return;
        }
        else
        {
            printUnknownCommand();
        }
        return;
    }
    else
    {
        printUnknownCommand();
        return;
    }
}
