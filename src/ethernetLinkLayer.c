#include "ethernetLinkLayer.h"
#include "arp.h"
#include "ip.h"
#include <os.h>
#include <lpc17xx_emac.h>
#include <../ComplexCortex/driver/LPC13xx/types.h>


#define RX_DATA_BUFFER_SIZE  1542u   // 1542 bytes is biggest ethernet frame
#define RX_DATA_BUFFER_COUNT 6u
#define TX_DATA_BUFFER_SIZE  1542u
#define TX_DATA_BUFFER_COUNT 2u

static const uint8_t etherTypeCustom[2u] = {0x88u,0x66u};
static const uint8_t etherTypeArp[2u] = {0x08u,0x06u};
static const uint8_t etherTypeIp[2u] = {0x08u,0x00u};

static uint8_t macAddress[6] = {0x00u,0xE5u,0xC1u,0x67u,0x00u,0x05u};   // default mac address

static OS_SEM rxSemaphore;
static EMAC_PACKETBUF_Type rxPacketBuffer;

static EMAC_PACKETBUF_Type txPacketBuffer;

//static uint32_t __attribute__ ((aligned (4))) *ethernetRxBufferData = (uint32_t*)0x20080000u;
//static uint32_t __attribute__ ((aligned (4))) *ethernetTxBufferData = (uint32_t*)(0x20080000u + 2700u);

static OS_MEM ethernetRxBuffer;
static OS_MEM ethernetTxBuffer;

static OS_Q   ethernetRxQueue;
static OS_Q   ethernetTxQueue;

void EthernetLinkLayer_processRxData(uint8_t* data, uint32_t size);

void EthernetLinkLayer_TaskRead(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    uint8_t* rxDataBuffer;
    
    // init rx buffer
    OSMemCreate((OS_MEM     *) &ethernetRxBuffer,
                (CPU_CHAR   *) "EthernetRxBuffer",
                (void       *) 0x20080000u,
                (OS_MEM_QTY  ) RX_DATA_BUFFER_COUNT,
                (OS_MEM_SIZE ) RX_DATA_BUFFER_SIZE,
                (OS_ERR     *) &err);
    
    OSQCreate((OS_Q     *) &ethernetRxQueue,
              (CPU_CHAR *) "EthernetRxQueue",
              (OS_MSG_QTY) RX_DATA_BUFFER_COUNT,
              (OS_ERR   *) &err);
    
    //rxPacketBuffer.pbDataBuf = (uint32_t*)rxDataBuffer;
    rxPacketBuffer.ulDataLen = RX_DATA_BUFFER_SIZE;
    
    while (DEF_TRUE)
    {
        OSSemPend(&rxSemaphore, (OS_TICK)0u, (OS_OPT)OS_OPT_PEND_BLOCKING, &ts, &err);  // Wait until we receive something 
        
        rxDataBuffer = OSMemGet((OS_MEM    *) &ethernetRxBuffer,
                                (OS_ERR    *) &err);
        
        if (err == OS_ERR_NONE)
        {
            rxPacketBuffer.pbDataBuf = (uint32_t*)rxDataBuffer;
            
            EMAC_ReadPacketBuffer(&rxPacketBuffer);
            
            if (EMAC_CheckReceiveIndex() == TRUE)
            {
                EMAC_UpdateRxConsumeIndex();
            }
            
            OSQPost((OS_Q   *) &ethernetRxQueue,
                    (void   *) rxDataBuffer,
                    (OS_MSG_SIZE) EMAC_GetReceiveDataSize(),
                    (OS_OPT  ) OS_OPT_POST_FIFO,
                    (OS_ERR *) &err);
        }
    }
}

