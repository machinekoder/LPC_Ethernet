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

/** Processes a arp request
 *  @param sourceAddress source mac address
 *  @param requestData arp packet payload
 *  @return 0 on success, -1 on failure
 */
int8_t Arp_processRequest(uint8_t* sourceAddress, uint8_t* requestData);

/** Creates an ARP reqest for the specified IP address
 *  @param ipAddress IP address to resolve
 */
void Arp_createRequest(uint8_t* ipAddress);

/** Resolves an mac address to an IPv4 address based on the arp table
 *  @param macAddress mac address to resolve
 *  @param ipAddress  pointer to where the ip address should be stored
 *  @return 0 if entry was found -1 if not
 */
int8_t Arp_getIpAddress(uint8_t *macAddress, uint8_t *ipAddress);

/** Sets the local IP address
 *  @param ipAddress the IP address to set
 */
void Arp_setLocalIpAddress(uint8_t *ipAddress);

/** 1 minute tick for cleaning up the arp tables
 */
void Arp_timeTick1m(void);
