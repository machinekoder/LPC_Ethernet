#pragma once
#include "app.h"

static OS_TCB EthernetLinkLayer_TaskReadTCB;
static CPU_STK EthernetLinkLayer_TaskReadStk[APP_STACK_SIZE];

static OS_TCB EthernetLinkLayer_TaskWriteTCB;
static CPU_STK EthernetLinkLayer_TaskWriteStk[APP_STACK_SIZE];

void EthernetLinkLayer_TaskRead (void *p_arg);
void EthernetLinkLayer_TaskWrite (void *p_arg);

uint8_t*    EthernetLinkLayer_macAddress(void);
void        EthernetLinkLayer_setMacAddress(uint8_t *address);
OS_SEM*     EthernetLinkLayer_rxSemaphore(void);

int8_t      EthernetLinkLayer_sendPacket(uint32_t* data, uint32_t size);