void EthernetLinkLayer_TaskWrite(void* p_arg)
{
    OS_ERR  err;
    CPU_TS  ts;
    void *txDataBuffer;
    OS_MSG_SIZE msgSize;
    
    OSQCreate((OS_Q     *) &ethernetTxQueue,
              (CPU_CHAR *) "EthernetTxQueue",
              (OS_MSG_QTY) TX_DATA_BUFFER_COUNT,
              (OS_ERR   *) &err);
    
    // init tx buffer
    OSMemCreate((OS_MEM     *) &ethernetTxBuffer,
                (CPU_CHAR   *) "EthernetTxBuffer",
                (void       *) (0x20080000u + 2700u),
                (OS_MEM_QTY  ) TX_DATA_BUFFER_COUNT,
                (OS_MEM_SIZE ) TX_DATA_BUFFER_SIZE,
                (OS_ERR     *) &err);
    txPacketBuffer.ulDataLen = TX_DATA_BUFFER_SIZE;
    
    while (DEF_TRUE)
    {
        txDataBuffer = OSQPend((OS_Q   *) &ethernetTxQueue,
                                (OS_TICK ) 0u,
                                (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                                (OS_MSG_SIZE *) &msgSize,
                                (CPU_TS *) &ts,
                                (OS_ERR *) &err);
        
        txPacketBuffer.pbDataBuf = (uint32_t*)txDataBuffer;
              
        EMAC_WritePacketBuffer(&txPacketBuffer);
        
        if(EMAC_CheckTransmitIndex() == TRUE)    // if not available wait 1ms
        {
            EMAC_UpdateTxProduceIndex();
        }
        
        OSMemPut((OS_MEM    *) &ethernetTxBuffer,
                 (void      *) txDataBuffer,
                 (OS_ERR    *) &err);
    }
}

void EthernetLinkLayer_TaskProcess(void* p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    OS_MSG_SIZE msgSize;
    void *rxDataBuffer;
    
    
    while (DEF_TRUE)
    {
        rxDataBuffer = OSQPend((OS_Q   *) &ethernetRxQueue,
                                (OS_TICK ) 0u,
                                (OS_OPT  ) OS_OPT_PEND_BLOCKING,
                                (OS_MSG_SIZE *) &msgSize,
                                (CPU_TS *) &ts,
                                (OS_ERR *) &err);
                
        EthernetLinkLayer_processRxData((uint8_t*)(rxDataBuffer), (uint32_t)msgSize);
    
        OSMemPut((OS_MEM    *) &ethernetRxBuffer,
                (void      *) rxDataBuffer,
                (OS_ERR    *) &err);
    }
    
}

void EthernetLinkLayer_processRxData(uint8_t* data, uint32_t size)
{
    EthernetFrameHeader *ethernetFrameHeader;
    uint8_t             *payload;
    uint32_t            payloadSize;
    
    ethernetFrameHeader = (EthernetFrameHeader*)data;   // the frame header
    payload             = &data[ETHERNET_FRAME_HEADER_SIZE];                    // payload should always start at position 22
    payloadSize         = size - ETHERNET_FRAME_HEADER_SIZE;
    
    if (memcmp((void*)(ethernetFrameHeader->etherType), (void*)etherTypeCustom, 2u) == (int)0) // Custom type
    {
#if 0
        uint8_t response[3] = { 'A', 'C', 'K' };
        uint32_t responseSize = 3u;
        
        // Send and simple ACK response, switch destination and source address
        EthernetLinkLayer_sendPacket(ethernetFrameHeader->macDestination,
                                     ethernetFrameHeader->macSource,
                                     ethernetFrameHeader->etherType,
                                     response,
                                     responseSize);
#else
    EthernetLinkLayer_sendPacket(ethernetFrameHeader->macDestination,
                                    ethernetFrameHeader->macSource,
                                    ethernetFrameHeader->etherType,
                                    Arp_getTable(),
                                    ARP_TABLE_BYTE_SIZE);
#endif
    }
    else if (memcmp((void*)(ethernetFrameHeader->etherType), (void*)etherTypeArp, 2u) == (int)0) // ARP protocol
    {
        Arp_processRequest(ethernetFrameHeader->macSource, payload);
    }
    else if (memcmp((void*)(ethernetFrameHeader->etherType), (void*)etherTypeIp, 2u) == (int)0)
    {
        Ip_processRequest(payload);
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
    void* txDataBuffer;
    
    EthernetFrameHeader *ethernetFrameHeader;
    uint8_t             *destinationPayload;
    
    txDataBuffer = OSMemGet((OS_MEM    *) &ethernetTxBuffer,
                            (OS_ERR    *) &err);
        
    if (err != OS_ERR_NONE)
    {
        return (int8_t)(-1);
    }
        
    ethernetFrameHeader = (EthernetFrameHeader*)txDataBuffer;
    destinationPayload = (uint8_t*)&((uint8_t*)txDataBuffer)[ETHERNET_FRAME_HEADER_SIZE];
    
    memcpy((void*)(ethernetFrameHeader->macSource), (void*)macSource, 6u);
    memcpy((void*)(ethernetFrameHeader->macDestination), (void*)macDestination, 6u);
    memcpy((void*)(ethernetFrameHeader->etherType), (void*)type, 2u);
    memcpy((void*)destinationPayload, (void*)payload, payloadSize);
    
    txPacketBuffer.ulDataLen = ETHERNET_FRAME_HEADER_SIZE + payloadSize;
    
    OSQPost((OS_Q   *) &ethernetTxQueue,
            (void   *) txDataBuffer,
            (OS_MSG_SIZE) EMAC_GetReceiveDataSize(),
            (OS_OPT  ) OS_OPT_POST_FIFO,
            (OS_ERR *) &err);
    
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
