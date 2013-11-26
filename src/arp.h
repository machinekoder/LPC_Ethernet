#pragma once
#include <stdint.h>

#define ARP_TABLE_SIZE 10u

/** Generic ARP packet header */
typedef struct {
    uint8_t hardwareType[2u];
    uint8_t protocolType[2u];
    uint8_t hardwareAddressLength;
    uint8_t protocolAddressLength;
    uint8_t operationCode[2u];
} ArpPacket;

/** ARP packet for Ethernet and IPv4 */
typedef struct {
    uint8_t hardwareType[2u];
    uint8_t protocolType[2u];
    uint8_t hardwareAddressLength;
    uint8_t protocolAddressLength;
    uint8_t operationCode[2u];
    uint8_t sourceHardwareAddress[6u];
    uint8_t sourceProtocolAddress[4u];
    uint8_t destinationHardwareAddress[6u];
    uint8_t destinationProtocolAddress[4u];
} ArpPacketEthernetIPv4;

typedef struct {
    uint8_t macAddress[6u];
    uint8_t ipAddress[4u];
    uint32_t expirationTimestamp;
} ArpTableItem;

/** 
 * 
 */
int8_t Arp_processRequest(uint8_t* sourceAddress, uint8_t* requestData);
