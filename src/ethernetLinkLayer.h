#pragma once
#include "app.h"

#define ETHERNET_FRAME_HEADER_SIZE 14u

typedef struct {
    uint8_t macDestination[6];
    uint8_t macSource[6];
    uint8_t etherType[2];
} EthernetFrameHeader;  // !will be extended to 16byte

static OS_TCB EthernetLinkLayer_TaskReadTCB;
static CPU_STK EthernetLinkLayer_TaskReadStk[APP_STACK_SIZE];

static OS_TCB EthernetLinkLayer_TaskWriteTCB;
static CPU_STK EthernetLinkLayer_TaskWriteStk[APP_STACK_SIZE];

static OS_TCB EthernetLinkLayer_TaskProcessTCB;
static CPU_STK EthernetLinkLayer_TaskProcessStk[APP_STACK_SIZE];

void EthernetLinkLayer_TaskRead (void *p_arg);
void EthernetLinkLayer_TaskWrite (void *p_arg);
void EthernetLinkLayer_TaskProcess(void *p_arg);

uint8_t*    EthernetLinkLayer_macAddress(void);
void        EthernetLinkLayer_setMacAddress(uint8_t* address);
void        EthernetLinkLayer_setMacAddress(uint8_t *address);
OS_SEM*     EthernetLinkLayer_rxSemaphore(void);

int8_t      EthernetLinkLayer_sendPacket(uint8_t* macSource, uint8_t* macDestination, uint8_t* type, uint8_t* payload, uint32_t payloadSize);
