#include "ip.h"
#include "arp.h"
#include "ethernetLinkLayer.h"
#include <../ComplexCortex/driver/LPC13xx/types.h>
#include <string.h>

#define IP_SEND_BUFFER_SIZE 1500u
#define IP_ARP_RETRIES 5u
#define IP_ARP_TIMEOUT 100u

static const uint8_t protocolTypeIp[2u] = {0x08u, 0x00u};
static const uint8_t protocolIcmp = 1u;
static const uint8_t dummyChecksum[2u] = {0xFFu, 0xFFu};

static uint8_t ipv4Address[4u] = {10u, 42u, 0u, 10u};

static uint8_t ipSendBuffer[IP_SEND_BUFFER_SIZE];

int8_t Ip_processIcmp(uint8_t ttl, uint8_t* sourceAddress, uint8_t* destinationAddress, uint8_t* requestData, uint16_t reqestDataSize);
uint16_t Ip_calcChecksum(uint16_t * header, uint16_t len );

int8_t Ip_initialize(void)
{
    if (Arp_initialize() == (int8_t)(-1))
    {
        return (int8_t)(-1);
    }
    
    return (int8_t)0;
}

int8_t Ip_processRequest(uint8_t* requestData)
{
    IPv4Header *ipv4Header;
    uint8_t ipHeaderLength;
    
    ipv4Header = (IPv4Header*)requestData;
    
    if (ipv4Header->version == 4u)
    {
        ipHeaderLength = ipv4Header->ihl;
        
        if (ipv4Header->protocol == protocolIcmp)
        {
            return Ip_processIcmp(ipv4Header->ttl-1u,
                                  ipv4Header->sourceAddress, 
                                  ipv4Header->destinationAddress, 
                                  &(requestData[4u*ipHeaderLength]),
                                  (uint16_t)((ipv4Header->totalLength[0u] << 8) || (ipv4Header->totalLength[1u])));
        }
        else
        {
            // protocol not known
        }
    }
    else
    {
        // IP version other to 4 -> ignore
    }
    
    return (int8_t)(-1);
}

uint16_t Ip_icmpChecksum(uint16_t *data, uint8_t size)
{
    uint8_t i;
    uint16_t checksum;
    
    size = size/2u; // 16 bit
    checksum = 0u;
    
    for (i = 0u; i < size; i++)
    {
        checksum |= data[i];
    }
    
    return ~checksum;
}

int8_t Ip_processIcmp(uint8_t ttl, uint8_t* sourceAddress, uint8_t* destinationAddress, uint8_t* requestData, uint16_t reqestDataSize)
{
    IcmpPacket *icmpPacket;
    IcmpPacket *icmpResponsePacket;
    uint8_t    icmpResponse[8u];
    uint16_t   icmpChecksum;
    
    icmpPacket = (IcmpPacket*)requestData;
    icmpResponsePacket = (IcmpPacket*)icmpResponse;
    
    if (icmpPacket->type == 8u) // echoRequest
    {
        if (memcmp((void*)destinationAddress, (void*)ipv4Address, 4u) == (int)0) // our address
        {
            icmpResponsePacket->type = 0u; // echo response
            icmpResponsePacket->code = 0u;
            memset((void*)(icmpResponsePacket->checksum), 0, 2u);
            memcpy((void*)(icmpResponsePacket->data), (void*)(icmpPacket->data), 4u);
            
            icmpChecksum = Ip_icmpChecksum((uint16_t*)icmpResponsePacket, 8u);
            
            icmpResponsePacket->checksum[0u] = (uint8_t)((icmpChecksum >> 8u) & 0xFFu);
            icmpResponsePacket->checksum[1u] = (uint8_t)(icmpChecksum & 0xFFu);
            
            Ip_sendIPv4Packet(protocolIcmp, sourceAddress, ttl, icmpResponse, 8u);
        }
        return (int8_t)0;
    }
    else if (icmpPacket->type == 0u) // echoResponse
    {
        if (memcmp((void*)destinationAddress, (void*)ipv4Address, 4u) == (int)0) // our address
        {
            memcpy((void*)icmpResponsePacket, (void*)icmpPacket, 8u);
            
            Ip_sendIPv4Packet(protocolIcmp, sourceAddress, ttl, icmpResponse, 8u);
        }
        return (int8_t)0;
    }
    else
    {
        // unsupported type
    }
    
    return (int8_t)(-1);
}

