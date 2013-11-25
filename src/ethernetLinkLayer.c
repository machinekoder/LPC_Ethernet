#include "ethernetLinkLayer.h"
#include <os.h>
#include <lpc17xx_emac.h>

#define RX_DATA_BUFFER_SIZE 256u
#define TX_DATA_BUFFER_SIZE 256u

uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x00u,0x05u};

OS_SEM rxSemaphore;
uint32_t rxDataBuffer[RX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type rxPacketBuffer;

OS_SEM txSemaphore;
uint32_t txDataBuffer[TX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type txPacketBuffer;

void EthernetLinkLayer_processRxData(void);

void EthernetLinkLayer_TaskRead(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    
    rxPacketBuffer.pbDataBuf = rxDataBuffer;
    rxPacketBuffer.ulDataLen = RX_DATA_BUFFER_SIZE;
    
    while (DEF_TRUE)
    {
        OSSemPend(&rxSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we receive something 
        EMAC_ReadPacketBuffer(&rxPacketBuffer);
        EthernetLinkLayer_processRxData();
        EMAC_UpdateRxConsumeIndex();
    }
}

void EthernetLinkLayer_TaskWrite(void* p_arg)
{
    OS_ERR       err;
    
    while (DEF_TRUE)
    {
        OSTimeDlyHMSM(0u, 0u, 1u, 0u,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}

void EthernetLinkLayer_processRxData(void)
{
    //rxDataBuffer;
    
}

uint8_t* EthernetLinkLayer_macAddress(void)
{
    return macAddress;
}

void EthernetLinkLayer_setMacAddress(uint8_t* address)
{
    uint8_t i;
    
    for (i = 0u; i < 6u; i++)
    {
        macAddress[i] = address[i];
    }
}

OS_SEM* EthernetLinkLayer_rxSemaphore(void)
{
    return &rxSemaphore;
}
