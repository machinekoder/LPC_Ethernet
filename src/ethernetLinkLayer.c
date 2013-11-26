#include "ethernetLinkLayer.h"
#include <os.h>
#include <lpc17xx_emac.h>

#define RX_DATA_BUFFER_SIZE 1542u   // 1542 bytes is biggest ethernet frame
#define TX_DATA_BUFFER_SIZE 1542u

uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x67,0x00u,0x05u};

OS_SEM rxSemaphore;
uint8_t rxDataBuffer[RX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type rxPacketBuffer;

OS_SEM txSemaphore;
uint8_t txDataBuffer[TX_DATA_BUFFER_SIZE];
EMAC_PACKETBUF_Type txPacketBuffer;

void EthernetLinkLayer_processRxData(uint8_t* data, uint32_t size);

void EthernetLinkLayer_TaskRead(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    
    // init rx buffer
    rxPacketBuffer.pbDataBuf = (uint32_t*)rxDataBuffer;
    rxPacketBuffer.ulDataLen = RX_DATA_BUFFER_SIZE;
    
    while (DEF_TRUE)
    {
        OSSemPend(&rxSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we receive something 
        
        EMAC_ReadPacketBuffer(&rxPacketBuffer);
        EthernetLinkLayer_processRxData((uint8_t*)(rxPacketBuffer.pbDataBuf), EMAC_GetReceiveDataSize());
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
    txPacketBuffer.pbDataBuf = (uint32_t*)txDataBuffer;
    txPacketBuffer.ulDataLen = TX_DATA_BUFFER_SIZE;
    
    OSSemCreate(&txSemaphore, "TX_SEM", (OS_SEM_CTR)0u, &err);
    
    while (DEF_TRUE)
    {
        OSSemPend(&txSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we want to send something 
        
        if(EMAC_CheckTransmitIndex() == TRUE)    // if not available wait 1ms
        {
            EMAC_UpdateTxProduceIndex();
            //OSTimeDlyHMSM(0u, 0u, 0u, 10u,OS_OPT_TIME_HMSM_STRICT,&err);
        }
        
        EMAC_WritePacketBuffer(&txPacketBuffer);
    }
}

void EthernetLinkLayer_processRxData(uint8_t* data, uint32_t size)
{
    EthernetFrameHeader *ethernetFrameHeader;
    uint8_t             *payload;
    
    ethernetFrameHeader = (EthernetFrameHeader*)data;   // the frame header
    payload             = &data[14];                    // payload should always start at position 22
    
    if ((ethernetFrameHeader->etherType[0] == 0x88u)
        && (ethernetFrameHeader->etherType[1] == 0x66u))
    {
        uint8_t sourceMacAddress[6];
        uint8_t destinationMacAddress[6];
        uint8_t response[3] = { 'A', 'C', 'K' };
        uint32_t responseSize = 3u;
        
        memcpy((void*)sourceMacAddress, (void*)(ethernetFrameHeader->macSource), 6u);
        memcpy((void*)destinationMacAddress, (void*)(ethernetFrameHeader->macDestination), 6u);
        
        memcpy((void*)(ethernetFrameHeader->macSource), (void*)destinationMacAddress, 6u);
        memcpy((void*)(ethernetFrameHeader->macDestination), (void*)sourceMacAddress, 6u);
        
        memcpy((void*)payload, (void*)response, 3u);
        
        EthernetLinkLayer_sendPacket(data, 22u + responseSize);
    }
}

int8_t EthernetLinkLayer_sendPacket(uint8_t* data, uint32_t size)
{
    OS_ERR err;
    
    //if (size > TX_DATA_BUFFER_SIZE)                         // check packet size
    //{
    //    return (int8_t)(-1);
    //}
    
    txPacketBuffer.ulDataLen = size;
    
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
