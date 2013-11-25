#include "ethernetLinkLayer.h"
#include <os.h>
#include <lpc17xx_emac.h>

#define RX_DATA_BUFFER_SIZE 256u
#define TX_DATA_BUFFER_SIZE 256u

uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x67,0x00u,0x05u};

OS_SEM rxSemaphore;
uint32_t rxDataBuffer[RX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type rxPacketBuffer;

OS_SEM txSemaphore;
uint32_t txDataBuffer[TX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type txPacketBuffer;

void EthernetLinkLayer_processRxData(uint32_t size);

void EthernetLinkLayer_TaskRead(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    
    // init rx buffer
    rxPacketBuffer.pbDataBuf = rxDataBuffer;
    rxPacketBuffer.ulDataLen = RX_DATA_BUFFER_SIZE;
    
    while (DEF_TRUE)
    {
        OSSemPend(&rxSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we receive something 
        
        EMAC_ReadPacketBuffer(&rxPacketBuffer);
        EthernetLinkLayer_processRxData(EMAC_GetReceiveDataSize());
        if (EMAC_CheckReceiveIndex() == TRUE)
        {
            EMAC_UpdateRxConsumeIndex();
        }
    }
}

void EthernetLinkLayer_TaskWrite(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    
    // init tx buffer
    txPacketBuffer.pbDataBuf = txDataBuffer;
    txPacketBuffer.ulDataLen = TX_DATA_BUFFER_SIZE;
    
    OSSemCreate(&txSemaphore, "TX_SEM", (OS_SEM_CTR)0u, &err);
    
    while (DEF_TRUE)
    {
        OSSemPend(&txSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we want to send something 
        
        while(EMAC_CheckTransmitIndex() == TRUE)    // if not available wait 1ms
        {
            OSTimeDlyHMSM(0u, 0u, 0u, 1u,OS_OPT_TIME_HMSM_STRICT,&err);
        }
        
        EMAC_WritePacketBuffer(&txPacketBuffer);
    }
}

void EthernetLinkLayer_processRxData(uint32_t size)
{
    EthernetLinkLayer_sendPacket(rxDataBuffer, size);
    
}

int8_t EthernetLinkLayer_sendPacket(uint32_t* data, uint32_t size)
{
    OS_ERR err;
    
    if (size > TX_DATA_BUFFER_SIZE)                         // check packet size
    {
        return (int8_t)(-1);
    }
    
    memcpy((void*)txDataBuffer, (void*)data, size);         // copy data to internal buffer
    OSSemPost(&txSemaphore, (OS_OPT)OS_OPT_POST_ALL,&err);  // post semaphore
    
    if (err != OS_ERR_NONE)                                 // check for errors
    {
        return(int8_t)(-1);
    }
    
    return (int8_t)0;
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
