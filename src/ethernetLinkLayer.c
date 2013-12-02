#include "ethernetLinkLayer.h"
#include "arp.h"
#include <os.h>
#include <lpc17xx_emac.h>


#define RX_DATA_BUFFER_SIZE 1542u   // 1542 bytes is biggest ethernet frame
#define TX_DATA_BUFFER_SIZE 1542u

static uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x67,0x00u,0x05u};   // default mac address

static OS_SEM rxSemaphore;
static uint8_t rxDataBuffer[RX_DATA_BUFFER_SIZE];
static EMAC_PACKETBUF_Type rxPacketBuffer;

static OS_SEM txSemaphore;
static uint8_t txDataBuffer[TX_DATA_BUFFER_SIZE];
static EMAC_PACKETBUF_Type txPacketBuffer;

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
        }
        
        EMAC_WritePacketBuffer(&txPacketBuffer);
    }
}

void EthernetLinkLayer_TaskProcess(void* p_arg)
{
    OS_ERR err;
    CPU_TS ts;

    
}


void EthernetLinkLayer_processRxData(uint8_t* data, uint32_t size)
{
    EthernetFrameHeader *ethernetFrameHeader;
    uint8_t             *payload;
    uint32_t            payloadSize;
    
    ethernetFrameHeader = (EthernetFrameHeader*)data;   // the frame header
    payload             = &data[ETHERNET_FRAME_HEADER_SIZE];                    // payload should always start at position 22
    payloadSize         = size - ETHERNET_FRAME_HEADER_SIZE;
    
    if ((ethernetFrameHeader->etherType[0] == 0x88u)
        && (ethernetFrameHeader->etherType[1] == 0x66u))
    {
        uint8_t response[3] = { 'A', 'C', 'K' };
        uint32_t responseSize = 3u;
        
        // Send and simple ACK response, switch destination and source address
        EthernetLinkLayer_sendPacket(ethernetFrameHeader->macDestination,
                                     ethernetFrameHeader->macSource,
                                     ethernetFrameHeader->etherType,
                                     response,
                                     responseSize);
    }
    else if ((ethernetFrameHeader->etherType[0] == 0x80u)   // ARP protocol request
        && (ethernetFrameHeader->etherType[1] == 0x60u))
    {
        Arp_processRequest(ethernetFrameHeader->macSource, payload);
    }
    else
    {
        // ignore packet
    }
}

int8_t EthernetLinkLayer_sendPacket(uint8_t* macSource, 
                                    uint8_t* macDestination, 
                                    uint8_t* type, 
                                    uint8_t* payload, 
                                    uint32_t payloadSize)
{
    OS_ERR err;
    
    EthernetFrameHeader *ethernetFrameHeader;
    uint8_t             *destinationPayload;
    
    ethernetFrameHeader = (EthernetFrameHeader*)txDataBuffer;
    destinationPayload = (uint8_t*)&((uint8_t*)txDataBuffer)[ETHERNET_FRAME_HEADER_SIZE];
    
    memcpy((void*)(ethernetFrameHeader->macSource), (void*)macSource, 6u);
    memcpy((void*)(ethernetFrameHeader->macDestination), (void*)macDestination, 6u);
    memcpy((void*)(ethernetFrameHeader->etherType), (void*)type, 2u);
    memcpy((void*)destinationPayload, (void*)payload, payloadSize);
    
    txPacketBuffer.ulDataLen = ETHERNET_FRAME_HEADER_SIZE + payloadSize;
    
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
    memcpy((void*)macAddress, (void*)address, 6u);
}

OS_SEM* EthernetLinkLayer_rxSemaphore(void)
{
    return &rxSemaphore;
}
