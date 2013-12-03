#include "arp.h"
#include "ethernetLinkLayer.h"
#include <os_cfg_app.h>

OS_TMR tickTimer;

static ArpTableItem arpTable[ARP_TABLE_SIZE];
static uint8_t      arpTablePos = 0u;

static ArpPacketEthernetIPv4 arpEthernetIPv4ResponsePacket;
static uint32_t timeTick = 0u;

static uint8_t localIpAddress[4u] = {10u, 42u, 0u, 10u};

static const uint8_t broadcastMacAddress[6u] = {0x00u,0x00u,0x00u,0x00u,0x00u,0x00u};
static const uint8_t arpType[2u] = {0x08u, 0x06u};
static const uint8_t hardwareTypeEthernet[2u] = {0x00u, 0x01u};
static const uint8_t protocolTypeIPv4[2u] = {0x08u, 0x00u};
static const uint8_t operationCodeRequest[2u] = {0x00u, 0x01u};
static const uint8_t operationCodeResponse[2u] = {0x00, 0x02u};
static const uint32_t  arpTableExpirationTime = 20u; // 20min

/* private functions */
/** Adds an entry or updates an arp table entry
 *  @param macAddress mac address of the new entry
 *  @ipAddress ip address of the new entry
 */
void Arp_addArpTableEntry(uint8_t *macAddress, uint8_t *ipAddress);
/** Adds an entry or updates an arp table entry
 *  @param macAddress mac address of the new entry
 *  @param ipAddress ip address of the new entry
 */
void Arp_createResponse(uint8_t* packetSourceAddress, uint8_t* sourceMacAddress, uint8_t* sourceIpAddress, uint8_t* destinationMacAddress, uint8_t* destinationIpAddress);
/** Removes one entry from the arp table 
 */
void Arp_removeArpTableEntry(uint8_t pos);
/** Looks for outdated arp table entries 
 */
void Arp_updateArpTable(void);

int8_t Arp_initialize(void)
{
    OS_ERR err;
    
    OSTmrCreate((OS_TMR *) &tickTimer,
                (CPU_CHAR *) "ARP tick timer",
                (OS_TICK ) 0u,
                (OS_TICK ) (OS_CFG_TMR_TASK_RATE_HZ*60u),
                (OS_OPT  ) OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR) &Arp_timeTick1m,
                (void   *) NULL,
                (OS_ERR *) &err);
    
    if (err != OS_ERR_NONE)
    {
        return (int8_t)(-1);
    }
    
    return (int8_t)0;
}

int8_t Arp_processRequest(uint8_t* sourceAddress, uint8_t* requestData)
{
    ArpPacket *arpPacket;
    
    arpPacket = (ArpPacket*)requestData;
    
    if (memcmp((void*)(arpPacket->hardwareType), (void*)hardwareTypeEthernet, 2u) == (int)0)  // Ethernet
    {
        if (memcmp((void*)(arpPacket->protocolType), (void*)protocolTypeIPv4, 2u) == (int)0)  // IPv4
        {
            ArpPacketEthernetIPv4 *arpPacketEthernetIPv4;
            arpPacketEthernetIPv4 = (ArpPacketEthernetIPv4*)requestData;
            
            Arp_addArpTableEntry(arpPacketEthernetIPv4->sourceHardwareAddress,  
                                 arpPacketEthernetIPv4->sourceProtocolAddress); // Update out arp table
            
            if (memcmp((void*)(arpPacketEthernetIPv4->operationCode), (void*)operationCodeRequest, 2u) == (int)0)
            {
                Arp_createResponse(sourceAddress,
                                arpPacketEthernetIPv4->sourceHardwareAddress,
                                arpPacketEthernetIPv4->sourceProtocolAddress,
                                arpPacketEthernetIPv4->destinationHardwareAddress,
                                arpPacketEthernetIPv4->destinationProtocolAddress);
            }
            else
            {
                // we do not respond to responses
            }
            
            return (int8_t)(0);
        }
        else
        {
            return (int8_t)(-1);    // protocol type not supported
        }
    }
    else
    {
        return (int8_t)(-1);    // hardware type not supported
    }
    
    return (int8_t)(-1);
}

void Arp_createResponse(uint8_t* packetSourceAddress,
                        uint8_t* sourceMacAddress,
                        uint8_t* sourceIpAddress,
                        uint8_t* destinationMacAddress, 
                        uint8_t* destinationIpAddress)
{
    if (memcmp((void*)destinationMacAddress, (void*)broadcastMacAddress, 6u) == (int)(0))  // we have received an brodcast
    {
        if (memcmp((void*)destinationIpAddress, (void*)localIpAddress, 4u) == (int)(0))
        {
            // lets respond
            memcpy((void*)(arpEthernetIPv4ResponsePacket.hardwareType), (void*)hardwareTypeEthernet, 2u);
            memcpy((void*)(arpEthernetIPv4ResponsePacket.protocolType), (void*)protocolTypeIPv4, 2u);
            arpEthernetIPv4ResponsePacket.hardwareAddressLength = 6u;
            arpEthernetIPv4ResponsePacket.protocolAddressLength = 4u;
            memcpy((void*)(arpEthernetIPv4ResponsePacket.operationCode), (void*)operationCodeResponse, 2u);
            memcpy((void*)(arpEthernetIPv4ResponsePacket.sourceHardwareAddress), (void*)EthernetLinkLayer_macAddress(), 6u);
            memcpy((void*)(arpEthernetIPv4ResponsePacket.sourceProtocolAddress), (void*)localIpAddress, 4u);
            memcpy((void*)(arpEthernetIPv4ResponsePacket.destinationHardwareAddress), (void*)sourceMacAddress, 6u);
            memcpy((void*)(arpEthernetIPv4ResponsePacket.destinationProtocolAddress), (void*)sourceIpAddress, 4u);
            EthernetLinkLayer_sendPacket(EthernetLinkLayer_macAddress(),
                                         packetSourceAddress,
                                         (uint8_t*)arpType,
                                         (uint8_t*)(&arpEthernetIPv4ResponsePacket),
                                         28u);
        }
        else
        {
            // not our address, lets ignore it
        }
    }
    else
    {
        // not a broadcast, not our business
    }
}

