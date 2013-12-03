#include "ip.h"
#include "arp.h"
#include "ethernetLinkLayer.h"
#include <string.h>

#define IP_SEND_BUFFER_SIZE 1500u
#define IP_ARP_RETRIES 5u
#define IP_ARP_TIMEOUT 10u

static const uint8_t protocolTypeIp[2u] = {0x08u, 0x00u};
static const uint8_t protocolIcmp = 1u;
static const uint8_t dummyChecksum[2u] = {0xFFu, 0xFFu};

static uint8_t ipv4Address[4u] = {10u, 42u, 0u, 10u};

static uint8_t ipSendBuffer[IP_SEND_BUFFER_SIZE];

int8_t Ip_initialize(void)
{
    if (Arp_initialize() == (int8_t)(-1))
    {
        return (int8_t)(-1);
    }
    
    return (int8_t)0;
}

int8_t Ip_sendIPv4Packet(uint8_t protocol, uint8_t *destinationAddress, uint8_t *payload, uint32_t payloadSize)
{
    OS_ERR err;
    IPv4Header *ipv4Header;
    uint8_t targetMacAddress[6u];
    uint8_t retryCount;
    uint16_t totalLength;
    
    ipv4Header = (IPv4Header*)ipSendBuffer;
    totalLength = 5u + payloadSize;
    retryCount = 0u;
    
    ipv4Header->version = 4u;
    ipv4Header->ihl = 5u;
    ipv4Header->tos = 0u;
    ipv4Header->totalLength[0u] = (uint8_t)(totalLength >> 8u);
    ipv4Header->totalLength[1u] = (uint8_t)(totalLength && 0xFFu);
    ipv4Header->flags = 0u;
    ipv4Header->fragmentOffset = 0u;
    ipv4Header->ttl = 255u;
    ipv4Header->protocol = protocol;
    memcpy((void*)(ipv4Header->headerChecksum), (void*)dummyChecksum, 2u);
    memcpy((void*)(ipv4Header->sourceAddress), (void*)ipv4Address, 4u);
    memcpy((void*)(ipv4Header->destinationAddress), (void*)destinationAddress, 4u);
    memset((void*)(ipv4Header->optionsAndPadding), 0, 4u);
    
    memcpy((void*)(&ipSendBuffer[6u]), (void*)payload, payloadSize);
    
    while (Arp_getMacAddress(destinationAddress, targetMacAddress) != (int8_t)0)
    {
        if (retryCount >= IP_ARP_RETRIES)
        {
            return (int8_t)(-1);
        }
        
        Arp_createRequest(destinationAddress);
        retryCount++;
        
        OSTimeDlyHMSM(0u, 0u, 0u, IP_ARP_TIMEOUT, OS_OPT_TIME_HMSM_STRICT, &err);
    }
    
    EthernetLinkLayer_sendPacket(EthernetLinkLayer_macAddress(),
                                 (uint8_t*) targetMacAddress,
                                 (uint8_t*) protocolTypeIp,
                                 (uint8_t*) ipSendBuffer,
                                 totalLength);
    
    return (int8_t)0;
}

int8_t Ip_sendPing(uint8_t* destinationAddress)
{
    uint8_t icmpData[8u];
    IcmpPacket *icmpPacket;
    
    icmpPacket = (IcmpPacket*)icmpData;
    
    icmpPacket->type = 8u;  // echo request
    icmpPacket->code = 0u;
    icmpPacket->checksum[0u] = 0x90u;
    icmpPacket->checksum[1u] = 0xd7u;
    
    return Ip_sendIPv4Packet(protocolIcmp, destinationAddress, icmpData, 8u);
}

void Ip_setIPv4Address(uint8_t* address)
{
    Arp_setLocalIpAddress(address);
    memcpy((void*)ipv4Address, (void*)address, 4u);
}