// len = header length in bytes
uint16_t Ip_calcChecksum(uint16_t * header, uint16_t len )
{        
  uint16_t *pWord;
  uint32_t checksum = 0u;

  pWord = (uint16_t *)header;
  len = (len + 1u)/2u;
  while (len--) 
  {
    checksum += *(pWord++);   
  }
  checksum = (checksum & 0x0000ffff) + (checksum >> 16);
  checksum = (checksum & 0x0000ffff) + (checksum >> 16);  // if overflow in previous line
  checksum = ~checksum;
  return( (uint16_t)(checksum & 0x0000ffff) );
}

int8_t Ip_sendIPv4Packet(uint8_t protocol, uint8_t *destinationAddress, uint8_t ttl, uint8_t *payload, uint32_t payloadSize)
{
    OS_ERR err;
    IPv4Header *ipv4Header;
    uint8_t targetMacAddress[6u];
    uint8_t retryCount;
    uint16_t totalLength;
    uint16_t headerChecksum;
    uint8_t headerLength = 5u;
    
    ipv4Header = (IPv4Header*)ipSendBuffer;
    totalLength = headerLength*4u + (uint16_t)payloadSize;
    retryCount = 0u;
    
    ipv4Header->version = 4u;
    ipv4Header->ihl = headerLength;
    ipv4Header->tos = 0u;
    ipv4Header->totalLength[0u] = (uint8_t)((totalLength >> 8u) & 0xFFu);
    ipv4Header->totalLength[1u] = (uint8_t)(totalLength & 0xFFu);
    ipv4Header->identification[0u] = 0x00u;
    ipv4Header->identification[1u] = 0x00u;
    ipv4Header->flags = 0u;
    ipv4Header->fragmentOffset = 0u;
    ipv4Header->ttl = ttl;
    ipv4Header->protocol = protocol;
    ipv4Header->headerChecksum[0u] = 0x00u;
    ipv4Header->headerChecksum[1u] = 0x00u;
    memcpy((void*)(ipv4Header->sourceAddress), (void*)ipv4Address, 4u);
    memcpy((void*)(ipv4Header->destinationAddress), (void*)destinationAddress, 4u);
    memset((void*)(ipv4Header->optionsAndPadding), 0, 4u);
    
    headerChecksum = Ip_calcChecksum((uint16_t*)ipv4Header, (uint16_t)(headerLength*4u));
    ipv4Header->headerChecksum[1u] = (uint8_t)((headerChecksum >> 8u) & 0xFFu);
    ipv4Header->headerChecksum[0u] = (uint8_t)(headerChecksum & 0xFFu);
    
    memcpy((void*)(&ipSendBuffer[headerLength*4u]), (void*)payload, payloadSize);
    
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
    uint16_t icmpChecksum;
    IcmpPacket *icmpPacket;
    
    icmpPacket = (IcmpPacket*)icmpData;
    
    icmpPacket->type = 8u;  // echo request
    icmpPacket->code = 0u;
    memset((void*)(icmpPacket->checksum), 0, 2u);
    memcpy((void*)(icmpPacket->data), (void*)(icmpPacket->data), 4u);
    
    icmpChecksum = Ip_icmpChecksum((uint16_t*)icmpPacket, 8u);
    icmpPacket->checksum[0u] = (uint8_t)((icmpChecksum >> 8u) & 0xFFu);
    icmpPacket->checksum[1u] = (uint8_t)(icmpChecksum & 0xFFu);
    
    return Ip_sendIPv4Packet(protocolIcmp, destinationAddress, 64u, icmpData, 8u);
}

void Ip_setIPv4Address(uint8_t* address)
{
    Arp_setLocalIpAddress(address);
    memcpy((void*)ipv4Address, (void*)address, 4u);
}