void Arp_createRequest(uint8_t* ipAddress)
{
    memcpy((void*)(arpEthernetIPv4ResponsePacket.hardwareType), (void*)hardwareTypeEthernet, 2u);
    memcpy((void*)(arpEthernetIPv4ResponsePacket.protocolType), (void*)protocolTypeIPv4, 2u);
    arpEthernetIPv4ResponsePacket.hardwareAddressLength = 6u;
    arpEthernetIPv4ResponsePacket.protocolAddressLength = 4u;
    memcpy((void*)(arpEthernetIPv4ResponsePacket.operationCode), (void*)operationCodeRequest, 2u);
    memcpy((void*)(arpEthernetIPv4ResponsePacket.sourceHardwareAddress), (void*)EthernetLinkLayer_macAddress(), 6u);
    memcpy((void*)(arpEthernetIPv4ResponsePacket.sourceProtocolAddress), (void*)localIpAddress, 4u);
    memcpy((void*)(arpEthernetIPv4ResponsePacket.destinationHardwareAddress), (void*)broadcastMacAddress, 6u);
    memcpy((void*)(arpEthernetIPv4ResponsePacket.destinationProtocolAddress), (void*)ipAddress, 4u);
    EthernetLinkLayer_sendPacket(EthernetLinkLayer_macAddress(),
                                    (uint8_t*)broadcastMacAddress,
                                    (uint8_t*)arpType,
                                    (uint8_t*)(&arpEthernetIPv4ResponsePacket),
                                    28u);   
}

void Arp_addArpTableEntry(uint8_t *macAddress, uint8_t *ipAddress)
{
    uint8_t i;
    uint8_t targetTablePos;
    
    targetTablePos = arpTablePos+1u;
    
    for (i = 0u; i <= arpTablePos; i++)
    {
        if (memcmp((void*)macAddress, (void*)(arpTable[i].macAddress), 6u) == (int)(0))
        {
            targetTablePos = i;
            break;
        }
    }
    
    if (targetTablePos < ARP_TABLE_SIZE)
    {
        memcpy((void*)(arpTable[targetTablePos].macAddress), (void*)macAddress, 6u);    // copy mac address
        memcpy((void*)(arpTable[targetTablePos].ipAddress), (void*)ipAddress, 4u);      // copy ip address
        arpTable[targetTablePos].expirationTimestamp = timeTick + arpTableExpirationTime;
        
        if (targetTablePos > arpTablePos)   // if no entry was found a new entry was created
        {
            arpTablePos++;
        }
    }
    else
    {
        // Arp table is full
    }   
}

int8_t Arp_getMacAddress(uint8_t *ipAddress, uint8_t *macAddress)
{
    uint8_t i;
    
    for (i = 0u; i < arpTablePos; i++)
    {
        if (memcmp((void*)ipAddress, (void*)(arpTable[i].ipAddress), 4u) == (int)(0))
        {
            memcpy((void*)macAddress, (void*)(arpTable[i].macAddress), 4u);
            return (int8_t)0;
        }
    }
    
    return (int8_t)(-1);    // we do not know the mac address
}

void Arp_removeArpTableEntry(uint8_t pos)
{
    uint8_t i;
    
    for (i = pos; i < arpTablePos; i++)
    {
        memcpy((void*)(arpTable[i].macAddress), (void*)(arpTable[i+1u].macAddress), 6u);
        memcpy((void*)(arpTable[i].ipAddress), (void*)(arpTable[i+1u].ipAddress), 4u);
        arpTable[i].expirationTimestamp = arpTable[i+1u].expirationTimestamp;
    }
    
    arpTablePos--;
}

void Arp_updateArpTable(void)
{
    uint8_t i;
    
    for (i = arpTablePos; i >= 0u; i++)
    {
        if (arpTable[i].expirationTimestamp >= timeTick)
        {
            Arp_removeArpTableEntry(i);
        }
    }
}

void Arp_setLocalIpAddress(uint8_t* ipAddress)
{
    memcpy((void*)localIpAddress, (void*)ipAddress, 4u);
}

void Arp_timeTick1m(OS_TMR* p_tmr, void* p_arg)
{
    timeTick++;
    Arp_updateArpTable();
}
