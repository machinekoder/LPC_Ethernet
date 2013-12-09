#pragma once
#include "app.h"
#include <defines.h>


static OS_TCB App_PeriodicProcesses_TaskTCB;
static CPU_STK App_PeriodicProcesses_TaskStk[APP_STACK_SIZE];

void App_PeriodicProcesses_Task (void *p_arg);

