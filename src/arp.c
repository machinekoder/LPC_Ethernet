#include "arp.h"
#include "ethernetLinkLayer.h"

static ArpTableItem arpTable[ARP_TABLE_SIZE];
static uint8_t      arpTablePos = 0u;

static uint8_t localIpAddress[4u] = {10u, 42u, 0u, 10u};
static const uint8_t broadcastMacAddress[6u] = {0x00,0x00,0x00,0x00,0x00,0x00};

/* private functions */
void Arp_addArpTableEntry(uint8_t *macAddress, uint8_t *ipAddress);
void Arp_createResponse(uint8_t* sourceMacAddress, uint8_t* sourceIpAddress, uint8_t* destinationMacAddress, uint8_t* targetIpAddress);

int8_t Arp_processRequest(uint8_t* sourceAddress, uint8_t* requestData)
{
    ArpPacket *arpPacket;
    
    arpPacket = (ArpPacket*)requestData;
    
    if ((arpPacket->hardwareType[0u] == 0x00u)
        && (arpPacket->hardwareType[1u] == 0x01u))  // Ethernet
    {
        if ((arpPacket->protocolType[0u] == 0x08u)
            && (arpPacket->protocolType[1u] == 0x00u))  // IPv4
        {
            ArpPacketEthernetIPv4 *arpPacketEthernetIPv4;
            arpPacketEthernetIPv4 = (ArpPacketEthernetIPv4*)requestData;
            
            Arp_addArpTableEntry(arpPacketEthernetIPv4->sourceHardwareAddress,  
                                 arpPacketEthernetIPv4->sourceProtocolAddress); // Update out arp table
            
            Arp_createResponse(arpPacketEthernetIPv4->sourceHardwareAddress,
                               arpPacketEthernetIPv4->sourceProtocolAddress,
                               arpPacketEthernetIPv4->destinationHardwareAddress,
                               arpPacketEthernetIPv4->destinationProtocolAddress);
            
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

void Arp_createResponse(uint8_t* sourceMacAddress,
                        uint8_t* sourceIpAddress,
                        uint8_t* destinationMacAddress, 
                        uint8_t* destinationIpAddress)
{
    uint8_t i;
    
    if (memcmp((void*)destinationMacAddress, (void*)broadcastMacAddress, 6u) == (int)(0))  // we have received an brodcast
    {
        if (memcmp((void*)destinationIpAddress, (void*)localIpAddress, 4u) == (int)(0))
        {
            
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

void Arp_addArpTableEntry(uint8_t *macAddress, uint8_t *ipAddress)
{
    uint8_t i;
    uint8_t j;
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
